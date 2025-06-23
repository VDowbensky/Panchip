/**************************************************************************//**
 * @file     wdt_timeout_interval_selection_test.c
 * @version  V1.0
 * $Date:    19/11/12 17:23 $
 * @brief    WDT test case 1, test different Timeout Interval Configurations.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wdt_common.h"


static void WDT_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                               | \n");
    SYS_TEST("|                                                                       | \n");
    SYS_TEST("|    Input 'A'    4th  power of 2 (16)       times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'B'    6th  power of 2 (64)       times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'C'    8th  power of 2 (256)      times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'D'    10th power of 2 (1024)     times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'E'    12th power of 2 (4096)     times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'F'    14th power of 2 (16384)    times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'G'    15th power of 2 (32768)    times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'H'    16th power of 2 (65536)    times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'I'    17th power of 2 (131072)   times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'J'    18th power of 2 (262144)   times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'K'    19th power of 2 (524288)   times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'L'    20th power of 2 (1048576)  times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'M'    21st power of 2 (2097152)  times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'N'    22nd power of 2 (4194304)  times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'O'    23rd power of 2 (8388608)  times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Input 'P'    24th power of 2 (16777216) times of WDT_CLK Period.   | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.                  | \n");
    SYS_TEST("+-----------------------------------------------------------------------+ \n");
}


static void WDT_TimeoutSelectionProc(WDT_TimeoutDef TimeoutInterval)
{
    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz)...\n", CLK_GetPCLK1Freq());

    // Unlock Regs before Enable WDT as several WDT Regs are write-protected
    SYS_UnlockReg();

    // Select Clock Source
    CLK_SetWdtClkSrc(WDT_CLK_SRC_SEL_APBDIV);

    // Enable WDT, disable Reset Mode, disable Wakeup Signal
    WDT_Open(TimeoutInterval, WDT_RESET_DELAY_1025CLK, FALSE, FALSE);

    // Re-lock Regs
    SYS_LockReg();

    size_t tout_cnt = 5;
    while(tout_cnt--)
    {
        // Pull down LA_INPUT_PIN to indicate WDT counting start
        LA_INPUT_PIN = 0;

        // Wait for timeout event occur
        while(!WDT_GetTimeoutFlag());

        // Pull up LA_INPUT_PIN to indicate WDT timeout occurred
        LA_INPUT_PIN = 1;

        SYS_TEST("TIMEOUT\n");

        // Clear Timeout Event Flag
        WDT_ClearTimeoutFlag();

        // Reset Counter to restart counting
        SYS_UnlockReg();
        WDT_ResetCounter();
        SYS_LockReg();
    }

    SYS_UnlockReg();
    // Disable WDT, need to ensure Regs are unlocked
    WDT_Close();
    // Re-lock Regs
    SYS_LockReg();
}

T_WDT_TEST_RESULT WDT_TimeoutIntervalSelectionTestCase1(void)
{
    char c;
    while(1)
    {
        WDT_PrintInfoCase1();
        switch (c = getchar())
        {
        /* A. 4th power of 2 (16) times of WDT_CLK Period */
        case 'A':
        case 'a':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW4);
            break;

        /* B. 6th power of 2 (64) times of WDT_CLK Period */
        case 'B':
        case 'b':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW6);
            break;

        /* C. 8th power of 2 (256) times of WDT_CLK Period */
        case 'C':
        case 'c':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW8);
            break;

        /* D. 10th power of 2 (1024) times of WDT_CLK Period */
        case 'D':
        case 'd':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW10);
            break;

        /* E. 12th power of 2 (4096) times of WDT_CLK Period */
        case 'E':
        case 'e':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW12);
            break;

        /* F. 14th power of 2 (16384) times of WDT_CLK Period */
        case 'F':
        case 'f':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW14);
            break;

        /* G. 15th power of 2 (32768) times of WDT_CLK Period */
        case 'G':
        case 'g':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW15);
            break;

        /* H. 16th power of 2 (65536) times of WDT_CLK Period */
        case 'H':
        case 'h':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW16);
            break;

        /* I. 17th power of 2 (131072) times of WDT_CLK Period */
        case 'I':
        case 'i':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW17);
            break;

        /* J. 18th power of 2 (262144) times of WDT_CLK Period */
        case 'J':
        case 'j':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW18);
            break;

        /* K. 19th power of 2 (524288) times of WDT_CLK Period */
        case 'K':
        case 'k':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW19);
            break;

        /* L. 20th power of 2 (1048576) times of WDT_CLK Period */
        case 'L':
        case 'l':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW20);
            break;

        /* M. 21st power of 2 (2097152) times of WDT_CLK Period */
        case 'M':
        case 'm':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW21);
            break;

        /* N. 22nd power of 2 (4194304) times of WDT_CLK Period */
        case 'N':
        case 'n':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW22);
            break;

        /* O. 23rd power of 2 (8388608) times of WDT_CLK Period */
        case 'O':
        case 'o':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW23);
            break;

        /* P. 24th power of 2 (16777216) times of WDT_CLK Period */
        case 'P':
        case 'p':
            WDT_TimeoutSelectionProc(WDT_TIMEOUT_2POW24);
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
