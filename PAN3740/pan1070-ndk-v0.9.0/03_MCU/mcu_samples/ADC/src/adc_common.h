/********************************************************* * 
@file		:adc_common.c 
@brief  	:common interface header file 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#ifndef __ADC_USR_H__
#define __ADC_USR_H__

#define BUFSIZE                         (256)
#define __BYTE_LEN	                    (32)

#define ADC_TEMPERATURE_CH	(ADC_CHEN_CH9_TMP_Msk)
#define ADC_TEST_CH			(ADC_CHEN_CHEN5_Msk)
#define ADC_CH_NUM			(ADC_CHEN_CHEN5_Pos)

extern volatile bool gTfrFlag,gRxTfrFlag,gTfrFlag1;
extern volatile bool compare_finish_flag;

void FillBuffer(uint8_t *pBuffer, uint32_t size);
void delay(uint32_t t);
void I2C_InitRole(I2C_T * I2Cx, uint16_t role,uint8_t Adr10BitEn);

uint32_t ADC_RegisterDefaultCheckCase1(void);
uint32_t ADC_ConvertTestCase2(void);
uint32_t ADC_ExternaltriggerTestCase3(void);
uint32_t ADC_CompareTestCase4(void);
uint32_t ADC_PwmSequentialTestCase5(void);
uint32_t ADC_TemperatureTestCase6(void);
uint32_t ADC_OneAdcChSequentialTestCase7(void);
uint32_t ADC_LeftShiftTestCase8(void);
uint32_t ADC_ConvertTestChVbgCase9(void);
uint32_t ADC_ConvertTestChVDD4Case10(void);
void ADC_SetExternalTriggerProcess(void);
void ADC_TestFunctionEnter(uint16_t Idx);
#endif

