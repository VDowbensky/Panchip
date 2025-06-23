/**************************************************************************//**
 * @file     timer_common.c
 * @version  V1.0
 * $Date:    19/10/30 16:47 $
 * @brief    Common source file for Timer test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "timer_common.h"

volatile uint32_t tmr_int_trigger_cnt = 0;
volatile uint32_t tmr_cap_int_cnt = 0;
volatile bool tmr_wakup_int = false;

T_TIMER_TEST_RESULT (*const TIMER_TestCase[])(void) = 
{
    TIMER_RegisterDefaultValueCheckCase0,
    TIMER_TimerCountingModesTestCase1,
    TIMER_EventCountingModeTestCase2,
    TIMER_InputCaptureModesTestCase3,
    TIMER_WakeupTestCase4,
	TIMER_BitCheckTestCase5,
};

void TIMER_TestFunctionEnter(uint16_t TcIdx)
{
    T_TIMER_TEST_RESULT r = TIMER_TST_OK;

    if (TcIdx >= sizeof(TIMER_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (TIMER_TestCase[TcIdx])();
    if (r != TIMER_TST_OK)
    {
        SYS_TEST("TIMER Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("TIMER Test OK, Success case: %d\n", TcIdx);
    }
}


void TIMER_HandleProc(TIMER_T* TIMERx)
{
    if (TIMER_GetIntFlag(TIMERx))
    {
        // Clear Timer interrupt flag
        TIMER_ClearIntFlag(TIMERx);
        tmr_int_trigger_cnt++;
        SYS_TEST("CNT%d\n", tmr_int_trigger_cnt);
    }

    if (TIMER_GetCaptureIntFlag(TIMERx))
    {
        // Clear Timer capture interrupt flag
        TIMER_ClearCaptureIntFlag(TIMERx);
        tmr_cap_int_cnt++;
        SYS_TEST("CAP%02d CntVal:%6d, CapVal:%6d\n",
            tmr_cap_int_cnt, TIMER_GetCounter(TIMERx), TIMER_GetCaptureData(TIMERx));
    }

	uint32_t wake_flag = TIMER_GetWakeupFlag(TIMERx);
	uint32_t timer_flag = TIMER_GetTFFlag(TIMERx);
    if (wake_flag)
    {
        // Clear Timer interrupt flag
		TIMER_ClearTFFlag(TIMERx,timer_flag);
		TIMER_ClearIntFlag(TIMERx);
		TIMER_ClearWakeupFlag(TIMERx,wake_flag);
        tmr_wakup_int = true;
        SYS_TEST("WAKEUP\n");
    }
}

void TMR0_IRQHandler(void)
{
    TIMER_HandleProc(TIMER0);
}

void TMR1_IRQHandler(void)
{
    TIMER_HandleProc(TIMER1);
}

void TMR2_IRQHandler(void)
{
    TIMER_HandleProc(TIMER2);
}
void LP_IRQHandler(void)
{
	LP_ClearWakeFlag(ANA,ANAC_INT_DP_FLAG_Msk|ANAC_INT_STANDBY_M1_FLAG_Msk|ANAC_INT_STANDBY_M0_FLAG_Msk);
	SYS_TEST("sleep irq \r\n");
}
void TIMER_TestModuleInit(void)
{
#if TGT_TIMER_ID == TEST_TIMER0
    // Enable Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);    //Target TMR
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_TMR1, ENABLE);    //Auxiliary TMR
    // Config Pinmux
    SYS_SET_MFP(P1, 5, TIMER0_EXT);
    SYS_SET_MFP(P0, 3, TIMER0_CNT_OUT);
    GPIO_EnableDigitalPath(P1, BIT5);   //Enable the input digital path
#elif TGT_TIMER_ID == TEST_TIMER1
    // Enable Clock
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_TMR1, ENABLE);    //Target TMR
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_TMR2, ENABLE);    //Auxiliary TMR
    // Config Pinmux
    SYS_SET_MFP(P0, 6, TIMER1_EXT);
    SYS_SET_MFP(P2, 4, TIMER1_CNT_OUT);
    GPIO_EnableDigitalPath(P0, BIT6);   //Enable the input digital path
#elif TGT_TIMER_ID == TEST_TIMER2
    // Enable Clock
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_TMR2, ENABLE);    //Target TMR
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);    //Auxiliary TMR
    // Config Pinmux
    SYS_SET_MFP(P1, 7, TIMER2_EXT);
    SYS_SET_MFP(P1, 6, TIMER2_CNT_OUT);
    GPIO_EnableDigitalPath(P1, BIT7);   //Enable the input digital path
#else
#error "Error, Does not support this test TGT_TIMER_ID now."
#endif

#if PWM_ID == 2
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH23, ENABLE);
    SYS_SET_MFP(P2, 4, PWM_CH2);
#elif PWM_ID == 3
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH23, ENABLE);
    SYS_SET_MFP(P2, 5, PWM_CH3);
#else
#error "Error, Does not support this PWM_ID now."
#endif
//    GPIO_SetMode(P4, BIT1, GPIO_MODE_OUTPUT);   //For debug
}

void TIMER_DelayMs(TIMER_T *timer, uint32_t u32Ms)
{
    uint32_t u32Clk = CLK_GetPeripheralFreq(timer);
	
    uint32_t u32Prescale = 0xFF;
    uint32_t u32Cmp;

    // Set timer clock source to APB
    CLK_SetTmrClkSrc(timer, NULL);

    // Clear current timer configuration
    timer->CTL = 0;
    timer->EXTCTL = 0;

    // Get the largest divisor (u32Prescale+1) which can evenly divide u32Clk
    while(u32Clk % (u32Prescale+1) || (u32Clk / (u32Prescale+1) % 1000u))
    {
        u32Prescale--;
    }

    u32Cmp = u32Clk / (u32Prescale + 1) / 1000u * u32Ms;

    TIMER_SetCmpValue(timer, TMR0_COMPARATOR_SEL_CMP, u32Cmp);

    TIMER_SetPrescaleValue(timer, u32Prescale);

    TIMER_SetCountingMode(timer, TIMER_ONESHOT_MODE);

    TIMER_Start(timer);

    while(timer->CTL & TIMER_CTL_ACTSTS_Msk);
}

void PWM_GenerateWaveform(uint32_t u32Freq, uint32_t u32DutyCycle, uint32_t u32DurationMs)
{
//    GPIO_SetMode(P4, BIT1, GPIO_MODE_OUTPUT);   //For debug
    PWM_ResetPrescaler(PWM, PWM_ID);
    PWM_ConfigOutputChannel(PWM, PWM_ID, u32Freq, u32DutyCycle, OPERATION_CENTER_ALIGNED);
    PWM_EnableOutput(PWM, BIT(PWM_ID));

//    P41 = 1;  //For debug
    PWM_Start(PWM, BIT(PWM_ID));

    TIMER_DelayMs(AUX_TIMER, u32DurationMs);

    PWM_ForceStop(PWM, BIT(PWM_ID));    //Stop the PWM output immediately
//    P41 = 0;  //For debug
}


T_TIMER_TEST_RESULT TIMER_RegisterDefaultValueCheckCase0(void)
{
    SYS_TEST("Target Timer%d Register Default Values:\n", TGT_TIMER_ID);
    SYS_TEST("---------------------------\n");
    SYS_TEST("CTL           = 0x%08x\n", TGT_TIMER->CTL     );
    SYS_TEST("CMP           = 0x%08x\n", TGT_TIMER->CMP     );
    SYS_TEST("INTSTS        = 0x%08x\n", TGT_TIMER->INTSTS  );
    SYS_TEST("CNT           = 0x%08x\n", TGT_TIMER->CNT     );
    SYS_TEST("CAP           = 0x%08x\n", TGT_TIMER->CAP     );
    SYS_TEST("EXTCTL        = 0x%08x\n", TGT_TIMER->EXTCTL  );
    SYS_TEST("EINTSTS       = 0x%08x\n", TGT_TIMER->EINTSTS );

    return TIMER_TST_OK;
}
