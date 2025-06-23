/**************************************************************************//**
 * @file     efuse_write_user_area.c
 * @version  V1.0
 * $Date:    20/10/15 17:08 $
 * @brief    Sample code to write data to User Area in eFuse.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "efuse_common.h"


T_EFUSE_TEST_RESULT eFuse_UserWriteProc(void)
{
    uint32_t address;
    uint8_t data[256] = {0,};
    int write_len;
    T_EFUSE_TEST_RESULT r = EFUSE_TST_OK;

    SYS_TEST("Write multiple data to User Area in eFuse.\nNow input start address (in hexadecimal format, e.g. 0x7C):\n");

    address = GetInputHexNumber();

    SYS_TEST("Now input data to write to eFuse (in hexadecimal format, e.g. abCDfe99):\n");

    write_len = GetInputHexString(data, sizeof(data));

    // Print data to write
    SYS_TEST("\teFuse Write, start address = 0x%02x, length = 0x%02x\n", address, write_len);
    SYS_TEST("\tdata = ");
    for (size_t i = 0; i < write_len; i++)
    {
        SYS_TEST("0x%02x ", data[i]);
    }
    SYS_TEST("\n");

    // Unlock Protected Registers
    SYS_UnlockReg();

    // Init eFuse module
    EFUSE_Init(EFUSE);

    // Write eFuse
    if (EFUSE_UserWrite(EFUSE, address, data, write_len) == false)
    {
        r = EFUSE_TST_FAIL;
        SYS_TEST("eFuse Write User Area Error, address range should be 0x7C ~ 0x7F!\n");
    }
    else
    {
        SYS_TEST("eFuse Write User Area Success!\n");
    }

    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);

    // Relock Protected Registers
    SYS_UnlockReg();

    return r;
}
