/**************************************************************************//**
 * @file     efuse_common.h
 * @version  V1.0
 * $Date:    20/10/14 16:30 $
 * @brief    Common header file for eFuse test.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __EFUSE_COMMON_H__
#define __EFUSE_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum _T_EFUSE_TST_RESULT
{
    EFUSE_TST_OK,
    EFUSE_TST_FAIL,
} T_EFUSE_TEST_RESULT;


T_EFUSE_TEST_RESULT eFuse_RegisterDefaultValueCheck(void);
T_EFUSE_TEST_RESULT eFuse_ReadOneByteProc(void);
T_EFUSE_TEST_RESULT eFuse_WriteOneByteProc(void);
T_EFUSE_TEST_RESULT eFuse_ReadStreamProc(void);
T_EFUSE_TEST_RESULT eFuse_WriteStreamProc(void);
T_EFUSE_TEST_RESULT eFuse_UserReadProc(void);
T_EFUSE_TEST_RESULT eFuse_UserWriteProc(void);
T_EFUSE_TEST_RESULT eFuse_ReadWholeTable(void);
T_EFUSE_TEST_RESULT eFuse_SwitchAccessPermission(void);

extern void EFUSE_TestFunctionEnter(uint16_t TcIdx);
extern int GetInputHexString(uint8_t* byte_array, uint32_t byte_array_len);
extern uint32_t GetInputHexNumber(void);

#ifdef __cplusplus
}
#endif

#endif //__EFUSE_COMMON_H__
