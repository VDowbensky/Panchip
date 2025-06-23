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
uint32_t ADC_ExternaltriggerTestCase3(void)
{
    uint32_t AdcResult[100],cnt = 100;
    
    // Enable channel 0
    ADC_Open(ADC,ADC_TEST_CH);
    
    //Select ADC input range.1 means 0V~vddV ;0 means 0V~1.2V.
    //0V~1.2V & 0V~VDD both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_HIGH);  
    
    ADC_EnableHWTrigger(ADC, ADC_TRIGGER_BY_EXT_PIN, ADC_FALLING_EDGE_TRIGGER);       //ADC Hardware External trigger Enabled by PMW
    ADC_SequentialModeDisable(ADC);       //ADC sequential mode Disabled
    
	SYS_SET_MFP(P0, 5, EXT_STADC);  //Set P44 as external trigger channel
    GPIO_SetMode(P0, BIT5, GPIO_MODE_INPUT);
	
	SYS_SET_MFP(P2,2,PWM_CH0);
    // PWM0 frequency is 1200Hz, duty 30%,
    PWM_ConfigOutputChannel(PWM, 0, 1200, 30, OPERATION_EDGE_ALIGNED);
    // Enable output of 0 PWM channel
    PWM_EnableOutput(PWM, BIT0);
    // Start PWM
    PWM_Start(PWM, BIT0);

    // Power on ADC
    ADC_PowerOn(ADC);
//    ADC_SetExternalTriggerProcess();
		
//    while(ADC_IsBusy(ADC)){}
//    ADC_StartConvert(ADC);
	while(cnt--){
		while(!ADC_IsDataValid(ADC)){}
		AdcResult[99-cnt] = ADC_GetConversionData(ADC);
    }
	for(cnt=0;cnt<100;cnt++){
		SYS_TEST("%d,",AdcResult[cnt]);
		if(cnt % 10 == 9)
			SYS_TEST("\r\n");		
	}
	  ADC_PowerDown(ADC);

    return 0;
}

