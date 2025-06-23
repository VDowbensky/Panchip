/********************************************************* * 
@file		:clk_common.c 
@brief  	:common interface source file,store some common interface functions 
@author 	:zhongfeng
@version	:v1.0 
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"

uint32_t (*CLK_TestEntry[])(void) = 
{
    CLK_RchTestCase1,
    CLK_XthTestCase2,
    CLK_DpllTestCase3,
    CLK_RclTestCase4,
    CLK_XtlTestCase5,
	CLK_DividerTestCase6,
	CLK_DefaultRegValTestCase7,
};

void CLK_TestFunctionEnter(uint16_t Idx)
{
    (CLK_TestEntry[Idx])();
}

