/**************************************************************************//**
 * @file     efuse_read_one_byte.c
 * @version  V1.0
 * $Date:    20/10/15 17:08 $
 * @brief    Sample code to read one byte data from eFuse.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "efuse_common.h"


T_EFUSE_TEST_RESULT eFuse_ReadOneByteProc(void)
{
    uint32_t address;
    uint8_t data;
    T_EFUSE_TEST_RESULT r = EFUSE_TST_OK;

    SYS_TEST("Read one byte of data from eFuse.\nNow input address (in hexadecimal format, e.g. 0x7C):\n");

    address = GetInputHexNumber();

    // Unlock Protected Registers
    SYS_UnlockReg();

    // Init eFuse module
    EFUSE_Init(EFUSE);

    // Read eFuse
    data = EFUSE_ReadByte(EFUSE, address);

    // Check if previous Read Operation succeeded or not
    if (EFUSE_GetErrorStatus(EFUSE) == EFUSE_STATUS_FAIL)
    {
        EFUSE_ClrErrorStatus(EFUSE);
        r = EFUSE_TST_FAIL;
        SYS_TEST("eFuse Read One Byte Error (unreadable address), the result is unreliable!\n");
    }
    else
    {
        SYS_TEST("eFuse Read One Byte Success!\n");
    }

    // Print result
    SYS_TEST("\taddress = 0x%02x, data = 0x%02x\n", address, data);

    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);

    // Relock Protected Registers
    SYS_UnlockReg();

    return r;
}
