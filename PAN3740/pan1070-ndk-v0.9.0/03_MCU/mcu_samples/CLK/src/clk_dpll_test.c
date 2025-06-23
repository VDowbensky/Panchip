/********************************************************* *
@file		:clk_dpll_test.c
@brief  	:clk dpll output test
@author 	:zhongfeng
@version	:v1.0
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"

uint32_t CLK_DpllTestCase3(void)
{

	SYS_TEST("Dpll src select rch, output 64M,P06 output 64M/8 \n");
	SYS_TEST("Dpll src select rch, output 48M,P06 output 48M/8\n");
	SYS_TEST("Dpll src select xth, output 64M,P06 output 64M/8 \n");
	SYS_TEST("Dpll src select xth, output 48M,P06 output 48M/8 \n");
	SYS_delay_10nop(100000);

	// This is used to support the P23 pin DPLL_DIV8 function
	CLK->DPLL_CTRL |= CLK_DPLLCTL_DPLL_TST_EN_Msk;
	CLK->AHB_CLK_CTRL |= CLK_AHBCLK_DPLL_CLK_TST_EN_Msk;
	
	SYS_SET_MFP(P0, 6, AHB_CLK);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_RCH);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_RCH, CLK_DPLL_OUT_64M);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	CLK_HCLKConfig(7);
	SYS_SET_MFP(P2, 3, DPLL_DIV8);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P2, 3, GPIO);
	SYS_SET_MFP(P0, 6, GPIO);
	SYS_delay_10nop(100000);

	// SYS_TEST("Dpll src select rch, output 48M,P06 output 48M/8\n");
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P0, 6, AHB_CLK);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_RCH);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_RCH, CLK_DPLL_OUT_48M);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	CLK_HCLKConfig(7);
	SYS_SET_MFP(P2, 3, DPLL_DIV8);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P2, 3, GPIO);
	SYS_SET_MFP(P0, 6, GPIO);
	SYS_delay_10nop(100000);

	// SYS_TEST("Dpll src select xth, output 64M,P06 output 64M/8 \n");
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P0, 6, AHB_CLK);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_64M);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	CLK_HCLKConfig(7);
	SYS_SET_MFP(P2, 3, DPLL_DIV8);
	SYS_delay_10nop(100000);
	Peri_Init();
	SYS_SET_MFP(P2, 3, GPIO);
	SYS_SET_MFP(P0, 6, GPIO);
	SYS_delay_10nop(100000);

	// SYS_TEST("Dpll src select xth, output 48M,P06 output 48M/8 \n");
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P0, 6, AHB_CLK);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_XTH);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_48M);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
	CLK_HCLKConfig(7);
	SYS_SET_MFP(P2, 3, DPLL_DIV8);
	Peri_Init();
	SYS_delay_10nop(100000);
	SYS_SET_MFP(P2, 3, GPIO);
	SYS_SET_MFP(P0, 6, GPIO);
	return 0;
}
