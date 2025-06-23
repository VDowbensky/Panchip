/**************************************************************************//**
 * @file     wdt_clock_source_selection_test.c
 * @version  V1.0
 * $Date:    19/11/13 09:30 $
 * @brief    WDT test case 2, test changing WDT clock source from APB to LIRC.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wdt_common.h"


static void WDT_PrintInfoCase2(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                     | \n");
    SYS_TEST("|                                                             | \n");
    SYS_TEST("|    Input 'A'    16     times of LIRC (WDT_CLK) Period.      | \n");
    SYS_TEST("|    Input 'B'    4096   times of LIRC (WDT_CLK) Period.      | \n");
    SYS_TEST("|    Input 'C'    262144 times of LIRC (WDT_CLK) Period.      | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.        | \n");
    SYS_TEST("+-------------------------------------------------------------+ \n");
}

static void WDT_ClockSourceSelectionProc(uint32_t u32WdtClkSel, WDT_TimeoutDef TimeoutInterval)
{
    SYS_TEST("\nStart WDT Counting (CLock is 32KHz LIRC)...\n");

    // Unlock Regs before Enable WDT as several WDT Regs are write-protected
    SYS_UnlockReg();

    // Select Clock Source
    CLK_SetWdtClkSrc(u32WdtClkSel);

    // Enable WDT, disable Reset Mode, disable Wakeup Signal
    WDT_Open(TimeoutInterval, WDT_RESET_DELAY_1025CLK, FALSE, FALSE);

    size_t tout_cnt = 5;
    while(tout_cnt--)
    {
        // Pull down LA_INPUT_PIN to indicate WDT counting start
        LA_INPUT_PIN = 0;

        // Wait for timeout Event occur
        while(!WDT_GetTimeoutFlag());

        // Pull up LA_INPUT_PIN to indicate WDT timeout occurred
        LA_INPUT_PIN = 1;

        SYS_TEST("TIMEOUT\n");

        // Clear Timeout Event Flag
        WDT_ClearTimeoutFlag();

        // Reset Counter to restart counting
        WDT_ResetCounter();
    }

    // Disable WDT, need to ensure Regs are unlocked
    WDT_Close();

    // Re-lock Regs
    SYS_LockReg();
}

T_WDT_TEST_RESULT WDT_ClockSourceSelectionTestCase2(void)
{
    char c;
    while(1)
    {
        WDT_PrintInfoCase2();
        switch(c = getchar())
        {
        /* A. 16 times of LIRC (WDT_CLK) Period */
        case 'A':
        case 'a':
            WDT_ClockSourceSelectionProc(WDT_CLK_SRC_SEL_RCL, WDT_TIMEOUT_2POW4);
            break;
        /* B. 4096 times of LIRC (WDT_CLK) Period */
        case 'B':
        case 'b':
            WDT_ClockSourceSelectionProc(WDT_CLK_SRC_SEL_RCL, WDT_TIMEOUT_2POW12);
            break;
        /* C. 262144 times of LIRC (WDT_CLK) Period */
        case 'C':
        case 'c':
            WDT_ClockSourceSelectionProc(WDT_CLK_SRC_SEL_RCL, WDT_TIMEOUT_2POW18);
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
