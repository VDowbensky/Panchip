/********************************************************* * 
@file		:adc_default_value.c 
@brief  	:adc register default value check
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/10/28
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "adc_common.h"

const uint32_t AdcDefaultValue[]=
{
    0x0,                    //ADC->DAT  
    0x08000000,                    //ADC->CTL
    0x0,                    //ADC->CHEN
    0x0,                    //ADC->CMP0
    0x0,                    //ADC->CMP1
    0x0,                    //ADC->STATUS
    0x0,                    //ADC->TRGDLY
    0x200,                  //ADC->EXTSMPT
    0x0,                    //ADC->SEQCTL
    0x0,                    //ADC->SEQDAT1
    0x0,                    //ADC->SEQDAT2
    0x75520A00              //ADC->CTL2            
};


/*
  * @brief	   :adc register default value check
  * @retval    :0/1
*/
uint32_t ADC_RegisterDefaultCheckCase1(void)
{
    uint8_t i = 0,different = 0;

    if(ADC->DAT     != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->DAT     expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->DAT    );}
    if(ADC->CTL     != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->CTL     expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->CTL    );}
    if(ADC->CHEN    != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->CHEN    expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->CHEN   );}
    if(ADC->CMP0    != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->CMP0    expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->CMP0   );}
    if(ADC->CMP1    != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->CMP1    expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->CMP1   );}
    if(ADC->STATUS  != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->STATUS  expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->STATUS );}
    if(ADC->TRGDLY  != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->TRGDLY  expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->TRGDLY );}
    if(ADC->EXTSMPT != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->EXTSMPT expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->EXTSMPT);}
    if(ADC->SEQCTL  != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->SEQCTL  expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->SEQCTL );}
    if(ADC->SEQDAT1 != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->SEQDAT1 expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->SEQDAT1);}
    if(ADC->SEQDAT2 != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->SEQDAT2 expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->SEQDAT2);}
    if(ADC->CTL2    != AdcDefaultValue[i++]) {different++;SYS_TEST("ADC->CTL2    expect:%08x,current:%08x \r\n",AdcDefaultValue[i-1],ADC->CTL2   );}
    
    if(!different){
       SYS_TEST("ADC default value check ok \r\n"); 
    }
    return 0;
}

