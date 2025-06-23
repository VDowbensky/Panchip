/**************************************************************************//**
 * @file     efuse_read_multiple_data.c
 * @version  V1.0
 * $Date:    20/10/15 17:08 $
 * @brief    Sample code to read multiple data from eFuse.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "efuse_common.h"


T_EFUSE_TEST_RESULT eFuse_ReadStreamProc(void)
{
    uint32_t address;
    uint8_t data[256] = {0,};
    uint16_t read_len;
    T_EFUSE_TEST_RESULT r = EFUSE_TST_OK;

    SYS_TEST("Read multiple data from eFuse.\nNow input start address (in hexadecimal format, e.g. 0x1E):\n");

    address = GetInputHexNumber();

    SYS_TEST("Now input number of bytes to read (in hexadecimal format, e.g. 0x10):\n");

    read_len = GetInputHexNumber();;

    // Unlock Protected Registers
    SYS_UnlockReg();

    // Init eFuse module
    EFUSE_Init(EFUSE);

    // Read eFuse
    if (EFUSE_Read(EFUSE, address, data, read_len) == false)
    {
        r = EFUSE_TST_FAIL;
        SYS_TEST("eFuse Read Multiple Data Error (unreadable address), the result is unreliable!\n");
    }
    else
    {
        SYS_TEST("eFuse Read Multiple Data Success!\n");
    }

    // Print result
    SYS_TEST("\tstart address = 0x%02x, length = 0x%02x\n", address, read_len);
    SYS_TEST("\tdata = ");
    for (size_t i = 0; i < read_len; i++)
    {
        SYS_TEST("0x%02x ", data[i]);
    }
    SYS_TEST("\n");

    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);

    // Relock Protected Registers
    SYS_UnlockReg();

    return r;
}
