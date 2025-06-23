/********************************************************* * 
@file		:adc_data_left_shift_test.c 
@brief  	:adc data_left_shift function test
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/10/28
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#include "PanSeries.h"
#include "adc_common.h"


/*
  * @brief	   :adc register default value check
  * @retval    :0/1
*/
uint32_t ADC_LeftShiftTestCase8(void)
{
    uint32_t RcvDataBuf[128] = {0};
	uint32_t i = 0,j=0,tmp_data;

	ADC_Open(ADC,ADC_TEST_CH);
    //Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
    //0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_HIGH);   
    ADC_PowerOn(ADC);

	ADC_SeqOneChModeConfig(ADC,ADC_FALLING_EDGE_TRIGGER,ADC_FIFO_TRIG_LEVEL_HALF,0,1);
    // TRG1CTL  select for 1-shunt sequential mode
    ADC_Trigger2Select(ADC,DISABLE);
    // ADC PWM first Trigger Type and Source Selection, PWM0 Falling for ADC one channel;
    ADC_SeqModeTriggerSrc(ADC, ADC_SEQMODE_PWM0_PERIOD);
    // Delay ADC start conversion time after PWM trigger
    ADC_TriggerDelay(ADC, 0x0a);     
	ADC_SetExtraSampleTime(ADC,0x20);
	
	SYS_TEST("Press key to set left shift or not \n");
	SYS_TEST("Press 0 to set left shift \n");
	SYS_TEST("Press 1 to set 0x888 bias \n");
	SYS_TEST("Press 2 to set left shift  and 0x888 bias\n");

	char input = getchar();
	
	if(input == '0'){
		ADC_LeftShiftEn(ADC,ENABLE);
	}
	else if(input == '1'){
		ADC_SubtractBiasEn(ADC,ENABLE);
		ADC_SetBiasData(ADC,0x888);
	}
	else if(input == '2'){
		ADC_LeftShiftEn(ADC,ENABLE);	
		ADC_SubtractBiasEn(ADC,ENABLE);
		ADC_SetBiasData(ADC,0x888);	
	}
    // PWM0 frequency is 1200Hz, duty 30%,
    PWM_ConfigOutputChannel(PWM, 0, 1200, 30, OPERATION_EDGE_ALIGNED);
    // Enable output of 0 PWM channel
    PWM_EnableOutput(PWM, BIT0);
    // Start PWM
    PWM_Start(PWM, BIT0);

	while (i < 100)
	{
		if (ADC_StatusFlag(ADC, ADC_STATUS_FLAG_HALF_Msk))
		{
			//			PWM_ForceStop(PWM,BIT0);
			while (j < 8)
			{
				RcvDataBuf[i++] = ADC_GetConversionData(ADC);
				j++;
			}
			j = 0;
			ADC_ClearStatusFlag(ADC, ADC_STATUS_FLAG_HALF_Msk);
			//			PWM_Start(PWM, BIT0);
		}
	}

	ADC_PowerDown(ADC);
	PWM_ForceStop(PWM, BIT0);

	ADC_LeftShiftEn(ADC, DISABLE);
	ADC_SubtractBiasEn(ADC, DISABLE);
	for (uint8_t cnt = 0; cnt < 100; cnt++)
	{
		SYS_TEST("%x ", RcvDataBuf[cnt]);
		if (cnt % 10 == 9)
			SYS_TEST("\r\n");
	}
	return 0;
}
