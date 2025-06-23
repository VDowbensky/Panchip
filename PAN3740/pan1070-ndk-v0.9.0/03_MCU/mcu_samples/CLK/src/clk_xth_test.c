/********************************************************* * 
@file		:clk_xth_test.c 
@brief  	:clk xth output test
@author 	:zhongfeng
@version	:v1.0 
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"



uint32_t CLK_XthTestCase2(void)
{
	SYS_TEST("\nSystem clk select xth,P06 pin output frequence xth32M/16 \n");
	SYS_delay_10nop(100000);

	SYS_SET_MFP(P2,7,XTH);
	SYS_SET_MFP(P0,6,AHB_CLK);
	CLK_HCLKConfig(15);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	// CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_RCH);

	Peri_Init();
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P0,6,GPIO);
	SYS_SET_MFP(P2,7,GPIO);
	SYS_delay_10nop(100000);
//	CLK_HCLKConfig(0);
//	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	Peri_Init();
	return 0; 

}

