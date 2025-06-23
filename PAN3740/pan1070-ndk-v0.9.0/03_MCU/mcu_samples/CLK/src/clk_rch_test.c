/********************************************************* * 
@file		:clk_rch_test.c 
@brief  	:clk rch output test
@author 	:zhongfeng
@version	:v1.0 
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"



uint32_t CLK_RchTestCase1(void)
{
	SYS_TEST("System clk select rch,P17 pin output frequence rch32M \n");
	SYS_delay_10nop(100000);

	SYS_SET_MFP(P0, 6, AHB_CLK);
	SYS_SET_MFP(P1,7,RCH);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_RCH);
	CLK_HCLKConfig(7);	
	Peri_Init();
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P1,7,GPIO);
	SYS_SET_MFP(P0, 6, GPIO);
	return 0; 

}

