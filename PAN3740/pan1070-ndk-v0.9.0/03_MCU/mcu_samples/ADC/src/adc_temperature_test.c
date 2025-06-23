/********************************************************* * 
@file		:adc_convert_test.c 
@brief  	:adc convert function test
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/10/28
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#include "PanSeries.h"
#include "adc_common.h"

/*
  * @brief     :adc Temperature Test
  * @retval    :0/1
*/
uint32_t ADC_TemperatureTestCase6(void)
{
    float temperature;

    // Initialize ADC module
    if (!ADC_Init(ADC, true)) {
        SYS_TEST("Error: ADC init fail! Check if SoC hardware calibration data is successfully loaded!\n");
        return 1;
    }

    // Change ADC Ref to ADC-VBG (1.2v) if is not
    if (ADC->CTL2 & ADC_SEL_VREF_Msk) {
        ADC_SelInputRange(ADC, ADC_INPUTRANGE_LOW);
    }

    // Measure temperature several times with 1s interval
    temperature = ADC_MeasureSocTemperature(ADC);
    SYS_TEST("Temperature = %.2f C\n", temperature);
    for (size_t i = 0; i < 9; i++) {
        TIMER_Delay(TIMER0, 1 * 1000 * 1000);   // Delay 1s
        temperature = ADC_MeasureSocTemperature(ADC);
        SYS_TEST("Temperature = %.2f C\n", temperature);
    }

    return 0;
}
