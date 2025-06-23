/**************************************************************************//**
 * @file     wdt_interrupt_mode_test.c
 * @version  V1.0
 * $Date:    19/11/13 10:46 $
 * @brief    WDT test case 4, test WDT Reset Mode.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wdt_common.h"


static void WDT_PrintInfoCase4(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                             | \n");
    SYS_TEST("|                                                                     | \n");
    SYS_TEST("|    Input 'A'    Enable WDT Reset, delay 1025 times of WDT_CLK.      | \n");
    SYS_TEST("|    Input 'B'    Enable WDT Reset, delay 129  times of WDT_CLK.      | \n");
    SYS_TEST("|    Input 'C'    Enable WDT Reset, delay 17   times of WDT_CLK.      | \n");
    SYS_TEST("|    Input 'D'    Enable WDT Reset, delay 2    times of WDT_CLK.      | \n");
    SYS_TEST("|    Input 'E'    Enable WDT Reset and feed WDT before timeout.       | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.                | \n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
}

static uint32_t GetResetCountFromConfig(uint32_t ConfigValue)
{
    switch (ConfigValue)
    {
    case 0:
        return 1025;
    case 1:
        return 129;
    case 2:
        return 17;
    case 3:
        return 2;
    default:
        return NULL;
    }
}

static void WDT_ResetProc(WDT_ResetDelayDef ResetDelay)
{
    // Pull down LA_INPUT_PIN to indicate Reset Procedure start
    LA_INPUT_PIN = 0;

    SYS_TEST("\nWDT Reset Delay Count = %d\n", GetResetCountFromConfig(ResetDelay));
    SYS_TEST("Start WDT Counting (APB1_CLK = %dHz, TimeoutCnt = 4096)...\n", CLK_GetPCLK1Freq());

    // Unlock Regs before Enable WDT as several WDT Regs are write-protected
    SYS_UnlockReg();

    // Select Clock Source
    CLK_SetWdtClkSrc(WDT_CLK_SRC_SEL_APBDIV);

    // Enable WDT, ENABLE Reset Mode, disable Wakeup Signal
    WDT_Open(WDT_TIMEOUT_2POW12, ResetDelay, TRUE, FALSE);

    WDT_EnableInt();
    NVIC_EnableIRQ(WDT_IRQn);

    // Pull up LA_INPUT_PIN to indicate WDT counting start
    LA_INPUT_PIN = 1;

    // Re-lock Regs
    SYS_LockReg();

    // Wait for WDT Reset
    while(1);
}

static void WDT_FeedProc(void)
{
    // Pull down LA_INPUT_PIN to indicate entering WDT Feed Procedure
    LA_INPUT_PIN = 0;

    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz, TimeoutCnt = 4096)...\n", CLK_GetPCLK1Freq());

    // Unlock Regs before Enable WDT as several WDT Regs are write-protected
    SYS_UnlockReg();

    // Select Clock Source
    CLK_SetWdtClkSrc(WDT_CLK_SRC_SEL_APBDIV);

    // Enable WDT, ENABLE Reset Mode, disable Wakeup Signal
    WDT_Open(WDT_TIMEOUT_2POW12, WDT_RESET_DELAY_2CLK, TRUE, FALSE);

    // Re-lock Regs
    SYS_LockReg();

    // Wait for WDT Reset
    size_t tout_cnt = 5;
    while(tout_cnt--)
    {
       // Pull up LA_INPUT_PIN to indicate WDT counting start
        LA_INPUT_PIN = 1;
        // Delay 50ms
        TIMER_Delay(TIMER0, 50000);
        // About to feed WDT
        SYS_TEST("Feed WDT\n");
        // Unlock Regs
        SYS_UnlockReg();
        // Feed Watchdog
        WDT_ResetCounter();
        // Delay some nops before Re-lock Regs
        SYS_delay_10nop(1);
        // Re-Lock Regs
        SYS_LockReg();
        // Pull down LA_INPUT_PIN to indicate WDT counting restart
        LA_INPUT_PIN = 0;
    }

    // Wait for timeout
    while(1);
}

T_WDT_TEST_RESULT WDT_ResetModeTestCase4(void)
{
    char c;
    while(1)
    {
        WDT_PrintInfoCase4();
		c = getchar();
        switch(c)
        {
        /* A. Enable WDT Reset, delay 1025 times of WDT_CLK */
        case 'A':
        case 'a':
            WDT_ResetProc(WDT_RESET_DELAY_1025CLK);
            break;
        /* B. Enable WDT Reset, delay 129 times of WDT_CLK */
        case 'B':
        case 'b':
            WDT_ResetProc(WDT_RESET_DELAY_129CLK);
            break;
        /* C. Enable WDT Reset, delay 17 times of WDT_CLK */
        case 'C':
        case 'c':
            WDT_ResetProc(WDT_RESET_DELAY_17CLK);
            break;
        /* D. Enable WDT Reset, delay 2 times of WDT_CLK */
        case 'D':
        case 'd':
            WDT_ResetProc(WDT_RESET_DELAY_2CLK);
            break;
        /* E. Enable WDT Reset and feed WDT before timeout */
        case 'E':
        case 'e':
            WDT_FeedProc();
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return WDT_TST_OK;
}
