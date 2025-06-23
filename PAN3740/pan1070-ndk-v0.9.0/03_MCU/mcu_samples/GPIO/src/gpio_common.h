/**************************************************************************//**
 * @file     gpio_common.h
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    Header file for GPIO test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __GPIO_COMMON_H__
#define __GPIO_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _T_GPIO_TST_RESULT
{
    GPIO_TST_OK,
    GPIO_TST_TGT_PULL_UP_FAIL,
    GPIO_TST_TGT_PULL_DOWN_FAIL,
    GPIO_TST_TGT_INPUT_GET_0_FAIL,
    GPIO_TST_TGT_INPUT_GET_1_FAIL,

    GPIO_TST_TGT_PULL_UP_MASK_FAIL,
    GPIO_TST_TGT_PULL_DOWN_MASK_FAIL,
} T_GPIO_TEST_RESULT;

#define GPIO_INTEN_MASK_BASE                0x00010001UL
#define GPIO_INTTYPE_MASK_BASE              0x00000001UL

/* PIN ID Definitions */
#define P0_0            0x00
#define P0_1            0x01
#define P0_2            0x02
#define P0_3            0x03
#define P0_4            0x04
#define P0_5            0x05
#define P0_6            0x06
#define P0_7            0x07
#define P1_0            0x10
#define P1_1            0x11
#define P1_2            0x12
#define P1_3            0x13
#define P1_4            0x14
#define P1_5            0x15
#define P1_6            0x16
#define P1_7            0x17
#define P2_0            0x20
#define P2_1            0x21
#define P2_2            0x22
#define P2_3            0x23
#define P2_4            0x24
#define P2_5            0x25
#define P2_6            0x26
#define P2_7            0x27
#define P3_0            0x30
#define P3_1            0x31
#define P3_2            0x32
#define P3_3            0x33
#define P3_4            0x34
#define P3_5            0x35
#define P3_6            0x36
#define P3_7            0x37
#define P4_0            0x40
#define P4_1            0x41
#define P4_2            0x42
#define P4_3            0x43
#define P4_4            0x44
#define P4_5            0x45
#define P4_6            0x46
#define P4_7            0x47
#define P5_0            0x50
#define P5_1            0x51
#define P5_2            0x52
#define P5_3            0x53
#define P5_4            0x54
#define P5_5            0x55
#define P5_6            0x56
#define P5_7            0x57

#define WAKEUP_PIN      P2_2

/*--------------- Configurations for Test ---------------*/
// Disable this macro if there is already external pullup resistor when testing open-drain mode / quasi mode
#define ENABLE_INTERNAL_PULLUP_RES

// Config UART pins for test output
#define DBG_UART_TX     P3_7
#define DBG_UART_RX     P5_0


// GPIO Driver Wrapper Defines
#define GPIO_GET_PORT(PinId)    (PinId / 0x10)
#define GPIO_GET_BIT(PinId)     (PinId % 0x10)
#define P(PinId)                GPIO_PIN_ADDR(GPIO_GET_PORT(PinId), GPIO_GET_BIT(PinId))
//#define GPIO_GET_IRQn(PinId)    GPIO_IRQn   //((IRQn_Type)((int8_t)GPIO0_IRQn + GPIO_GET_PORT(PinId)))


extern const uint8_t GPIO_TEST_PINS[][2];
extern const uint8_t GPIO_TEST_GRP_NUM;

// GPIO Driver Wrapper Functions
extern void SYS_ConfigMFP(uint8_t PinID, uint32_t PinFunction);
extern void GPIO_SetModeByPin(uint8_t PinID, GPIO_ModeDef Mode);
extern void GPIO_EnablePullupPathByPin(uint32_t PinID);
extern void GPIO_DisablePullupPathByPin(uint32_t PinID);
extern void GPIO_EnableIntByPin(uint32_t PinID, GPIO_IntAttrDef IntAttribs);
extern void GPIO_DisableIntByPin(uint32_t PinID);
extern void GPIO_EnableDebounceByPin(uint32_t PinID);
extern void GPIO_DisableDebounceByPin(uint32_t PinID);
extern void GPIO_EnableDoutMaskByPin(uint32_t PinID);
extern void GPIO_DisableDoutMaskByPin(uint32_t PinID);
extern void GPIO_SetOutValByPin(uint32_t PinID, bool PinVal);
extern bool GPIO_GetOutValByPin(uint32_t PinID);
extern void GPIO_EnterDeepSleepMode(void);

// GPIO Test Cases
extern T_GPIO_TEST_RESULT GPIO_RegisterDefaultValueCheckCase0(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_PushPullModeTestCase1(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_OpenDrainModeTestCase2(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_QuasiBidirectionalModeTestCase3(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_DebounceTestCase4(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_InterruptTestCase5(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_WakeupTestCase6(uint8_t TargetPin, uint8_t AuxiliaryPin);
extern T_GPIO_TEST_RESULT GPIO_DoutWriteMaskTestCase7(uint8_t TargetPin, uint8_t AuxiliaryPin);

extern void GPIO_TestFunctionEnter(uint16_t TcIdx);

extern IRQn_Type GPIO_GetIRQn(uint8_t PinId);
#ifdef __cplusplus
}
#endif

#endif //__GPIO_COMMON_H__
