/**************************************************************************//**
 * @file     wdt_common.c
 * @version  V1.0
 * $Date:    19/11/12 17:17 $
 * @brief    Common source file for WDT test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "wdt_common.h"


T_WDT_TEST_RESULT (*const WDT_TestCase[])(void) = 
{
    WDT_RegisterDefaultValueCheckCase0,
    WDT_TimeoutIntervalSelectionTestCase1,
    WDT_ClockSourceSelectionTestCase2,
    WDT_InterruptModeTestCase3,
    WDT_ResetModeTestCase4,
    WDT_WakeupSignalTestCase5,
};

void WDT_TestFunctionEnter(uint16_t TcIdx)
{
    T_WDT_TEST_RESULT r = WDT_TST_OK;

    if (TcIdx >= sizeof(WDT_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (WDT_TestCase[TcIdx])();
    if (r != WDT_TST_OK)
    {
        SYS_TEST("WDT Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("WDT Test OK, Success case: %d\n", TcIdx);
    }
}

// void SLEEP_IRQHandler(void)
// {
// 	LP_ClearWakeFlag(ANA,ANAC_INT_DP_FLAG_Msk|ANAC_INT_STANDBY_M1_FLAG_Msk|ANAC_INT_STANDBY_M0_FLAG_Msk);
// 	SYS_TEST("sleep irq \r\n");
// }

void WDT_IRQHandler(void)
{
	if(WDT_GetTimeoutIntFlag())
	{
		WDT_ClearTimeoutIntFlag();
		WDT_ClearTimeoutFlag();
		WDT_ClearTimeoutWakeupFlag();
	}

    // Pulse LA_INPUT_PIN to indicate WDT Int occurred
    LA_INPUT_PIN = 0;
    SYS_delay_10nop(1);
    LA_INPUT_PIN = 1;
    SYS_TEST("WDT INT\n");
    LA_INPUT_PIN = 1;

    // NVIC_DisableIRQ(WDT_IRQn);
	// WDT_Close();
    
}

void LP_IRQHandler(void)
{
	if(ANA->LP_INT_CTRL & ANAC_INT_DP_FLAG_Msk){
		SYS_TEST("Deepsleep \r\n");
	}
	LP_ClearWakeFlag(ANA,ANAC_INT_DP_FLAG_Msk|ANAC_INT_STANDBY_M1_FLAG_Msk|ANAC_INT_STANDBY_M0_FLAG_Msk);
}






void WDT_ResetRegionSelect(void)
{
	char input;
	uint32_t tmp_reg,value;
	
	SYS_UnlockReg();
	SYS_TEST("wdt reset region select\n");
	SYS_TEST("a:  only 1p2v region reset\n");
	SYS_TEST("b:  all chip reset without rom mode\n");
	input = getchar();
	switch(input){
		case 'A':
		case 'a':
			WDT->CTL &= ~WDT_CTL_RST_REGION_SEL_Msk;;
			break;
		case 'B':
		case 'b':
			WDT->CTL |= WDT_CTL_RST_REGION_SEL_Msk;;
			break;
		default:break;
	}
	tmp_reg = CLK->XTL_CTRL_3V;
	value = (tmp_reg & CLK_XTLCTL_XTL_CAP_EN_Msk_3v) >> CLK_XTLCTL_XTL_CAP_EN_Pos_3v;
	SYS_TEST("CAP_EN default value is %x \n", value);
	/*XTL cap enable signal is 3v area signal with value 0 at default mode, 
		we set this value to 1 to check 3v area reset or not */
	tmp_reg |= CLK_XTLCTL_XTL_CAP_EN_Msk_3v;
	CLK->XTL_CTRL_3V = tmp_reg;
	value = (tmp_reg & CLK_XTLCTL_XTL_CAP_EN_Msk_3v) >> CLK_XTLCTL_XTL_CAP_EN_Pos_3v;
	SYS_TEST("XTL_EN set value is %x \n", value);
}

void WDT_TestModuleInit(void)
{
    // Enable Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_WDT, ENABLE);     //Enable WDT Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);    //Enalbe Timer0 Clock for TIMER_Delay()

    // Config Pinmux of Auxiliary GPIO for Timing Measure by Logic Analyzer
    SYS_SET_MFP(P2, 0, GPIO);
    GPIO_SetMode(P2, BIT0, GPIO_MODE_OUTPUT);
    /* NOTE: We should make sure LA_INPUT_PIN in wdt_common.h is defined as the same pin (P12) !!! */
}

T_WDT_TEST_RESULT WDT_RegisterDefaultValueCheckCase0(void)
{
    SYS_TEST("WDT Register Default Values:\n");
    SYS_TEST("---------------------------\n");
    SYS_TEST("CTL           = 0x%08x\n", WDT->CTL     );
    SYS_TEST("ALTCTL        = 0x%08x\n", WDT->ALTCTL  );

    SYS_TEST("\nWDT Flags: TOF=%d, IF=%d, RSTF=%d, WKF=%d\n", WDT_GetTimeoutFlag(),
            WDT_GetTimeoutIntFlag(), WDT_GetResetFlag(), WDT_GetTimeoutWakeupFlag());
    SYS_TEST("---------------------------\n");

    return WDT_TST_OK;
}
