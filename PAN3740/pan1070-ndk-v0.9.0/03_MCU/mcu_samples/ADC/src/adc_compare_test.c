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
  * @brief	   :iic register default value check
  * @retval    :0/1
*/
uint32_t ADC_CompareTestCase4(void)
{
//    uint32_t AdcResult = 0;
    
    // Enable channel 0
    ADC_Open(ADC,ADC_TEST_CH);
    //Select ADC input range.1 means 0V~vddV ;0 means 0V~1.2V.
    //0V~1.2V & 0V~VDD both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_HIGH);   
    // Power on ADC
    ADC_PowerOn(ADC);
	
    // Configure and enable Comparator 0 to monitor channel 0 input less than 2000(1.8v input voltage)
    ADC_CompareEnable(ADC, ADC_CH_NUM, ADC_CMP0_LESS_THAN, 2000, 1,ADC_COMPARATOR_0);
    // Configure and enable Comparator 1 to monitor channel 0 input greater or equal to 2000
    ADC_CompareEnable(ADC, ADC_CH_NUM, ADC_CMP1_GREATER_OR_EQUAL_TO, 2000, 1,ADC_COMPARATOR_1);

    // Enable ADC comparator 0 and 1 interrupt
    ADC_EnableInt(ADC, ADC_CMP0_INT);
    ADC_EnableInt(ADC, ADC_CMP1_INT);
	ADC_IntMask(ADC,ADC_STATUS_INTMSK_CMP0_Msk|ADC_STATUS_INTMSK_CMP1_Msk,DISABLE);
    NVIC_EnableIRQ(ADC_IRQn);        

	ADC_StartConvert(ADC);
	while(!compare_finish_flag){}
    compare_finish_flag = false;
		
    SYS_TEST("Compare AdcResult :%d \r\n",ADC_GetConversionData(ADC));
	ADC_PowerDown(ADC);	
    return 0;                  
}

