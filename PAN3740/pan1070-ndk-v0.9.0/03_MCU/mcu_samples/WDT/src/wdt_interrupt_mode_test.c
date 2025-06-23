/**************************************************************************//**
 * @file     wdt_interrupt_mode_test.c
 * @version  V1.0
 * $Date:    19/11/13 10:08 $
 * @brief    WDT test case 3, test WDT Interrupt Mode.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wdt_common.h"


static void WDT_PrintInfoCase3(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                     | \n");
    SYS_TEST("|                                                             | \n");
    SYS_TEST("|    Input 'A'    Enable WDT Interrupt.                       | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.        | \n");
    SYS_TEST("+-------------------------------------------------------------+ \n");
}

static void WDT_InterruptProc(void)
{
    // Pull down LA_INPUT_PIN to indicate Interrupt Procedure start
    LA_INPUT_PIN = 0;

    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz, TimeoutCnt = 4096)...\n", CLK_GetPCLK1Freq());

    // Unlock Regs before Enable WDT as several WDT Regs are write-protected
    SYS_UnlockReg();

    // Select Clock Source
    CLK_SetWdtClkSrc(WDT_CLK_SRC_SEL_APBDIV);

    // Enable WDT, Disable Reset Mode, disable Wakeup Signal
    WDT_Open(WDT_TIMEOUT_2POW12, WDT_RESET_DELAY_1025CLK, FALSE, FALSE);
    // Enable WDT INT, Note this API should be called AFTER WDT_Open()

		WDT_EnableInt();
    // Enable NVIC WDT IRQ
    NVIC_EnableIRQ(WDT_IRQn);

    // Pull up LA_INPUT_PIN to indicate WDT counting start
    LA_INPUT_PIN = 1;
    // Delay 1s
    TIMER_Delay(TIMER0, 1000000);
    // Pull down LA_INPUT_PIN to indicate WDT is going to closed
    LA_INPUT_PIN = 0;

    // Disable NVIC WDT IRQ
    NVIC_DisableIRQ(WDT_IRQn);
    // Disable WDT INT
    WDT_DisableInt();
    // Disable WDT, need to ensure Regs are unlocked
    WDT_Close();

    // Re-lock Regs
    SYS_LockReg();
    LA_INPUT_PIN = 1;
}

T_WDT_TEST_RESULT WDT_InterruptModeTestCase3(void)
{
    char c;
    while(1)
    {
        WDT_PrintInfoCase3();
        switch(c = getchar())
        {
        /* A. Enable WDT Interrupt */
        case 'A':
        case 'a':
            WDT_InterruptProc();
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
