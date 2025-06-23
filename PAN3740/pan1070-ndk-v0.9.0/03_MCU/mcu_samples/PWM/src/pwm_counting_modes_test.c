/**************************************************************************//**
 * @file     pwm_counting_modes_test.c
 * @version  V1.0
 * $Date:    19/11/06 19:20 $
 * @brief    PWM test case 1, test PWM Counting Modes, including One-Shot Mode
 *           and Auto-Reload Mode, and also test the Double Buffering Feature.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "pwm_common.h"


static void PWM_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    One-Shot Mode.                                  | \n");
    SYS_TEST("|    Input 'B'    Auto-Reload Mode with PERIOD and CMPDAT value   | \n");
    SYS_TEST("|                 changing (Double Buffering Feature involved).   | \n");
    SYS_TEST("|    Input 'C'    Auto-Reload Mode with PERIOD and CMPDAT value   | \n");
    SYS_TEST("|                 changing (Center Loading Operation Enabled).    | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}


static void PWM_OneShotModeProc(void)
{
    SYS_TEST("\nPWM One-Shot Mode is not supported yet!\n");
}

static void PWM_AutoReloadModeProc(void)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    // Config Target Output Channel
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);

    // Set counting mode
    // Note: This API is not actually necessary here as the previous PWM_ConfigOutputChannel()
    //       sets the counting mode to Auto-Reload Mode internally.
    // PWM_SetCntMode(TGT_PWM, TGT_PWM_CH, PWM_CNTMODE_AUTO_RELOAD);

    // Enable output of TGT_PWM channel
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH));

    // Start
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));
    FLAG_PRD_CMP = 0;
    SYS_TEST("\nPWM start, auto-reload mode, PERIOD: %d, CMPDAT: %d.\n",
             (&TGT_PWM->PERIOD0)[TGT_PWM_CH], (&TGT_PWM->CMPDAT0)[TGT_PWM_CH]);

    // Change PERIOD and CMPDAT
    FLAG_PRD_CMP = 1;
    PWM_SetCNR(TGT_PWM, TGT_PWM_CH, (&TGT_PWM->PERIOD0)[TGT_PWM_CH] * 2);
    PWM_SetCMR(TGT_PWM, TGT_PWM_CH, (&TGT_PWM->CMPDAT0)[TGT_PWM_CH] * 4);
    FLAG_PRD_CMP = 0;
    SYS_TEST("PERIOD and CMPDAT changed, new PERIOD: %d, new CMPDAT: %d.\n",
             (&TGT_PWM->PERIOD0)[TGT_PWM_CH], (&TGT_PWM->CMPDAT0)[TGT_PWM_CH]);

    // Stop
    PWM_Stop(TGT_PWM, BIT(TGT_PWM_CH));
    FLAG_PRD_CMP = 1;
    SYS_TEST("TGT_PWM stopped.\n");
}

static void PWM_AutoReloadModeWithCenterLoadingProc(void)
{
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 1000, 30, OPERATION_CENTER_ALIGNED);

    // Enable Center Loading Operation (support only in Center-Aligned Mode)
    TGT_PWM->CTL |= 0x20;   //CTL.HCUPDT = 1

    // Enable output of TGT_PWM channel
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH));

    // Start
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));
    FLAG_PRD_CMP = 0;
    SYS_TEST("\nPWM start, auto-reload mode, PERIOD: %d, CMPDAT: %d.\n",
             (&TGT_PWM->PERIOD0)[TGT_PWM_CH], (&TGT_PWM->CMPDAT0)[TGT_PWM_CH]);
    SYS_delay_10nop(10000);

    // Change PERIOD and CMPDAT
    FLAG_PRD_CMP = 1;
    PWM_SetCMR(TGT_PWM, TGT_PWM_CH, (&TGT_PWM->CMPDAT0)[TGT_PWM_CH] * 2 / 3);
    FLAG_PRD_CMP = 0;
    SYS_TEST("PERIOD and CMPDAT changed, new PERIOD: %d, new CMPDAT: %d.\n",
             (&TGT_PWM->PERIOD0)[TGT_PWM_CH], (&TGT_PWM->CMPDAT0)[TGT_PWM_CH]);

    SYS_delay_10nop(10000);
    // Stop
    PWM_Stop(TGT_PWM, BIT(TGT_PWM_CH));
    FLAG_PRD_CMP = 1;
    SYS_TEST("TGT_PWM stopped.\n");
}

T_PWM_TEST_RESULT PWM_CountingModesTestCase1(void)
{
    char c;

    while(1)
    {
        PWM_PrintInfoCase1();
        switch(c = getchar())
        {
        /* A. One-Shot Mode */
        case 'A':
        case 'a':
            PWM_OneShotModeProc();
            break;
        /* B. Auto-Reload Mode with Double Buffering */
        case 'B':
        case 'b':
            PWM_AutoReloadModeProc();
            break;
        /* B. Auto-Reload Mode with Center Loading */
        case 'C':
        case 'c':
            PWM_AutoReloadModeWithCenterLoadingProc();
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return PWM_TST_OK;
}
