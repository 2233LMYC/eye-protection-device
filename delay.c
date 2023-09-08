#include "delay.h"
#include "math.h"

volatile u8 fac_us=0; //us延时倍乘数  

/**
  * @brief   延时函数初始化
  * @param   无
  * @retval  无
  * @attention  为确保准确度,请保证时钟频率最好为4的倍数,最低8Mhz;
  *             clk:时钟频率(24/16/12/8等)
  */
void delay_init(u8 clk)
{
  if (clk > 16)
  {
    fac_us = (16 - 4) / 4;//24Mhz时,stm8大概19个周期为1us
  }
  else if (clk > 4)
  {
    fac_us = (clk - 4) / 4; 
  }
  else fac_us = 1;
}

/**
  * @brief   延时us函数
  * @param   nus：延时时间，单位us
  * @retval  无
  * @attention    延时时间=(fac_us*4+4)*nus*(T)，T为CPU运行频率(Mhz)的倒数,单位为us.
  *               准确度: 92%  @24Mhz 
  *                       98%  @16Mhz           
  *                       98%  @12Mhz
  *                       86%  @8Mhz
  */
void delay_us(u16 nus)
{  
__asm(
"PUSH A          \n"  //1T,压栈
"DELAY_XUS:      \n"   
"LD A,fac_us     \n"   //1T,fac_us加载到累加器A
"DELAY_US_1:     \n"  
"NOP             \n"  //1T,nop延时
"DEC A           \n"  //1T,A--
"JRNE DELAY_US_1 \n"   //不等于0,则跳转(2T)到DELAY_US_1继续执行,若等于0,则不跳转(1T).
"NOP             \n"  //1T,nop延时
"DECW X          \n"  //1T,x--
"JRNE DELAY_XUS  \n"    //不等于0,则跳转(2T)到DELAY_XUS继续执行,若等于0,则不跳转(1T).
"POP A           \n"  //1T,出栈
); 
}

/**
  * @brief   延时ms函数
  * @param   nms：延时时间，单位ms
  * @retval  无
  * @attention  为保证准确度,nms不要大于16640.
  */
void delay_ms(u32 nms)
{
  u8 t;
  if (nms > 65)
  {
    t = nms / 65;
    while (t--)
    {
      delay_us(65000);
    }
    nms = nms % 65;
  }
  delay_us(nms * 1000);
}

/**
  * @brief   获得数值小数部分
  * @param   dt:输入数据,deci:小数位数,最多保留4位小数
  * @retval  x2:放大后的小数部分
  */
u16 Get_decimal(double dt, u8 deci)  
{
    long x1 = 0;
    u16 x2 = 0, x3 = 0;
    
    if (deci > 4) //最多保留4位小数
    {
      deci = 4;
    } 
    if (deci < 1) //至少保留4位小数
    {
      deci = 1;
    } 
    
    x3 = (u16)pow(10, deci); //pow()幂运算函数，需引入 math.h;pow(a,b)=a的b次方
    x1 = (long)(dt * x3);
    x2 = (u16)(x1%x3);
    
    return x2;
}


