/**************************************************************************//**
 * @file     efuse_full_table_read_test.c
 * @version  V1.0
 * $Date:    20/10/15 19:50 $
 * @brief    Sample code to read the whole eFuse table.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "efuse_common.h"


T_EFUSE_TEST_RESULT eFuse_ReadWholeTable(void)
{
    uint32_t address;
    uint8_t data;
    T_EFUSE_TEST_RESULT r = EFUSE_TST_OK;

    SYS_TEST("eFuse full table:\n");
    SYS_TEST("Addr:Data\tAddr:Data\tAddr:Data\tAddr:Data\n");

    // Unlock Protected Registers
    SYS_UnlockReg();

    // Init eFuse module
    EFUSE_Init(EFUSE);

    for (address = 0; address < 128; address++)
    {
        // Read eFuse
        data = EFUSE_ReadByte(EFUSE, address);

        SYS_TEST("0x%02x:0x%02x", address, data);

        // Check if previous Read Operation succeeded or not
        if (EFUSE_GetErrorStatus(EFUSE) == EFUSE_STATUS_FAIL)
        {
            EFUSE_ClrErrorStatus(EFUSE);
            SYS_TEST("(N/A)");
        }
        else
        {
            SYS_TEST("(OK)");
        }
        if ((address + 1) % 4 == 0)
            SYS_TEST("\n");
        else
            SYS_TEST("\t");
    }

    // Un-Init eFuse module
    EFUSE_UnInit(EFUSE);

    // Relock Protected Registers
    SYS_UnlockReg();

    return r;
}
