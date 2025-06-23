/**************************************************************************//**
 * @file     timer_event_counting_mode_test.c
 * @version  V1.0
 * $Date:    19/10/31 16:00 $
 * @brief    Timer test case 2, test Event Counting Mode.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "timer_common.h"


static void TIMER_PrintInfoCase2(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Event Counting Mode.                            | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void TIMER_EventCountModeProc(void)
{
    uint32_t tmrCmpValue = 0;
    uint32_t pwmFreq = 0;
    uint32_t pwmDutyCycle = 0;
    uint32_t pwmDurationMs = 0;
    uint32_t pwmCycleCount = 0;

    // No need to set clock reg for Event Counting Mode, as clock source is external input!
//    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer Mode
    TIMER_SetCountingMode(TGT_TIMER, TIMER_PERIODIC_MODE);

    // Set prescale
    TIMER_SetPrescaleValue(TGT_TIMER, 0);

    // Set compare value
    tmrCmpValue = 6100;
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Counter increase on falling edge
    TIMER_EnableEventCounter(TGT_TIMER, TIMER_COUNTER_FALLING_EDGE);

    SYS_TEST("\nTimer start counting, Compare Value = %d...\n\n", tmrCmpValue);
    // Start Timer counting
    TIMER_Start(TGT_TIMER);

    // Enable timer interrupt
    TIMER_EnableInt(TGT_TIMER);
    NVIC_EnableIRQ(TGT_TMR_IRQn);

    // Generate 1st waveform, Freq 500Hz, Duty cycle 50%, Duration 100ms
    pwmFreq = 800, pwmDutyCycle = 50, pwmDurationMs = 100;
    pwmCycleCount += pwmDurationMs * pwmFreq / 1000;
    SYS_TEST("PWM start generating the 1st waveform (%dHz %d%% %dms)...\n", pwmFreq, pwmDutyCycle, pwmDurationMs);
    PWM_GenerateWaveform(pwmFreq, pwmDutyCycle, pwmDurationMs);
    SYS_TEST("The 1st waveform counting done, expect count:%d, actual count:%d\n\n",
        pwmCycleCount, TIMER_GetCounter(TGT_TIMER));

    // Generate 2nd waveform, Freq 2000Hz, Duty cycle 80%, Duration 3000ms
    pwmFreq = 2000, pwmDutyCycle = 80, pwmDurationMs = 3000;
    pwmCycleCount += pwmDurationMs * pwmFreq / 1000;
    SYS_TEST("PWM start generating the 2nd waveform (%dHz %d%% %dms)...\n", pwmFreq, pwmDutyCycle, pwmDurationMs);
    PWM_GenerateWaveform(pwmFreq, pwmDutyCycle, pwmDurationMs);
    SYS_TEST("The 2nd waveform counting done, expect count:%d, actual count:%d\n\n",
        pwmCycleCount, TIMER_GetCounter(TGT_TIMER));

    // Generate 3rd waveform, Freq 20Hz, Duty cycle 15%, Duration 2500ms
    pwmFreq = 20, pwmDutyCycle = 15, pwmDurationMs = 2500;
    pwmCycleCount += pwmDurationMs * pwmFreq / 1000;
    SYS_TEST("PWM start generating the 3rd waveform (%dHz %d%% %dms)...\n", pwmFreq, pwmDutyCycle, pwmDurationMs);
    PWM_GenerateWaveform(pwmFreq, pwmDutyCycle, pwmDurationMs);
    SYS_TEST("The 3rd waveform counting done, expect count:%d, actual count:%d\n\n",
        pwmCycleCount, TIMER_GetCounter(TGT_TIMER));

    tmr_int_trigger_cnt = 0;

    // Disable interrupt
    TIMER_DisableInt(TGT_TIMER);
    NVIC_DisableIRQ(TGT_TMR_IRQn);

    // Disable event counter mode
    TIMER_DisableEventCounter(TGT_TIMER);

    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(TGT_TIMER);
}

T_TIMER_TEST_RESULT TIMER_EventCountingModeTestCase2(void)
{
    char c;

    while(1)
    {
        TIMER_PrintInfoCase2();
        switch(c = getchar())
        {
        /* A. Event Counting Mode */
        case 'A':
        case 'a':
            TIMER_EventCountModeProc();
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return TIMER_TST_OK;
}
