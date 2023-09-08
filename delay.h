/* delay.h 公共函数文件 */

#ifndef _DELAY_H
#define _DELAY_H

#include "stm8s.h"



//函数声明
void delay_init(u8 clk);
void delay_us(u16 nus); //延时us
void delay_ms(u32 nms); //延时ms

u16 Get_decimal(double dt, u8 deci);     //获得数值小数部分函数

#endif

