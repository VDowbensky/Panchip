/********************************************************* * 
@file       :fmc_small_data_read_write_test.c 
@brief      :flash small data read/write function test
@version    :v1.0 
@date       :19/10/31
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#include "PanSeries.h"
#include "fmc_common.h"

#define TEST_FLASH_ADDR     (0x30000)

const uint8_t DataByBytes[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
const uint16_t DataByHalfWords[8] = {0x1100, 0x3322, 0x5544, 0x7766, 0x9988, 0xBBAA, 0xDDCC, 0xFFEE};
const uint32_t DataByWords[4] = {0x33221100, 0x77665544, 0xBBAA9988, 0xFFEEDDCC};

uint32_t FMC_SmallDataReadWriteTest(void)
{
    SYS_TEST("[WARNING]\n");
    SYS_TEST("  - There is a limitation for the Small Data Write APIs: FMC_WriteByte() and FMC_WriteHalfWord().\n");
    SYS_TEST("  - That is, these two APIs should be called to write a specified flash page (256B) for no more\n");
    SYS_TEST("  - than 64 times after erase operation, which means a specified 256-bytes flash page could only\n");
    SYS_TEST("  - be wrote 64 bytes data by FMC_WriteByte(), or be wrote 128 bytes data by FMC_WriteHalfWord()!\n\n");

    SYS_TEST("Start Testing Small Data Read/Write..\n");

    /* Erase specific flash sector (4KB) for later use */
    FMC_EraseSector(FLCTL, TEST_FLASH_ADDR);

    /* Check if specific flash area is blank byte-by-byte */
    for (size_t i = 0; i < 4096; i++) {
        if (FMC_ReadByte(FLCTL, TEST_FLASH_ADDR + i, CMD_DREAD) != 0xFFu) {
            SYS_TEST("ERROR: Flash Area for Test is not Blank (Read Byte)\n");
            return 1;
        }
    }

    /* Check if specific flash area is blank word-by-word */
    for (size_t i = 0; i < 1024; i++) {
        if (FMC_ReadWord(FLCTL, TEST_FLASH_ADDR + i, CMD_DREAD) != 0xFFFFFFFFu) {
            SYS_TEST("ERROR: Flash Area for Test is not Blank (Read Word)\n");
            return 1;
        }
    }

    /* Write 16 bytes data to TEST_FLASH_ADDR by byte */
    for (size_t i = 0; i < 16; i++) {
        FMC_WriteByte(FLCTL, TEST_FLASH_ADDR + i, DataByBytes[i]);
    }

    /* Write 8 half-words data to TEST_FLASH_ADDR + 16 by half-word (2-bytes) */
    for (size_t i = 0; i < 8; i++) {
        FMC_WriteHalfWord(FLCTL, TEST_FLASH_ADDR + 16 + i * 2, DataByHalfWords[i]);
    }

    /* Check if data in specific flash area is expected byte-by-byte */
    for (size_t i = 0; i < 32; i++) {
        if (FMC_ReadByte(FLCTL, TEST_FLASH_ADDR + i, CMD_DREAD) != DataByBytes[i % 16]) {
            SYS_TEST("ERROR: Data in Flash Area for Test is not Expected (Read Byte)\n");
            return 1;
        }
    }

    /* Check if data in specific flash area is expected word-by-word */
    for (size_t i = 0; i < 8; i++) {
        if (FMC_ReadWord(FLCTL, TEST_FLASH_ADDR + i * 4, CMD_DREAD) != DataByWords[i % 4]) {
            SYS_TEST("ERROR: Data in Flash Area for Test is not Expected (Read Word)\n");
            return 1;
        }
    }

    SYS_TEST("Success\n");

    return 0;
}
