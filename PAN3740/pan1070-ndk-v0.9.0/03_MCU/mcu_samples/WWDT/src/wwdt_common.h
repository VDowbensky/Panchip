/**************************************************************************//**
 * @file     wwdt_common.h
 * @version  V1.0
 * $Date:    19/11/14 11:17 $
 * @brief    Common header file for WWDT test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __WWDT_COMMON_H__
#define __WWDT_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum _T_WWDT_TST_RESULT
{
    WWDT_TST_OK,
} T_WWDT_TEST_RESULT;


// Config UART for Debug/Log
#define DEBUG_PORT      UART0   //Note this define should be same with that in retarget.c

#define LA_INPUT_PIN    	P20
#define WDT_CLK_SRC_SEL_RCL  		CLK_APB1_WWDTSEL_RCL32K
#define WDT_CLK_SRC_SEL_APBDIV  	CLK_APB1_WWDTSEL_MILLI_PULSE


// Test Case Functions
extern T_WWDT_TEST_RESULT WWDT_RegisterDefaultValueCheckCase0(void);
extern T_WWDT_TEST_RESULT WWDT_TimeoutWindowTestCase1(void);
extern T_WWDT_TEST_RESULT WWDT_ReloadAndResetTestCase2(void);
extern T_WWDT_TEST_RESULT WWDT_InterruptTestCase3(void);

extern void WWDT_TestFunctionEnter(uint16_t TcIdx);
extern void WWDT_TestModuleInit(void);
extern uint32_t WWDT_GetPrescaleFromConfig(uint32_t ConfigValue);

#ifdef __cplusplus
}
#endif

#endif //__WWDT_COMMON_H__
