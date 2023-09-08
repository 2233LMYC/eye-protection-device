#include "delay.h"
#include "math.h"

volatile u8 fac_us=0; //us��ʱ������  

/**
  * @brief   ��ʱ������ʼ��
  * @param   ��
  * @retval  ��
  * @attention  Ϊȷ��׼ȷ��,�뱣֤ʱ��Ƶ�����Ϊ4�ı���,���8Mhz;
  *             clk:ʱ��Ƶ��(24/16/12/8��)
  */
void delay_init(u8 clk)
{
  if (clk > 16)
  {
    fac_us = (16 - 4) / 4;//24Mhzʱ,stm8���19������Ϊ1us
  }
  else if (clk > 4)
  {
    fac_us = (clk - 4) / 4; 
  }
  else fac_us = 1;
}

/**
  * @brief   ��ʱus����
  * @param   nus����ʱʱ�䣬��λus
  * @retval  ��
  * @attention    ��ʱʱ��=(fac_us*4+4)*nus*(T)��TΪCPU����Ƶ��(Mhz)�ĵ���,��λΪus.
  *               ׼ȷ��: 92%  @24Mhz 
  *                       98%  @16Mhz           
  *                       98%  @12Mhz
  *                       86%  @8Mhz
  */
void delay_us(u16 nus)
{  
__asm(
"PUSH A          \n"  //1T,ѹջ
"DELAY_XUS:      \n"   
"LD A,fac_us     \n"   //1T,fac_us���ص��ۼ���A
"DELAY_US_1:     \n"  
"NOP             \n"  //1T,nop��ʱ
"DEC A           \n"  //1T,A--
"JRNE DELAY_US_1 \n"   //������0,����ת(2T)��DELAY_US_1����ִ��,������0,����ת(1T).
"NOP             \n"  //1T,nop��ʱ
"DECW X          \n"  //1T,x--
"JRNE DELAY_XUS  \n"    //������0,����ת(2T)��DELAY_XUS����ִ��,������0,����ת(1T).
"POP A           \n"  //1T,��ջ
); 
}

/**
  * @brief   ��ʱms����
  * @param   nms����ʱʱ�䣬��λms
  * @retval  ��
  * @attention  Ϊ��֤׼ȷ��,nms��Ҫ����16640.
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
  * @brief   �����ֵС������
  * @param   dt:��������,deci:С��λ��,��ౣ��4λС��
  * @retval  x2:�Ŵ���С������
  */
u16 Get_decimal(double dt, u8 deci)  
{
    long x1 = 0;
    u16 x2 = 0, x3 = 0;
    
    if (deci > 4) //��ౣ��4λС��
    {
      deci = 4;
    } 
    if (deci < 1) //���ٱ���4λС��
    {
      deci = 1;
    } 
    
    x3 = (u16)pow(10, deci); //pow()�����㺯���������� math.h;pow(a,b)=a��b�η�
    x1 = (long)(dt * x3);
    x2 = (u16)(x1%x3);
    
    return x2;
}


