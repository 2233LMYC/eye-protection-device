/*
*       project ：激光护眼仪（stm8s103f3p6）
*       author  ：LMTX
*       Version ：2.30
*       2022.8.31.18:00
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_gpio.h"
#include "stm8s_tim1.h"
#include "stm8s_tim2.h"

/*----------------------------------------------------------------------------
模式一 ===> 5min
模式二 ===> 3min

1、上电后，蜂鸣器长鸣一声，LED1与LED2双闪，提示设备已准备好

2、KEY1短按，选择模式一，蜂鸣器快响两声，点亮对应LED

3、KEY2长按，开始模式二，蜂鸣器长鸣一声，对应LED长亮

4、模式结束后，蜂鸣器长鸣一声，LED1与LED2双闪，提示设备已准备好

----------------------------------------------------------------------------*/
#define LED1_OFF    GPIO_WriteHigh(GPIOC, GPIO_PIN_7)
#define LED1_ON     GPIO_WriteLow(GPIOC, GPIO_PIN_7)
#define LED1_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_7)  //LED状态取反
#define LED2_OFF    GPIO_WriteHigh(GPIOC, GPIO_PIN_6)
#define LED2_ON     GPIO_WriteLow(GPIOC, GPIO_PIN_6)
#define LED2_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_6)
#define BEEP_ON     GPIO_WriteHigh(GPIOC, GPIO_PIN_3)
#define BEEP_OFF    GPIO_WriteLow(GPIOC, GPIO_PIN_3)    //蜂鸣器
#define BEEP_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_3)
#define LIGHT_ON    GPIO_WriteHigh(GPIOC, GPIO_PIN_4)   //激光
#define LIGHT_OFF   GPIO_WriteLow(GPIOC, GPIO_PIN_4)
#define LIGHT_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_4)

#define KEY1_PORT (GPIOD)
#define KEY1_PIN (GPIO_PIN_4)   
#define KEY2_PORT (GPIOD)
#define KEY2_PIN (GPIO_PIN_3)

#define L_TIME      5 //单位 minute
#define S_TIME      3 //单位 minute
 
#define key_time_long   1000    //单位ms
#define key_time_short  100    //单位ms


char SYS_START = 0;     //系统电源开关

char Start    = 0;      //开机标志

char Working  = 0;      //工作进行标志

char s03_flag = 0;      //0.3s标志
  
char mode     = 0;      //模式标志

//不同情况下的蜂鸣器计数
int Beep_Start=0,Beep_mode1=0,Beep_mode2=0,Beep_L1=0,Beep_L2=0;

void delayus(unsigned int nCount)   //16M 晶振时  延时 1个微妙
{
    nCount*=3;//等同于 nCount=nCount*3  相当于把nCount变量扩大3倍
    while(--nCount);//nCount变量数值先减一，再判断nCount的数值是否大于0，大于0循环减一，等于0退出循环。
}
void delayms(unsigned int nCount)  //16M 晶振时  延时 1个毫秒
{
    while(nCount--)//先判断while()循环体里的nCount数值是否大于0，大于0循环，减一执行循环体，等于0退出循环。
    {
        delayus(1000);//调用微妙延时函数，输入1000等译演示1毫秒。
    }
}

void KEY_func(void)
{  
  unsigned int key = 0;

   if(!(KEY1_PORT->IDR&KEY1_PIN)) //检测到有按键了，IO口电压会被拉低
  {
    delayms(5);
    if(!(KEY1_PORT->IDR&KEY1_PIN)) //仍然存在低电平
    {
        while(!(KEY1_PORT->IDR&KEY1_PIN))
        {
          key++;
          delayms(1);
        }
        if(key>=key_time_long)          //KEY1长按
        {
          key = 0;
          if(SYS_START == 1)    SYS_START = 0;
          else SYS_START = 1;
        }
        
        if((key>key_time_short)&&(key<key_time_long)) //KEY1短按
        {
           key = 0; 
           mode++;
           if(mode>2)  mode = 1;            
        }
        
    }                    
  }

  if(!(KEY2_PORT->IDR&KEY2_PIN)) //检测到有按键了，IO口电压会被拉低
  {
    delayms(5);
    if(!(KEY2_PORT->IDR&KEY2_PIN)) //仍然存在低电平
    {
        while(!(KEY2_PORT->IDR&KEY2_PIN))
        {
          key++;
          delayms(1);
        }
        if(key>=key_time_long)          //KEY2长按
        {
          key = 0;
          Working = 1;
          TIM1_Cmd(ENABLE);
        }
        
    }                    
  }
   	
} 


void TIM_Init(void)
{
  //时钟初始化
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
  
  TIM1_TimeBaseInit(15, TIM1_COUNTERMODE_UP, 1000, 0);//15+1分频  1/1000000s，向上计数，计时(1000+1）*1us，重复0次
  TIM1_ARRPreloadConfig(ENABLE);
  TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);

  TIM2_TimeBaseInit(TIM2_PRESCALER_16, 1000);
  TIM2_ClearFlag(TIM2_FLAG_UPDATE);
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
  
  TIM2_Cmd(ENABLE);     //其他定时
  TIM1_Cmd(DISABLE);    //工作定时
}


//TIMER1中断
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_BRK_IRQHandler, 11)//约1ms进一次中断
{
  static unsigned int MS_cnt = 0;
  static unsigned int S_cnt  = 0;
  
    MS_cnt++;
    
    if(MS_cnt == 1000)  //1s到
    {
      MS_cnt = 0; 
      S_cnt++;
    }
    if(mode == 1)
    {
      if(S_cnt == (60*L_TIME))     //模式一时间到
      {
        S_cnt = 0;
        Start = 0;
        Working = 0;
        LIGHT_OFF;
        LED1_OFF;
        LED2_OFF;
        Beep_Start = 0;
        Beep_mode1=0;
        Beep_mode2=0;
        Beep_L1=0;
        Beep_L2=0;
        TIM1_Cmd(DISABLE);
      }
    }
    else if(mode == 2)
    {
      if(S_cnt == (60*S_TIME))  //模式二时间到
      {
        S_cnt = 0;
        Start = 0;
        Working = 0;
        LIGHT_OFF;
        LED1_OFF;
        LED2_OFF;
        Beep_Start = 0;
        Beep_mode1=0;
        Beep_mode2=0;
        Beep_L1=0;
        Beep_L2=0;
        TIM1_Cmd(DISABLE);
      }
    }
   
    //清除更新标志位
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

//TIMER2中断
INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)//约1ms进一次
 {
    static unsigned int MS_cnt = 0;
  
    MS_cnt++;

    if(MS_cnt == 300)   //0.3s
    {
      s03_flag = 1;
      MS_cnt = 0;
    }

    TIM2_ClearITPendingBit(TIM2_IT_UPDATE);
 }

void main(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);//时钟速度为内部16M，1分频
    
    enableInterrupts();//中断使能                                 
    
    //GPIO初始化
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);

    TIM_Init(); //定时器启动

     //初始化完成
    LED1_OFF;
    LED2_OFF;
    LIGHT_OFF;
    BEEP_OFF; 

    while (1)
    {
      

          if(Start == 0)    //      上电准备好
          {
            BEEP_ON;
            Beep_mode1 = 0;
            Beep_mode2 = 0;
            if(s03_flag == 1)
            {
              s03_flag = 0;
              LED1_FILP;
              LED2_FILP;
              Beep_Start++;
            }
            if(Beep_Start > 5)      //蜂鸣器响0.3*5=1.5s
            {
              BEEP_OFF;
            }
          }
          if(Working == 0)
          {
            KEY_func();
            
            switch(mode)
            {
              case 1:
              {
                LED2_OFF;
                Start = 1;
                Beep_mode2 = 0;
                if(s03_flag == 1)
                {
                  s03_flag = 0;
                  LED1_FILP;
                  if(Beep_mode1 > 4) 
                    BEEP_OFF;
                  if(Beep_mode1 < 4)
                    BEEP_FILP;
                  
                  Beep_mode1++;
                }
                
              }break;
              case 2:
              {
                LED1_OFF;
                Start = 1;
                Beep_mode1 = 0;
                if(s03_flag == 1)
                {
                  s03_flag = 0;
                  LED2_FILP;
                  if(Beep_mode2 > 4) 
                    BEEP_OFF;
                  if(Beep_mode2 < 4)
                    BEEP_FILP;
                  Beep_mode2++;
                }
              }break;
              default: break;
              
            }
          }
          else if(Working == 1)
          {
            switch(mode)
            {
              case 1:
              {
                LED2_OFF;
                LED1_ON;
                Start = 1;
                if(Beep_L1 < 3)
                  BEEP_ON;
                else 
                {
                  BEEP_OFF;
                  LIGHT_ON;
                  TIM1_Cmd(ENABLE);
                }
                if(s03_flag == 1)
                {
                  s03_flag = 0;
                  Beep_L1++;
                } 
              }break;
              case 2:
              {
                LED1_OFF;
                LED2_ON;
                Start = 1;
                if(Beep_L2 < 3)
                  BEEP_ON;
                else 
                {
                  BEEP_OFF;
                  LIGHT_ON;
                  TIM1_Cmd(ENABLE);
                }
                if(s03_flag == 1)
                {
                  s03_flag = 0;
                  Beep_L2++;
                }
              }break;
              default: break;
              
            }
            
          }  
      
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retva: None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}



#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
