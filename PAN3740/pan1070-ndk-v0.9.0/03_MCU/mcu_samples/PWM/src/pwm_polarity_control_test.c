/**************************************************************************//**
 * @file     pwm_polarity_control_test.c
 * @version  V1.0
 * $Date:    19/11/08 13:52 $
 * @brief    PWM test case 4. Test PWM Independent Polarity Control Function.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "pwm_common.h"


static void PWM_PrintInfoCase4(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Output Waveform with Polarity Control.          | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}


static void PWM_PolarityControlProc(void)
{
		CLK->IPRST1 |= CLK_IPRST1_PWM0RST_Msk;
		CLK->IPRST1 &= ~CLK_IPRST1_PWM0RST_Msk;
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);
    PWM_ConfigOutputChannel(TGT_PWM, AUX_PWM_CH, 5000, 30, OPERATION_EDGE_ALIGNED);

    // Change Polarity of Auxiliary TGT_PWM Channel
    PWM_EnableOutputInverter(TGT_PWM, BIT(AUX_PWM_CH));

    /*----------------- 1. Independent Mode --------------------*/
    PWM_EnableIndependentMode(TGT_PWM);
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));

    SYS_TEST("\nIndependent TGT_PWM wave start...\n");
    SYS_delay_10nop(2000);

    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH) | BIT(AUX_PWM_CH));;
    SYS_TEST("TGT_PWM wave stopped.\n");
    SYS_delay_10nop(100);

    /*----- 2. Complementary Mode with Dead-Time Insertion -----*/
    PWM_EnableComplementaryMode(TGT_PWM);
    PWM_EnableDeadZone(TGT_PWM, TGT_PWM_CH, 100);
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));

    SYS_TEST("\nComplementary TGT_PWM wave with Dead-Time start...\n");
    SYS_delay_10nop(100);

    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH));
    PWM_DisableDeadZone(TGT_PWM, TGT_PWM_CH);
    SYS_TEST("TGT_PWM wave stopped.\n");
    SYS_delay_10nop(100);

    /*---------------- 3. Synchronized Mode --------------------*/
    PWM_EnableSyncMode(TGT_PWM);
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));

    SYS_TEST("\nSynchronized TGT_PWM wave start...\n");
    SYS_delay_10nop(1000);

    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH));
    PWM_DisableOutputInverter(TGT_PWM, BIT(AUX_PWM_CH));
    SYS_TEST("TGT_PWM wave stopped.\n");
}

T_PWM_TEST_RESULT PWM_PolarityControlTestCase4(void)
{
    char c;

    while(1)
    {
        PWM_PrintInfoCase4();
        switch(c = getchar())
        {
        /* A. Output Waveform with Polarity Control */
        case 'A':
        case 'a':
            PWM_PolarityControlProc();
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
