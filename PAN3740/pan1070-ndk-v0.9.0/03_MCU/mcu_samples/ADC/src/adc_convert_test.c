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

#define ADC_CONV_CODE_TOTAL_CNT     128
#define ADC_CONV_CODE_TRIM_CNT      16


uint32_t ADC_ConvertTestCase2(void)
{
    uint16_t code_buf[ADC_CONV_CODE_TOTAL_CNT];
    uint32_t result_code;
    float voltage;
    char input;

    SYS_TEST("adc clk source select\n");
    SYS_TEST("a:  clk source select ahb\n");
    SYS_TEST("b:  clk source select xth\n");
    input = getchar();
    switch(input){
        case 'A':
        case 'a':
            CLK->APB1_CLK_CTRL &= ~CLK_APB1CLK_ADC_CLK_SEL_Msk;
            break;
        case 'B':
        case 'b':
            CLK->APB1_CLK_CTRL |= CLK_APB1CLK_ADC_CLK_SEL_Msk;
            break;
        default:break;
    }

    // Initialize ADC module
    if (!ADC_Init(ADC, true)) {
        SYS_TEST("Error: ADC init fail! Check if SoC hardware calibration data is successfully loaded!\n");
        return 1;
    }

    // Enable channel 5
    ADC_Open(ADC,ADC_TEST_CH);

    //Select ADC input range.1 means 0V~vddV ;0 means 0V~1.2V.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_LOW);

    // Prepare ADC vbg calib paramter due to currnet soc vbat after setting input range to the 0~1.2v ref
    ADC_PrepareVbgCalibData(ADC);
    
    // Power on ADC
    ADC_PowerOn(ADC);

    // Delay a while to wait adc stable
    SYS_delay_10nop(50000);

    // Start ADC sampling
    result_code = ADC_SamplingCodeTrimMean(ADC, code_buf, ADC_CONV_CODE_TOTAL_CNT, ADC_CONV_CODE_TRIM_CNT);
    // Convert ADC sampling code to voltage
    voltage = ADC_OutputVoltage(ADC, result_code);

    // Power down ADC
    ADC_PowerDown(ADC);

    SYS_TEST("\nADC code result = %d\n", result_code);
    SYS_TEST("ADC voltage result = %.1f mV\n", voltage);

    return 0;
}

