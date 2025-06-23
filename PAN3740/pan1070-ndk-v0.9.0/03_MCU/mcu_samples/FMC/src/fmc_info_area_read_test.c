/********************************************************* * 
@file       :fmc_info_area_read_test.c.c 
@brief      :flash info area read function test
@version    :v1.0 
@date       :19/10/31
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#include "PanSeries.h"
#include "fmc_common.h"

uint32_t FMC_InfoAreaReadTest(void)
{
    uint8_t info[256] = {0};

    SYS_TEST("Read leading 256 bytes data of Flash INFO Area..\n");

    if (FMC_ReadInfoArea(FLCTL, 0x0, CMD_DREAD, info, sizeof(info))) {
        SYS_TEST("ERROR: Read flash INFO area fail, invalid parameter detected\n");
        return 1;
    }

    for (size_t i = 0; i < 32; i++) {
        for (size_t j = 0; j < 8; j++) {
            SYS_TEST("[%02X]=0x%02x, ", i * 8 + j, info[i * 8 + j]);
        }
        SYS_TEST("\n");
    }

    return 0;
}
