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
uint32_t ADC_PwmSequentialTestCase5(void)
{
    //Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
    //0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
    ADC_SelInputRange(ADC,ADC_INPUTRANGE_HIGH);   
    // Power on ADC
    ADC_PowerOn(ADC);

    // ADC Hardware External trigger Enabled by PMW
    ADC_EnableHWTrigger(ADC, ADC_TRIGGER_BY_PWM, ADC_FALLING_EDGE_TRIGGER);
    // ADC Sequential mode & type Enable
    ADC_SeqModeEnable(ADC,ADC_SEQMODE_TYPE_23SHUNT,ADC_SEQMODE_MODESELECT_CH12);
    // Delay ADC start conversion time after PWM trigger
    ADC_TriggerDelay(ADC, 0x0a);
    // TRG1CTL  select for 1-shunt sequential mode
    ADC_Trigger2Select(ADC,ENABLE);
    // ADC PWM first Trigger Type and Source Selection, PWM0 Falling for both ADC channel 0&1;
    ADC_SeqModeTriggerSrc(ADC, ADC_SEQMODE_PWM0_FALLING);
        
    // PWM0 frequency is 1200Hz, duty 30%,
    PWM_ConfigOutputChannel(PWM, 0, 1200, 30, OPERATION_EDGE_ALIGNED);
    // Enable output of 0 PWM channel
    PWM_EnableOutput(PWM, BIT0);
    // Start PWM
    PWM_Start(PWM, BIT0);        
    
    SYS_delay_10nop(1000000);
    SYS_TEST("+---------------------------------------------------+\r\n");
    SYS_TEST("|A/D PWM Sequential Mode First Result  is %d        |\r\n", ADC->SEQDAT1&0xfff);
    SYS_TEST("|A/D PWM Sequential Mode Second Result is %d        |\r\n", ADC->SEQDAT2&0xfff);
    SYS_TEST("+---------------------------------------------------+\r\n");
    ADC_PowerDown(ADC);

    return 0;                  
}

