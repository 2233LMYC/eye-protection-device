/*
*       project �����⻤���ǣ�stm8s103f3p6��
*       author  ��LMTX
*       Version ��2.30
*       2022.8.31.18:00
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_gpio.h"
#include "stm8s_tim1.h"
#include "stm8s_tim2.h"

/*----------------------------------------------------------------------------
ģʽһ ===> 5min
ģʽ�� ===> 3min

1���ϵ�󣬷���������һ����LED1��LED2˫������ʾ�豸��׼����

2��KEY1�̰���ѡ��ģʽһ������������������������ӦLED

3��KEY2��������ʼģʽ��������������һ������ӦLED����

4��ģʽ�����󣬷���������һ����LED1��LED2˫������ʾ�豸��׼����

----------------------------------------------------------------------------*/
#define LED1_OFF    GPIO_WriteHigh(GPIOC, GPIO_PIN_7)
#define LED1_ON     GPIO_WriteLow(GPIOC, GPIO_PIN_7)
#define LED1_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_7)  //LED״̬ȡ��
#define LED2_OFF    GPIO_WriteHigh(GPIOC, GPIO_PIN_6)
#define LED2_ON     GPIO_WriteLow(GPIOC, GPIO_PIN_6)
#define LED2_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_6)
#define BEEP_ON     GPIO_WriteHigh(GPIOC, GPIO_PIN_3)
#define BEEP_OFF    GPIO_WriteLow(GPIOC, GPIO_PIN_3)    //������
#define BEEP_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_3)
#define LIGHT_ON    GPIO_WriteHigh(GPIOC, GPIO_PIN_4)   //����
#define LIGHT_OFF   GPIO_WriteLow(GPIOC, GPIO_PIN_4)
#define LIGHT_FILP   GPIO_WriteReverse(GPIOC, GPIO_PIN_4)

#define KEY1_PORT (GPIOD)
#define KEY1_PIN (GPIO_PIN_4)   
#define KEY2_PORT (GPIOD)
#define KEY2_PIN (GPIO_PIN_3)

#define L_TIME      5 //��λ minute
#define S_TIME      3 //��λ minute
 
#define key_time_long   1000    //��λms
#define key_time_short  100    //��λms


char SYS_START = 0;     //ϵͳ��Դ����

char Start    = 0;      //������־

char Working  = 0;      //�������б�־

char s03_flag = 0;      //0.3s��־
  
char mode     = 0;      //ģʽ��־

//��ͬ����µķ���������
int Beep_Start=0,Beep_mode1=0,Beep_mode2=0,Beep_L1=0,Beep_L2=0;

void delayus(unsigned int nCount)   //16M ����ʱ  ��ʱ 1��΢��
{
    nCount*=3;//��ͬ�� nCount=nCount*3  �൱�ڰ�nCount��������3��
    while(--nCount);//nCount������ֵ�ȼ�һ�����ж�nCount����ֵ�Ƿ����0������0ѭ����һ������0�˳�ѭ����
}
void delayms(unsigned int nCount)  //16M ����ʱ  ��ʱ 1������
{
    while(nCount--)//���ж�while()ѭ�������nCount��ֵ�Ƿ����0������0ѭ������һִ��ѭ���壬����0�˳�ѭ����
    {
        delayus(1000);//����΢����ʱ����������1000������ʾ1���롣
    }
}

void KEY_func(void)
{  
  unsigned int key = 0;

   if(!(KEY1_PORT->IDR&KEY1_PIN)) //��⵽�а����ˣ�IO�ڵ�ѹ�ᱻ����
  {
    delayms(5);
    if(!(KEY1_PORT->IDR&KEY1_PIN)) //��Ȼ���ڵ͵�ƽ
    {
        while(!(KEY1_PORT->IDR&KEY1_PIN))
        {
          key++;
          delayms(1);
        }
        if(key>=key_time_long)          //KEY1����
        {
          key = 0;
          if(SYS_START == 1)    SYS_START = 0;
          else SYS_START = 1;
        }
        
        if((key>key_time_short)&&(key<key_time_long)) //KEY1�̰�
        {
           key = 0; 
           mode++;
           if(mode>2)  mode = 1;            
        }
        
    }                    
  }

  if(!(KEY2_PORT->IDR&KEY2_PIN)) //��⵽�а����ˣ�IO�ڵ�ѹ�ᱻ����
  {
    delayms(5);
    if(!(KEY2_PORT->IDR&KEY2_PIN)) //��Ȼ���ڵ͵�ƽ
    {
        while(!(KEY2_PORT->IDR&KEY2_PIN))
        {
          key++;
          delayms(1);
        }
        if(key>=key_time_long)          //KEY2����
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
  //ʱ�ӳ�ʼ��
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
  
  TIM1_TimeBaseInit(15, TIM1_COUNTERMODE_UP, 1000, 0);//15+1��Ƶ  1/1000000s�����ϼ�������ʱ(1000+1��*1us���ظ�0��
  TIM1_ARRPreloadConfig(ENABLE);
  TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);

  TIM2_TimeBaseInit(TIM2_PRESCALER_16, 1000);
  TIM2_ClearFlag(TIM2_FLAG_UPDATE);
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
  
  TIM2_Cmd(ENABLE);     //������ʱ
  TIM1_Cmd(DISABLE);    //������ʱ
}


//TIMER1�ж�
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_BRK_IRQHandler, 11)//Լ1ms��һ���ж�
{
  static unsigned int MS_cnt = 0;
  static unsigned int S_cnt  = 0;
  
    MS_cnt++;
    
    if(MS_cnt == 1000)  //1s��
    {
      MS_cnt = 0; 
      S_cnt++;
    }
    if(mode == 1)
    {
      if(S_cnt == (60*L_TIME))     //ģʽһʱ�䵽
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
      if(S_cnt == (60*S_TIME))  //ģʽ��ʱ�䵽
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
   
    //������±�־λ
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

//TIMER2�ж�
INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)//Լ1ms��һ��
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
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);//ʱ���ٶ�Ϊ�ڲ�16M��1��Ƶ
    
    enableInterrupts();//�ж�ʹ��                                 
    
    //GPIO��ʼ��
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);

    TIM_Init(); //��ʱ������

     //��ʼ�����
    LED1_OFF;
    LED2_OFF;
    LIGHT_OFF;
    BEEP_OFF; 

    while (1)
    {
      

          if(Start == 0)    //      �ϵ�׼����
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
            if(Beep_Start > 5)      //��������0.3*5=1.5s
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
