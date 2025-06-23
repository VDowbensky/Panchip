/**************************************************************************//**
 * @file     clktrim_common.h
 * @version  V1.0
 * $Date:    20/10/14 16:42 $
 * @brief    Common header file for clktrim test.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __WDT_COMMON_H__
#define __WDT_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum _T_WDT_TST_RESULT
{
    TRIM_TST_OK,
} T_TRIM_TEST_RESULT;


// Config UART for Debug/Log
#define DEBUG_PORT      	UART0   //Note this define should be same with that in retarget.c

//#define WAIT_CNT			(0X4)
//#define CALC_CNT			(0X64)

#define FUNC_SELECT_COARSE_TUNING		(1<<0)
#define FUNC_SELECT_FINE_TUNING			(1<<1)
#define FUNC_SELECT_PRECISION_TUNING	(1<<2)

#define EARLY_TERMINATION_OPEN			(1)
#define EARLY_TERMINATION_CLOSE			(0)

extern uint8_t stop_int_flag,hw_trig_flag;
extern volatile uint8_t lp_flag;

extern void TRIM_TestModuleInit(void);

// Test Case Functions
extern T_TRIM_TEST_RESULT TRIM_RegisterDefaultValueCheckCase0(void);
extern T_TRIM_TEST_RESULT TRIM_MeasureCase1(void);
extern T_TRIM_TEST_RESULT TRIM_TuningCase2(void);
extern T_TRIM_TEST_RESULT TRIM_InterruptTestCase3(void);
extern void TRIM_TestFunctionEnter(uint16_t TcIdx);
extern T_TRIM_TEST_RESULT TRIM_HwTuningCase4(void);
#ifdef __cplusplus
}
#endif

#endif //__WDT_COMMON_H__
