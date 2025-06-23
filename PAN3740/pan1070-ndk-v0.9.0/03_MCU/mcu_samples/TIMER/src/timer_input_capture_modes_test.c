/**************************************************************************//**
 * @file     timer_input_capture_modes_test.c
 * @version  V1.0
 * $Date:    19/11/01 15:20 $
 * @brief    Timer test case 3, test Input Capture Modes, including Free-Counting
 *           Capture Mode, External Reset Counter Mode and Trigger-Counting
 *           Capture Mode.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "timer_common.h"


static void TIMER_PrintInfoCase3(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Free-Counting Capture Mode.                     | \n");
    SYS_TEST("|    Input 'B'    External Reset Counter Mode.                    | \n");
    SYS_TEST("|    Input 'C'    Trigger-Counting Capture Mode.                  | \n");
    // SYS_TEST("|    Input 'D'    Internal RCL Capture Function.                  | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void TIMER_InputCaptureModesProc(TIMER_CntModeDef CountMode, uint32_t u32CmpVal, TIMER_CapModeDef capMode, TIMER_CapEdgeDef capEdge,
                                        uint32_t u32PwmFreq, uint32_t u32PwmDutyCycle, uint32_t u32PwmDurationMs)
{
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency

    // Select Timer clock source
    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = FREQ_1MHZ;
    tmrRealCntFreq = TIMER_Open(TGT_TIMER, CountMode, tmrExpCntFreq);

    // Set compare value
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, u32CmpVal);

    // Set capture source to ext PIN
    TIMER_SetCaptureSource(TGT_TIMER, TIMER_CAPTURE_SOURCE_EXT_PIN);

    // Configure Timer capture mode and capture edge
    TIMER_EnableCapture(TGT_TIMER, capMode, capEdge);

    // Enable Timer capture interrupt
    TIMER_EnableCaptureInt(TGT_TIMER);
    NVIC_EnableIRQ(TGT_TMR_IRQn);

    // Start Timer
    SYS_TEST("\nTimer%d start, Cnt Freq:%dHz, Compare Value:%d\n", TGT_TIMER_ID, tmrRealCntFreq, u32CmpVal);
    TIMER_Start(TGT_TIMER);

    // Start external capture signal (simulated by PWM module)
    SYS_TEST("Start external waveform signal (freq:%dHz, duty cycle:%d%%, duration:%dms)...\n",
        u32PwmFreq, u32PwmDutyCycle, u32PwmDurationMs);
    PWM_GenerateWaveform(u32PwmFreq, u32PwmDutyCycle, u32PwmDurationMs);

    tmr_cap_int_cnt = 0;    //Reset global INT count after use

    // Disable Timer capture interrupt
    TIMER_DisableCaptureInt(TGT_TIMER);
    NVIC_DisableIRQ(TGT_TMR_IRQn);

    // Disable Timer capture
    TIMER_DisableCapture(TGT_TIMER);

    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(TGT_TIMER);
}

static void TIMER_RclCalib(void)
{
    uint32_t tmrExpCntFreq = 0;     //Timer Expected Count Frequency
    uint32_t tmrRealCntFreq = 0;    //Timer Real Count Frequency
    uint32_t tmrCmpVal = 0;
    size_t capCnt = 0;
    uint32_t capVal[20] = {0,};

	TIMER_ClearCaptureFlag(TGT_TIMER);
    // Select Timer clock source
    CLK_SetTmrClkSrc(TGT_TIMER, TGT_TIMER_CLK_APB);

    // Set Timer work mode and expected clock frequency
    tmrExpCntFreq = FREQ_16MHZ;
    tmrRealCntFreq = TIMER_Open(TGT_TIMER, TIMER_ONESHOT_MODE, tmrExpCntFreq);

    // Set compare value to MAX (0xFFFFFF)
    tmrCmpVal = 0xFFFFFF;
    TIMER_SetCmpValue(TGT_TIMER, TMR0_COMPARATOR_SEL_CMP, tmrCmpVal);

    // Set capture source to internal 32KHz RCL
    TIMER_SetCaptureSource(TGT_TIMER, TIMER_CAPTURE_SOURCE_32K_OUTPUT);

    // Start Timer
    SYS_TEST("\nTimer%d start, Cnt Freq:%dHz, Compare Value:%d\n", TGT_TIMER_ID, tmrRealCntFreq, tmrCmpVal);
    TIMER_Start(TGT_TIMER);

    // Configure Timer as reset counter mode, capture Timer value on rising edge
    TIMER_EnableCapture(TGT_TIMER, TIMER_CAPTURE_COUNTER_RESET_MODE, TIMER_CAPTURE_RISING_EDGE);

    while(capCnt < 10)
    {
        // Polling, wait for the RCL rising edge
        while(!TIMER_GetCaptureFlag(TGT_TIMER));
        TIMER_ClearCaptureFlag(TGT_TIMER);
        capVal[capCnt] = TIMER_GetCaptureData(TGT_TIMER);
        capCnt++;
    }

    // Disable Timer capture
    TIMER_DisableCapture(TGT_TIMER);

    // Reset Timer counter/prescale and stop counting
    TIMER_Reset(TGT_TIMER);

    // Print log after disabling capture function to avoid timing risk
    for (size_t i = 0; i < capCnt; i++)
    {
        SYS_TEST("CapNum%02d: %04d --> Calculated RCL Freq: %d Hz\n", i, capVal[i], tmrRealCntFreq/capVal[i]);
    }
}

T_TIMER_TEST_RESULT TIMER_InputCaptureModesTestCase3(void)
{
    char c;

    while(1)
    {
        TIMER_PrintInfoCase3();
        switch(c = getchar())
        {
        /* A. Free-Counting Capture Mode */
        case 'A':
        case 'a':
            TIMER_InputCaptureModesProc(TIMER_ONESHOT_MODE, 400000, TIMER_CAPTURE_FREE_COUNTING_MODE,
                                        TIMER_CAPTURE_BOTH_EDGE, 20, 80, 500);
            break;
        /* B. External Reset Counter Mode */
        case 'B':
        case 'b':
            TIMER_InputCaptureModesProc(TIMER_CONTINUOUS_MODE, 0xFF, TIMER_CAPTURE_COUNTER_RESET_MODE,
                                        TIMER_CAPTURE_BOTH_EDGE, 20, 25, 500);
            break;
        /* C. Trigger-Counting Capture Mode */
        case 'C':
        case 'c':
            TIMER_InputCaptureModesProc(TIMER_ONESHOT_MODE, 0xFFFFFF, TIMER_CAPTURE_TRIGGER_COUNTING_MODE,
                                        TIMER_CAPTURE_RISING_THEN_FALLING_EDGE, 20, 60, 500);
            break;
        // /* D. Internal RCL Capture Function */
        // case 'D':
        // case 'd':
        //     TIMER_RclCalib();
        //     break;
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
