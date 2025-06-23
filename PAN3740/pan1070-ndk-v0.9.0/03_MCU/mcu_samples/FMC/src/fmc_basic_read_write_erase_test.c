/********************************************************* * 
@file       :fmc_basic_read_write_erase_test.c 
@brief      :flash basic functions test for Read/Write/Erase
@version    :v1.0 
@date       :19/10/31
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#include "PanSeries.h"
#include "fmc_common.h"

#define TEST_FLASH_ADDR     (0x20000)
#define DATA_BUF_SIZE       (4096 * 2)

static uint8_t SrcDataBuf[DATA_BUF_SIZE];
static uint8_t DstDataBuf[DATA_BUF_SIZE];

static void FillDataBuffer(uint8_t *buf, size_t buf_size)
{
    for (size_t i = 0; i < buf_size; i++) {
        buf[i] = i % 256;
    }
}

uint32_t FMC_BasicReadWriteEraseTest(void)
{
    SYS_TEST("Start Testing Flash Basic Read/Write/Erase..\n");

    FillDataBuffer(SrcDataBuf, DATA_BUF_SIZE);

    // Erase specific flash area for test
    for (size_t i = 0; i < DATA_BUF_SIZE; i += SECTOR_SIZE) {
        FMC_EraseSector(FLCTL, TEST_FLASH_ADDR + i);
    }

    // Write data to specific flash area
    if (FMC_WriteStream(FLCTL, TEST_FLASH_ADDR, SrcDataBuf, DATA_BUF_SIZE)) {
        SYS_TEST("ERROR: Write Flash Fail\n");
        return 1;
    }

    // Read data from specific flash area back to RAM buffer
    if (FMC_ReadStream(FLCTL, TEST_FLASH_ADDR, CMD_DREAD, DstDataBuf, DATA_BUF_SIZE)) {
        SYS_TEST("ERROR: Read Flash Fail\n");
        return 1;
    }

    // Check if the flash data read back is same with the source data
    if (memcmp(SrcDataBuf, DstDataBuf, DATA_BUF_SIZE)) {
        SYS_TEST("ERROR: Verify Flash Data Fail\n");
        return 1;
    }

    SYS_TEST("Success\n");

    return 0;
}
