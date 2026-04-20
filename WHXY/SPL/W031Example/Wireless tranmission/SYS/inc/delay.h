#ifndef __DELAY_H
#define __DELAY_H

#include "cw32w031_rcc.h"
#include "cw32w031_systick.h"


void delay1ms(uint32_t u32Cnt);
void delay100us(uint32_t u32Cnt);
void delay10us(uint32_t u32Cnt);
void delay1us(uint32_t u32Cnt);


#endif

