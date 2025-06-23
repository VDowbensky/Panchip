/**************************************************************************//**
 * @file     timer_counting_modes_test.c
 * @version  V1.0
 * $Date:    19/10/30 18:00 $
 * @brief    Timer test case 1, test Timer Counting Modes, including One-Shot
 *           Mode, Periodic Mode, Toggle-Output Mode, Continuous-Counting Mode.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "timer_common.h"


static void TIMER_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    One-Shot Mode.                                  | \n");
    SYS_TEST("|    Input 'B'    Periodic Mode.                                  | \n");
    SYS_TEST("|    Input 'C'    Toggle-Output Mode.                             | \n");
    SYS_TEST("|    Input 'D'    Continuous-Counting Mode.                       | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void TIMER_OneShotModeTest(void)
{
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency
    uint32_t tmrCmpValue = 0;       //Comparison Value

    // Select Timer clock source
    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = FREQ_16MHZ;
    tmrRealCntFreq = TIMER_Open(TGT_TIMER, TIMER_ONESHOT_MODE, tmrExpCntFreq);

    // Set compare value (to 16M)
    // expect_timeout = tmrCmpValue / tmrExpCntFreq = 16M / 16MHz = 1s
    // real_timeout = tmrCmpValue / tmrRealCntFreq = 16M / tmrRealCntFreq
    tmrCmpValue = 0xF42400; //16000000
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Enable interrupt
    TIMER_EnableInt(TGT_TIMER);
    NVIC_EnableIRQ(TGT_TMR_IRQn);

    // Start Timer counting
    SYS_TEST("\nTimer%d, Compare Value:%d\n",TGT_TIMER_ID, tmrCmpValue);
    SYS_TEST("Expected Cnt Freq:%d, Real Cnt Freq:%d\n", tmrExpCntFreq, tmrRealCntFreq);
    SYS_TEST("Expected Timeout:%llums, Real Timeout:%llums\n",
             1000ull*tmrCmpValue/tmrExpCntFreq, 1000ull*tmrCmpValue/tmrRealCntFreq);
    SYS_TEST("Start Timer...\n");
    TIMER_Start(TGT_TIMER);

    while(!tmr_int_trigger_cnt);
    tmr_int_trigger_cnt = 0;

    // Disable interrupt
    TIMER_DisableInt(TGT_TIMER);
    NVIC_DisableIRQ(TGT_TMR_IRQn);
/*
    CNT (Count Value) is automatically cleared by hardware in One-Shot Mode.
    CNTEN (Count Enable) is automatically reset to 0 by hardware in One-Shot Mode.
*/
}

static void TIMER_PeriodicModeTest(void)
{
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency
    uint32_t tmrCmpValue = 0;       //Comparison Value

    // Select Timer clock source
    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = FREQ_16MHZ;
    tmrRealCntFreq = TIMER_Open(TGT_TIMER, TIMER_PERIODIC_MODE, tmrExpCntFreq);

    // Set compare value (to 16M)
    // expect_timeout = tmrCmpValue / tmrExpCntFreq = 16M / 16MHz = 1s
    // real_timeout = tmrCmpValue / tmrRealCntFreq = 16M / tmrRealCntFreq
    tmrCmpValue = 0xF42400; //16000000
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Enable interrupt
    TIMER_EnableInt(TGT_TIMER);
    NVIC_EnableIRQ(TGT_TMR_IRQn);

    // Start Timer counting
    SYS_TEST("\nTimer%d, Compare Value:%d\n",TGT_TIMER_ID, tmrCmpValue);
    SYS_TEST("Expected Cnt Freq:%d, Real Cnt Freq:%d\n", tmrExpCntFreq, tmrRealCntFreq);
    SYS_TEST("Expected Timeout:%llums, Real Timeout:%llums\n",
             1000ull*tmrCmpValue/tmrExpCntFreq, 1000ull*tmrCmpValue/tmrRealCntFreq);
    SYS_TEST("Start Timer...\n");
    TIMER_Start(TGT_TIMER);

    while(tmr_int_trigger_cnt != 5);
    tmr_int_trigger_cnt = 0;

    // Disable interrupt
    TIMER_DisableInt(TGT_TIMER);
    NVIC_DisableIRQ(TGT_TMR_IRQn);

    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(TGT_TIMER);
}

static void TIMER_ToggleOutputModeTest(void)
{
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency
    uint32_t tmrCmpValue = 0;       //Comparison Value

    // Select Timer clock source
    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = FREQ_16MHZ;
    tmrRealCntFreq = TIMER_Open(TGT_TIMER, TIMER_TOGGLE_MODE, tmrExpCntFreq);

    // Set compare value (to 16K),
    // expect_timeout = tmrCmpValue / tmrExpCntFreq = 16K / 16MHz = 1ms
    // real_timeout = tmrCmpValue / tmrRealCntFreq = 16K / tmrRealCntFreq
    tmrCmpValue = 16000;
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);
    /*
       The Toggle Output Level changed each time the timeout reached, thus
       the output waveform period = 2*real_timeout, freq = 1 / (2*real_timeout)
    */
    // Enable interrupt
    TIMER_EnableInt(TGT_TIMER);
    NVIC_EnableIRQ(TGT_TMR_IRQn);

    // Start Timer counting
    SYS_TEST("\nTimer%d, Compare Value:%d\n",TGT_TIMER_ID, tmrCmpValue);
    SYS_TEST("Expected Cnt Freq:%d, Real Cnt Freq:%d\n", tmrExpCntFreq, tmrRealCntFreq);
    SYS_TEST("Expected Timeout:%lluus, Real Timeout:%lluus\n",
             1000000ull*tmrCmpValue/tmrExpCntFreq, 1000000ull*tmrCmpValue/tmrRealCntFreq);
    SYS_TEST("Output Waveform Frequency, Expected:%uHz, Real:%uHz\n",
             tmrExpCntFreq/tmrCmpValue/2, tmrRealCntFreq/tmrCmpValue/2);
    SYS_TEST("Start Timer...\n");
    TIMER_Start(TGT_TIMER);

    while(tmr_int_trigger_cnt < 20);
    tmr_int_trigger_cnt = 0;

    // Disable interrupt
    TIMER_DisableInt(TGT_TIMER);
    NVIC_DisableIRQ(TGT_TMR_IRQn);

    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(TGT_TIMER);
}

static void TIMER_ContiniousCountingModeTest(void)
{
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency
    uint32_t tmrCmpValue = 0;       //Comparison Value
    uint32_t tmrCntValue = 0;       //Counter Data Value

    // Select Timer clock source
    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = 300000; //300k
    tmrRealCntFreq = TIMER_Open(TGT_TIMER, TIMER_CONTINUOUS_MODE, tmrExpCntFreq);

    // Enable interrupt
    TIMER_EnableInt(TGT_TIMER);
    NVIC_EnableIRQ(TGT_TMR_IRQn);

    SYS_TEST("\nTimer%d, Cnt Freq:%dHz\n",TGT_TIMER_ID, tmrRealCntFreq);

    // First set compare value to 300K
    tmrCmpValue = 300000;
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Start Timer counting
    SYS_TEST("Start Timer...\n");
    TIMER_Start(TGT_TIMER);

    // Wait for the first INT
    while(tmr_int_trigger_cnt < 1);
    tmrCntValue = TIMER_GetCounter(TGT_TIMER);
    SYS_TEST("Timer Compare Value:%d, Counter Value:%d\n", tmrCmpValue, tmrCntValue);

    // Change compare value to 600K
    tmrCmpValue = 600000;
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Wait for the second INT
    while(tmr_int_trigger_cnt < 2);
    tmrCntValue = TIMER_GetCounter(TGT_TIMER);
    SYS_TEST("Timer Compare Value:%d, Counter Value:%d\n", tmrCmpValue, tmrCntValue);

    // Change compare value to 1200K
    tmrCmpValue = 1200000;
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpValue);

    // Wait for the second INT
    while(tmr_int_trigger_cnt < 3);
    tmrCntValue = TIMER_GetCounter(TGT_TIMER);
    SYS_TEST("Timer Compare Value:%d, Counter Value:%d\n", tmrCmpValue, tmrCntValue);

    tmr_int_trigger_cnt = 0;

    // Disable interrupt
    TIMER_DisableInt(TGT_TIMER);
    NVIC_DisableIRQ(TGT_TMR_IRQn);

    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(TGT_TIMER);
    SYS_TEST("Timer%d stopped.\n", TGT_TIMER_ID);
}

T_TIMER_TEST_RESULT TIMER_TimerCountingModesTestCase1(void)
{
    char c;

		TIMER0->CTL |= (0X7 << 8);
		TIMER1->CTL |= (0X7 << 8);
		TIMER2->CTL |= (0X7 << 8);
    while(1)
    {
        TIMER_PrintInfoCase1();
        switch(c = getchar())
        {
        /* A. One-Shot Mode */
        case 'A':
        case 'a':
            TIMER_OneShotModeTest();
            break;
        /* B. Periodic Mode */
        case 'B':
        case 'b':
            TIMER_PeriodicModeTest();
            break;
        /* C. Toggle-Output Mode */
        case 'C':
        case 'c':
            TIMER_ToggleOutputModeTest();
            break;
        /* D. Continuous-Counting Mode */
        case 'D':
        case 'd':
            TIMER_ContiniousCountingModeTest();
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
