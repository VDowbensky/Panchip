/********************************************************* * 
@file		:clk_divider_test.c 
@brief  	:clk divider function test
@author 	:zhongfeng
@version	:v1.0 
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"



uint32_t CLK_DividerTestCase6(void)
{

	SYS_TEST("clk src select xth, output 32M,P06 pin output frequence xth32M/4 \n");
	SYS_TEST("clk src select xth, output 32M,P06 pin output frequence xth32M/8 \n");
	SYS_TEST("clk src select xth, output 32M,P06 pin output frequence xth32M/16 \n");
	SYS_TEST("clk src select dpll, output 48M,P06 pin output frequence 48M/4 \n");
	SYS_TEST("clk src select dpll, output 48M,P06 pin output frequence 48M/8 \n");
	SYS_TEST("clk src select dpll, output 48M,P06 pin output frequence 48M/16 \n");




	SYS_delay_10nop(100000);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_HCLKConfig(3);
	// CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_RCH,CLK_DPLL_OUT_32M); 
	// CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	SYS_SET_MFP(P0,6,AHB_CLK);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P0,6,GPIO);
	SYS_delay_10nop(100000);
	
	// SYS_TEST("clk src select xth, output 32M,P06 pin output frequence xth64M/8 \n");
	SYS_delay_10nop(100000);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_HCLKConfig(7);
	// CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_RCH,CLK_DPLL_OUT_32M); 
	// CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	SYS_SET_MFP(P0,6,AHB_CLK);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P0,6,GPIO);
	SYS_delay_10nop(100000);

	// SYS_TEST("clk src select xth, output 32M,P06 pin output frequence xth64M/16 \n");
	SYS_delay_10nop(100000);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_HCLKConfig(15);
	// CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_RCH,CLK_DPLL_OUT_32M); 
	// CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	SYS_SET_MFP(P0,6,AHB_CLK);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P0,6,GPIO);
	SYS_delay_10nop(100000);

	// SYS_TEST("clk src select dpll, output 48M,P06 pin output frequence 48M/4 \n");
	SYS_delay_10nop(100000);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_HCLKConfig(3);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M); 
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	SYS_SET_MFP(P0,6,AHB_CLK);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P0,6,GPIO);
	SYS_delay_10nop(100000);
	
	// SYS_TEST("clk src select dpll, output 48M,P06 pin output frequence 48M/8 \n");
	SYS_delay_10nop(100000);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_HCLKConfig(7);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M); 
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	SYS_SET_MFP(P0,6,AHB_CLK);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P0,6,GPIO);
	SYS_delay_10nop(100000);

	// SYS_TEST("clk src select dpll, output 48M,P06 pin output frequence 48M/16 \n");
	SYS_delay_10nop(100000);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_HCLKConfig(15);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M); 
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	SYS_SET_MFP(P0,6,AHB_CLK);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P0,6,GPIO);
	SYS_delay_10nop(100000);
	
	CLK_HCLKConfig(0);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	Peri_Init();
	return 0; 

}

