/********************************************************* * 
@file		:clk_xtl_test.c 
@brief  	:clk xtl output test
@author 	:zhongfeng
@version	:v1.0 
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"

uint32_t CLK_XtlTestCase5(void)
{
	SYS_TEST("Low frequence clk select xtl,P11 pin output frequence xtl 32k \n");
	SYS_delay_10nop(100000);

	CLK-> XTL_CTRL_3V|= CLK_XTLCTL_XTL_EN_Msk_3v;
	while(!(CLK->XTL_CTRL_3V & CLK_XTLCTL_STABLE_Msk)){}
	CLK->CLK_TOP_CTRL_3V |= CLK_TOPCTL_32K_CLK_SEL_Msk_3v;

	SYS_delay_10nop(250);//Delay two 32K clock cycles
  CLK->RCL_CTRL_3V &=~CLK_RCLCTL_RC32K_EN_Msk_3v;
		
	SYS_SET_MFP(P1,1,CLK_32K);
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P1,1,GPIO);
	SYS_TEST("Finish \n");

	return 0; 

}

