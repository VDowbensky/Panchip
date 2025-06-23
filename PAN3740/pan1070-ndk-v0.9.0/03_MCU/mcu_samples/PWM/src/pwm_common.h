/**************************************************************************//**
 * @file     pwm_common.h
 * @version  V1.0
 * $Date:    19/11/06 17:10 $
 * @brief    Common header file for PWM test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __PWM_COMMON_H__
#define __PWM_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CHAR_NONE       (-1)

typedef enum _T_PWM_TST_RESULT
{
    PWM_TST_OK,
} T_PWM_TEST_RESULT;


// Config UART for Debug/Log
#define DEBUG_PORT      UART1   //Note this define should be same with that in retarget.c

// Config Target PWM Channel for Test
#define TGT_PWM_CH      PWM_CH0
#define AUX_PWM_CH      PWM_CH1

#define TEST_PWM0       0
#define TEST_PWM1       1
#define TEST_PWM2       2


#define TGT_PWM         PWM
#define TGT_PWM_IRQ     PWM_IRQn
#define FLAG_PRD_CMP    P10     //GPIO PIN flag to indicate PERIOD/CMPDAT value changed
#define FLAG_ZIF        P12     //GPIO PIN flag to indicate ZIF signal
#define FLAG_CMPUIF     P13     //GPIO PIN flag to indicate CMPUIF signal// Test Case Functions
#define FLAG_PIF        P14     //GPIO PIN flag to indicate PIF signal
#define FLAG_CMPDIF     P15     //GPIO PIN flag to indicate CMPDIF signal


extern T_PWM_TEST_RESULT PWM_RegisterDefaultValueCheckCase0(void);
extern T_PWM_TEST_RESULT PWM_CountingModesTestCase1(void);
extern T_PWM_TEST_RESULT PWM_CountingTypesAndInteruptsTestCase2(void);
extern T_PWM_TEST_RESULT PWM_OperationModesTestCase3(void);
extern T_PWM_TEST_RESULT PWM_PolarityControlTestCase4(void);
extern T_PWM_TEST_RESULT PWM_AdcTriggerTestCase5(void);

extern void PWM_TestFunctionEnter(uint16_t TcIdx);
extern void PWM_TestModuleInit(void);


#ifdef __cplusplus
}
#endif

#endif //__PWM_COMMON_H__
