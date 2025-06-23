/**************************************************************************//**
 * @file     pwm_adc_trigger_test.c
 * @version  V1.0
 * $Date:    19/11/08 15:07 $
 * @brief    PWM test case 5. Test ADC-Trigger Function of PWM.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "pwm_common.h"


static void PWM_PrintInfoCase5(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    PWM Trigger ADC Conversion.                     | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void PWM_TriggerAdcConversionProc(void)
{
    SYS_TEST("\nPlease reference to ADC sample code.\n");
}


T_PWM_TEST_RESULT PWM_AdcTriggerTestCase5(void)
{
    char c;

    while(1)
    {
        PWM_PrintInfoCase5();
        switch(c = getchar())
        {
        /* A. PWM Trigger ADC Conversion */
        case 'A':
        case 'a':
            PWM_TriggerAdcConversionProc();
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
