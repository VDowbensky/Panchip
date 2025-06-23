/********************************************************* * 
@file		:adc_common.c 
@brief  	:common interface header file 
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#ifndef __CLK_USR_H__
#define __CLK_USR_H__

void Peri_Init(void);

uint32_t CLK_RchTestCase1(void);
uint32_t CLK_XthTestCase2(void);
uint32_t CLK_DpllTestCase3(void);
uint32_t CLK_RclTestCase4(void);
uint32_t CLK_XtlTestCase5(void);
uint32_t CLK_DividerTestCase6(void);
uint32_t CLK_DefaultRegValTestCase7(void);


void CLK_TestFunctionEnter(uint16_t Idx);
#endif

