/**************************************************************************//**
 * @file     pwm_operation_modes_test.c
 * @version  V1.0
 * $Date:    19/11/07 19:20 $
 * @brief    PWM test case 3. Test PWM Operation Modes including Independent
 *           Mode, Complementary Mode, Synchronized Mode and Gouping Mode.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "pwm_common.h"


static void PWM_PrintInfoCase3(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Independent Mode.                               | \n");
    SYS_TEST("|    Input 'B'    Complementary Mode.                             | \n");
    SYS_TEST("|    Input 'C'    Complementary Mode with Dead-Time Insertion.    | \n");
    SYS_TEST("|    Input 'D'    Synchronized Mode.                              | \n");
    SYS_TEST("|    Input 'E'    Grouping Mode.                                  | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}


static void PWM_IndependentModeProc(void)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    // Config the two Target and Auxiliary channels independently
//    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);
//    PWM_ConfigOutputChannel(TGT_PWM, AUX_PWM_CH, 4000, 50, OPERATION_EDGE_ALIGNED);
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 100, 30, OPERATION_EDGE_ALIGNED);
    PWM_ConfigOutputChannel(TGT_PWM, AUX_PWM_CH, 200, 50, OPERATION_EDGE_ALIGNED);
    // Enable Independent Mode
    PWM_EnableIndependentMode(TGT_PWM);

    // Enable output of TGT_PWM channels
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));

    // Start
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));

    SYS_TEST("\nPWM start...\n");
    SYS_delay_10nop(100000);

    // Stop
    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));;
    SYS_TEST("TGT_PWM stopped.\n");
}

static void PWM_ComplementaryModeProc(void)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    // Config the Target channel
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);

    // Enable Complementary Mode
    PWM_EnableComplementaryMode(TGT_PWM);

    // Enable output of TGT_PWM channels
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));

    // Start
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));

    SYS_TEST("\nPWM start...\n");
    SYS_delay_10nop(100000);

    // Stop
    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH));
    SYS_TEST("TGT_PWM stopped.\n");
}

static void PWM_ComplModeWithDeadTimeInsertProc(void)
{
    uint32_t dzDuration = 100;  //Dead-Zone Duration

    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    // Config the Target channel
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);

    // Enable Complementary Mode
    PWM_EnableComplementaryMode(TGT_PWM);

    SYS_TEST("\nPWM Channel%d Channel%d start...\n", TGT_PWM_CH, AUX_PWM_CH);
    SYS_TEST("APB CLK Freq: %dHz, CLKPSC: %d, CLKDIV: %d, PERIOD: %d, CMPDAT: %d\n",
        CLK_GetPCLK1Freq(), ((uint8_t*)(&TGT_PWM->CLKPSC))[TGT_PWM_CH/2],
        (TGT_PWM->CLKDIV & (uint32_t)(PWM_CLKDIV_CLKDIV0_Msk << (TGT_PWM_CH*4))) >> (TGT_PWM_CH*4),
        (&TGT_PWM->PERIOD0)[TGT_PWM_CH], (&TGT_PWM->CMPDAT0)[TGT_PWM_CH]);
    SYS_TEST("Dead Zone Duration: %d\n", dzDuration);

    // Enable output of TGT_PWM channels
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));

    // Start (Only the First Channel needed)
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));

    // Enable Dead-Time Counter and Set Dead Time
    PWM_EnableDeadZone(TGT_PWM, TGT_PWM_CH, dzDuration);

    SYS_delay_10nop(100000);

    // Stop (Only the First Channel needed)
    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH));

    //Disable Dead-Time Insertion
    PWM_DisableDeadZone(TGT_PWM, TGT_PWM_CH);
    SYS_TEST("TGT_PWM stopped.\n");

    // Disable Complementary Mode
    PWM_DisableComplementaryMode(TGT_PWM);
}

static void PWM_SynchronizedModeProc(void)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    // Config the Target channel
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);

    // Enable Synchrinozed Mode
    PWM_EnableSyncMode(TGT_PWM);

    // Enable output of TGT_PWM channels
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));

    // Start
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));

    SYS_TEST("\nPWM start...\n");
    SYS_delay_10nop(100000);

    // Stop
    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH));
    SYS_TEST("TGT_PWM stopped.\n");

    // Disable Synchrinozed Mode
    PWM_DisableSyncMode(TGT_PWM);
}

static void PWM_GroupingModeProc(void)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, PWM_CH0);

    // Config TGT_PWM channel 0 and channel 1
    PWM_ConfigOutputChannel(TGT_PWM, PWM_CH0, 5000, 30, OPERATION_CENTER_ALIGNED);
    PWM_ConfigOutputChannel(TGT_PWM, PWM_CH1, 4000, 50, OPERATION_CENTER_ALIGNED);

    // Enable Grouping Mode
    PWM_EnableGroupMode(TGT_PWM);

    // Enable output of ALL TGT_PWM channels
    PWM_EnableOutput(TGT_PWM, 0xFF);

    // Start (Only the First Channel of each Group needed)
    PWM_Start(TGT_PWM, BIT(PWM_CH0) | BIT(PWM_CH1));

    SYS_TEST("\nPWM start...\n");
    SYS_delay_10nop(100000);

    // Stop (Only the First Channel of each Group needed)
    PWM_ForceStop(TGT_PWM, BIT(PWM_CH0) | BIT(PWM_CH1));
    SYS_TEST("TGT_PWM stopped.\n");

    // Disable Grouping Mode
    PWM_DisableGroupMode(TGT_PWM);
}

T_PWM_TEST_RESULT PWM_OperationModesTestCase3(void)
{
    char c;

    while(1)
    {
        PWM_PrintInfoCase3();
        switch(c = getchar())
        {
        /* A. Independent Mode */
        case 'A':
        case 'a':
            PWM_IndependentModeProc();
            break;
        /* B. Complementary Mode */
        case 'B':
        case 'b':
            PWM_ComplementaryModeProc();
            break;
        /* C. Complementary Mode with Dead-Time Insertion */
        case 'C':
        case 'c':
            PWM_ComplModeWithDeadTimeInsertProc();
            break;
        /* D. Synchronized Mode */
        case 'D':
        case 'd':
            PWM_SynchronizedModeProc();
            break;
        /* E. Grouping Mode */
        case 'E':
        case 'e':
            PWM_GroupingModeProc();
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
