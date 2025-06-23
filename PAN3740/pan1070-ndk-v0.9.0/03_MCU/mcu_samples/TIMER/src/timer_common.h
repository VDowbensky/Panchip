/**************************************************************************//**
 * @file     timer_common.h
 * @version  V1.0
 * $Date:    19/10/30 16:47 $
 * @brief    Common header file for Timer test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __TIMER_COMMON_H__
#define __TIMER_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CHAR_NONE       (-1)

typedef enum _T_TIMER_TST_RESULT
{
    TIMER_TST_OK,
} T_TIMER_TEST_RESULT;


// Config UART for Debug/Log
#define DEBUG_PORT      UART0   //Note this define should be same with that in retarget.c

// Config PWM to generate waveform to test timer input features
#define PWM_ID          2       //This value should be 2 or 3

// Config Target Timer for Test
#define TGT_TIMER_ID    TEST_TIMER0


#define TEST_TIMER0     0
#define TEST_TIMER1     1
#define TEST_TIMER2     2

#if TGT_TIMER_ID == TEST_TIMER0
#define TGT_TIMER           TIMER0
#define TGT_TIMER_CLK_APB   CLK_APB1_TMR0SEL_APB1CLK
#define TGT_TIMER_CLK_RCL   CLK_APB1_TMR0SEL_RCL32K
#define TGT_TMR_IRQn        TMR0_IRQn
#define AUX_TIMER           TIMER1
#elif TGT_TIMER_ID == TEST_TIMER1
#define TGT_TIMER           TIMER1
#define TGT_TIMER_CLK_APB   CLK_APB2_TMR1SEL_APB2CLK
#define TGT_TIMER_CLK_RCL   CLK_APB2_TMR1SEL_RCL32K
#define TGT_TMR_IRQn        TMR1_IRQn
#define AUX_TIMER           TIMER2
#elif TGT_TIMER_ID == TEST_TIMER2
#define TGT_TIMER           TIMER2
#define TGT_TIMER_CLK_APB   CLK_APB2_TMR2SEL_APB2CLK
#define TGT_TIMER_CLK_RCL   CLK_APB2_TMR2SEL_RCL32K
#define TGT_TMR_IRQn        TMR2_IRQn
#define AUX_TIMER           TIMER0
#else
#error "Error, invalid target TIMER ID, please check test config."
#endif


// Test Case Functions
extern T_TIMER_TEST_RESULT TIMER_RegisterDefaultValueCheckCase0(void);
extern T_TIMER_TEST_RESULT TIMER_TimerCountingModesTestCase1(void);
extern T_TIMER_TEST_RESULT TIMER_EventCountingModeTestCase2(void);
extern T_TIMER_TEST_RESULT TIMER_InputCaptureModesTestCase3(void);
extern T_TIMER_TEST_RESULT TIMER_WakeupTestCase4(void);
extern T_TIMER_TEST_RESULT TIMER_BitCheckTestCase5(void);
extern void TIMER_TestFunctionEnter(uint16_t TcIdx);
extern void TIMER_TestModuleInit(void);
extern void TIMER_DelayMs(TIMER_T *timer, uint32_t u32Ms);
extern void PWM_GenerateWaveform(uint32_t u32Freq, uint32_t u32DutyCycle, uint32_t u32DurationMs);

extern volatile uint32_t tmr_int_trigger_cnt;
extern volatile uint32_t tmr_cap_int_cnt;
extern volatile bool tmr_wakup_int;

#ifdef __cplusplus
}
#endif

#endif //__TIMER_COMMON_H__
