/**************************************************************************//**
 * @file     efuse_common.c
 * @version  V1.0
 * $Date:    20/10/14 16:30 $
 * @brief    Common source file for eFuse test.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "efuse_common.h"


T_EFUSE_TEST_RESULT (*const EFUSE_TestCase[])(void) = 
{
    eFuse_RegisterDefaultValueCheck,
    eFuse_ReadOneByteProc,
    eFuse_ReadStreamProc,
    eFuse_UserReadProc,
    eFuse_UserWriteProc,
    eFuse_ReadWholeTable,
};

void EFUSE_TestFunctionEnter(uint16_t TcIdx)
{
    T_EFUSE_TEST_RESULT r = EFUSE_TST_OK;

    if (TcIdx >= sizeof(EFUSE_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (EFUSE_TestCase[TcIdx])();
    if (r != EFUSE_TST_OK)
    {
        SYS_TEST("EFUSE Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("EFUSE Test OK, Success case: %d\n", TcIdx);
    }
}


static int HexStrToByte(char* source, uint8_t* dest, uint32_t dest_len)
{
    // string length should be even
    uint32_t str_len = strlen(source);
    if (str_len % 2 != 0)
    {
        return -1;
    }

    // check if the dest array is large enough to save the output
    if (str_len / 2 > dest_len)
    {
        return -1;
    }

    for (uint32_t i = 0; i < str_len; i += 2)
    {
        uint8_t tmp_byte_high, tmp_byte_low;
        if (source[i] >= 'A' && source[i] <= 'F')
        {
            tmp_byte_high = source[i] - 'A' + 10;
        }
        else if (source[i] >= 'a' && source[i] <= 'f')
        {
            tmp_byte_high = source[i] - 'a' + 10;
        }
        else if (source[i] >= '0' && source[i] <= '9')
        {
            tmp_byte_high = source[i] - '0';
        }
        else
        {
            // Invalid input
            return -1;
        }

        if (source[i + 1] >= 'A' && source[i + 1] <= 'F')
        {
            tmp_byte_low = source[i + 1] - 'A' + 10;
        }
        else if (source[i + 1] >= 'a' && source[i + 1] <= 'f')
        {
            tmp_byte_low = source[i + 1] - 'a' + 10;
        }
        else if (source[i + 1] >= '0' && source[i + 1] <= '9')
        {
            tmp_byte_low = source[i + 1] - '0';
        }
        else
        {
            // Invalid input
            return -1;
        }

        dest[i / 2] = tmp_byte_high * 16 + tmp_byte_low;
    }

    // number of output bytes
    return str_len / 2;
}


int GetInputHexString(uint8_t* byte_array, uint32_t byte_array_len)
{
    char hex_str_buf[520];
    size_t i = 0;

    while ((hex_str_buf[i] = getchar()) != '\n')
    {
        if (i >= sizeof(hex_str_buf) - 1)    //Truncated if input string is too long
        {
            break;
        }
        i++;
    }

    if (hex_str_buf[i-1] == '\r')
        hex_str_buf[i-1] = '\0';
    else
        hex_str_buf[i] = '\0';

    int bytes_num = HexStrToByte(hex_str_buf, byte_array, byte_array_len);
    if (bytes_num == -1)
    {
        return -1;  // Error occurs
    }

    return bytes_num;
}


uint32_t GetInputHexNumber(void)
{
    char in_str[10];
    size_t i = 0;

    while((in_str[i] = getchar()) != '\n')
    {
        if (i >= sizeof(in_str) - 1)    //Truncated if input string is too long
        {
            break;
        }
        i++;
    }
    in_str[i] = '\0';

    return strtol(in_str, NULL, 16);
}


T_EFUSE_TEST_RESULT eFuse_RegisterDefaultValueCheck(void)
{
    SYS_TEST("eFuse Register Default Values:\n");
    SYS_TEST("---------------------------\n");
    SYS_TEST("EF_CTL                = 0x%08x\n", EFUSE->EF_CTL               );
    SYS_TEST("EF_ADDR               = 0x%08x\n", EFUSE->EF_ADDR              );
    SYS_TEST("EF_DAT                = 0x%08x\n", EFUSE->EF_DAT               );
    SYS_TEST("EF_VDD                = 0x%08x\n", EFUSE->EF_VDD               );
    SYS_TEST("EF_CMD                = 0x%08x\n", EFUSE->EF_CMD               );
    SYS_TEST("EF_TRG                = 0x%08x\n", EFUSE->EF_TRG               );
    SYS_TEST("EF_PROG_TIM1          = 0x%08x\n", EFUSE->EF_PROG_TIMING1      );
    SYS_TEST("EF_PROG_TIM2          = 0x%08x\n", EFUSE->EF_PROG_TIMING2      );
    SYS_TEST("EF_PROG_TIM3          = 0x%08x\n", EFUSE->EF_PROG_TIMING3      );
    SYS_TEST("EF_READ_TIM4          = 0x%08x\n", EFUSE->EF_READ_TIMING4      );
    SYS_TEST("EF_READ_TIM5          = 0x%08x\n", EFUSE->EF_READ_TIMING5      );
    SYS_TEST("EF_OP_ERROR           = 0x%08x\n", EFUSE->EF_OP_ERROR          );
    SYS_TEST("EF_VERIFY_DEBUG1      = 0x%08x\n", EFUSE->EF_VERIFY_DEBUG1     );
    SYS_TEST("EF_VERIFY_DEBUG2      = 0x%08x\n", EFUSE->EF_VERIFY_DEBUG2     );
    SYS_TEST("EF_VERIFY_DEBUG3      = 0x%08x\n", EFUSE->EF_VERIFY_DEBUG3     );
    SYS_TEST("EF_VERIFY_DEBUG4      = 0x%08x\n", EFUSE->EF_VERIFY_DEBUG4     );
    SYS_TEST("EF_FLASH_PERMISSION   = 0x%08x\n", EFUSE->EF_FLASH_PERMISSION  );
    SYS_TEST("---------------------------\n");

    return EFUSE_TST_OK;
}
