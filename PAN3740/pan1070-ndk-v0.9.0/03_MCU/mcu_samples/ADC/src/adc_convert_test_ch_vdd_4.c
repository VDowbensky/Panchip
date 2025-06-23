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


uint32_t ADC_ConvertTestChVDD4Case10(void)
{
    uint16_t vdd_voltage_mv;

    // Initialize ADC module
    if (!ADC_Init(ADC, false)) {
        SYS_TEST("Error: ADC init fail! Check if SoC hardware calibration data is successfully loaded!\n");
        return 1;
    }

    // Change ADC Ref to ADC-VBG (1.2v) if is not
    if (ADC->CTL2 & ADC_SEL_VREF_Msk) {
        ADC_SelInputRange(ADC, ADC_INPUTRANGE_LOW);
    }

    // Measure temperature several times with 1s interval
    vdd_voltage_mv = ADC_MeasureSocVbat(ADC);
    SYS_TEST("SoC VDD = %d mV\n", vdd_voltage_mv);
    for (size_t i = 0; i < 9; i++) {
        TIMER_Delay(TIMER0, 1 * 1000 * 1000);   // Delay 1s
        vdd_voltage_mv = ADC_MeasureSocVbat(ADC);
        SYS_TEST("SoC VDD = %d mV\n", vdd_voltage_mv);
    }

    return 0;
}
