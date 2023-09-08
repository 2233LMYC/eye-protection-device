/* delay.h ���������ļ� */

#ifndef _DELAY_H
#define _DELAY_H

#include "stm8s.h"



//��������
void delay_init(u8 clk);
void delay_us(u16 nus); //��ʱus
void delay_ms(u32 nms); //��ʱms

u16 Get_decimal(double dt, u8 deci);     //�����ֵС�����ֺ���

#endif

