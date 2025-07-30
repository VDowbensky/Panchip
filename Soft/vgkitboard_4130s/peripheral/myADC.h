#ifndef __MY_ADC_H
#define __MY_ADC_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

void myADC_init(void);
uint16_t myADC_getValue(void);
float myADC_getVoltageValue(void);
#endif

