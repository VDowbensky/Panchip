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


uint32_t ADC_ConvertTestChVbgCase9(void)
{
    uint32_t AdcResult[100] = {0,};
    
	//en buffer(necessary)
	ADC->BV_CTL |= 0x2; 
    // Enable channel 0
    ADC_Open(ADC,ADC_CHEN_CHEN8_VBG_Msk);
	
    //Select ADC input range.1 means 0V~vddV ;0 means 0V~1.2V.
    //0V~1.2V & 0V~VDD both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_HIGH);  

    // Power on ADC
    ADC_PowerOn(ADC);
	  uint32_t cnt = 100;
	  while(cnt--){
			ADC_StartConvert(ADC);
			while(!ADC_IsDataValid(ADC)){}
			AdcResult[99-cnt] = ADC_GetConversionData(ADC);
		}
    
		for(cnt = 0;cnt < 100;cnt++){
			SYS_TEST("%d ",AdcResult[cnt]);
			if(cnt % 10 == 9)
				SYS_TEST("\r\n");
		}
		SYS_TEST("ADC convert test finish \r\n");
		ADC_PowerDown(ADC);
    return 0;
}
