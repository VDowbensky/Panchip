/**************************************************************************//**
 * @file     wdt_common.h
 * @version  V1.0
 * $Date:    19/11/12 17:17 $
 * @brief    Common header file for WDT test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
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
    WDT_TST_OK,
} T_WDT_TEST_RESULT;


// Config UART for Debug/Log
#define DEBUG_PORT      	UART0   //Note this define should be same with that in retarget.c

#define LA_INPUT_PIN    	P20
#define WDT_CLK_SRC_SEL_RCL  		CLK_APB1_WDTSEL_RCL32K
#define WDT_CLK_SRC_SEL_APBDIV  	CLK_APB1_WDTSEL_MILLI_PULSE

// Test Case Functions
extern T_WDT_TEST_RESULT WDT_RegisterDefaultValueCheckCase0(void);
extern T_WDT_TEST_RESULT WDT_TimeoutIntervalSelectionTestCase1(void);
extern T_WDT_TEST_RESULT WDT_ClockSourceSelectionTestCase2(void);
extern T_WDT_TEST_RESULT WDT_InterruptModeTestCase3(void);
extern T_WDT_TEST_RESULT WDT_ResetModeTestCase4(void);
extern T_WDT_TEST_RESULT WDT_WakeupSignalTestCase5(void);

extern void WDT_TestFunctionEnter(uint16_t TcIdx);
extern void WDT_TestModuleInit(void);
extern void WDT_ResetRegionSelect(void);

#ifdef __cplusplus
}
#endif

#endif //__WDT_COMMON_H__
