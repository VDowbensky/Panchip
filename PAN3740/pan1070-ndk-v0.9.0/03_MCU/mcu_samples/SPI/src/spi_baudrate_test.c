/**************************************************************************//**
 * @file     spi_baudrate_test.c
 * @version  V1.0
 * $Date:    19/10/24 15:30 $
 * @brief    SPI test case 2, test send/recv data under different baudrate.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"

#define BAUDRATE_100K   100000
#define BAUDRATE_500K   500000
#define BAUDRATE_1M     1000000
#define BAUDRATE_2M     2000000
#define BAUDRATE_5M     5000000
#define BAUDRATE_10M    10000000

static uint16_t TestDataBuf[8];

static const uint16_t TestValid16BitData[] = {0x0000, 0x0001, 0x0002, 0x0003, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};

static void SPI_PrintInfoCase2(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    16-bit data under 100K baudrate.                | \n");
    SYS_TEST("|    Input 'B'    16-bit data under 500K baudrate.                | \n");
    SYS_TEST("|    Input 'C'    16-bit data under 1M   baudrate.                | \n");
    SYS_TEST("|    Input 'D'    16-bit data under 2M   baudrate.                | \n");
    SYS_TEST("|    Input 'E'    16-bit data under 5M   baudrate.                | \n");
    SYS_TEST("|    Input 'F'    16-bit data under 10M  baudrate.                | \n");
    SYS_TEST("|    Input 'G'    16-bit data under baudrate by user.             | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

uint8_t CalcDivisorByBaudrateSpi(uint32_t baudrate)
{
    uint32_t apbclock = 0x0;

#ifndef FPGA_MODE
    apbclock = CLK_GetPeripheralFreq((void*)TGT_SPI);
#else
    uint32_t ahbclock = CLK_GetHCLKFreq();
    apbclock = ahbclock;
#endif

    uint32_t spi_div = apbclock / baudrate;

    //Filter invalid cases
    if (spi_div > 0xFE || spi_div < 0x2)
        return NULL;

    return (uint8_t)spi_div;
}

static void MultiDataSendReceiveSpi(const uint16_t* valid_data_to_send, size_t valid_data_size,
                                     const uint16_t* invalid_data_to_send, size_t invalid_data_size,
                                     uint16_t* buf_to_recv, size_t recv_buf_size)
{
    size_t actual_sent_size = 0;
    size_t actual_recv_size = 0;

    if (valid_data_size > recv_buf_size || invalid_data_size > recv_buf_size)
    {
        SYS_TEST("\nError, recv buffer is not enough!\n");
        return;
    }

    //1. Send/Recv valid data
    if (valid_data_to_send != NULL)
    {
        SYS_TEST("\nSend data by Target SPI (should be valid):");
        for (size_t i = 0; i < valid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", valid_data_to_send[i]);
        }
        SYS_TEST("\n");

        while (!(actual_sent_size == valid_data_size && actual_recv_size == valid_data_size))
        {
            actual_sent_size += SPI_SendMultiDataAsync(TGT_SPI, valid_data_to_send + actual_sent_size, valid_data_size - actual_sent_size);
            actual_recv_size += SPI_RecvMultiDataAsync(AUX_SPI, buf_to_recv + actual_recv_size, valid_data_size - actual_recv_size);
        }

        SYS_TEST("Receive data by Auxiliary SPI:");
        for (size_t i = 0; i < valid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", buf_to_recv[i]);
        }
        SYS_TEST("\n");
    }

    //2. Send/Recv invalid data
    if (invalid_data_to_send != NULL)
    {
        actual_sent_size = 0;
        actual_recv_size = 0;
        SYS_TEST("\nSend data by Target SPI (should be invalid):");
        for (size_t i = 0; i < invalid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", invalid_data_to_send[i]);
        }
        SYS_TEST("\n");

        while (!(actual_sent_size == valid_data_size && actual_recv_size == valid_data_size))
        {
            actual_sent_size += SPI_SendMultiDataAsync(TGT_SPI, invalid_data_to_send + actual_sent_size, invalid_data_size - actual_sent_size);
            actual_recv_size += SPI_RecvMultiDataAsync(AUX_SPI, buf_to_recv + actual_recv_size, invalid_data_size - actual_recv_size);
        }

        SYS_TEST("Receive data by Auxiliary SPI:");
        for (size_t i = 0; i < invalid_data_size; i++)
        {
            if (i % 8 == 0)
                SYS_TEST("\n");
            SYS_TEST(" 0x%04x", buf_to_recv[i]);
        }
        SYS_TEST("\n");
    }
}

static bool BaudrateTestProc(SPI_InitTypeDef spiInitStruct, uint32_t baudrate)
{
    uint8_t baud_div = CalcDivisorByBaudrateSpi(baudrate);
    if (baud_div == NULL)
    {
        return false;
    }
    spiInitStruct.SPI_baudRateDiv = (SPI_BaudRateDivDef)baud_div;
    /* Init Target SPI */
    spiInitStruct.SPI_role = SPI_RoleMaster;
    SPI_Init(TGT_SPI, &spiInitStruct);
    SPI_EnableSpi(TGT_SPI);
    /* Init Auxiliary SPI */
    spiInitStruct.SPI_role = SPI_RoleSlave;
    SPI_Init(AUX_SPI, &spiInitStruct);
    SPI_EnableSpi(AUX_SPI);
    /* Transmit and reseive data */
    MultiDataSendReceiveSpi(TestValid16BitData, sizeof(TestValid16BitData)/sizeof(uint16_t), NULL, 0, TestDataBuf, sizeof(TestDataBuf)/sizeof(uint16_t));
    /* Disable SPI after use */
    SPI_DisableSpi(TGT_SPI);
    SPI_DisableSpi(AUX_SPI);

    return true;
}

T_SPI_TEST_RESULT SPI_BaudrateTestCase2(void)
{
    char c;
	char s[32];
    SPI_InitTypeDef spiInitStruct;

    spiInitStruct.SPI_format = SPI_FormatMotorola;
    spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;

    while(1)
    {
        SPI_PrintInfoCase2();
        switch(c = getchar())
        {
        /* A. 16-bit data under 100K baudrate. */
        case 'A':
        case 'a':
            if (BaudrateTestProc(spiInitStruct, BAUDRATE_100K) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        /* B. 16-bit data under 500K baudrate. */
        case 'B':
        case 'b':
            if (BaudrateTestProc(spiInitStruct, BAUDRATE_500K) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        /* C. 16-bit data under 1M baudrate. */
        case 'C':
        case 'c':
            if (BaudrateTestProc(spiInitStruct, BAUDRATE_1M) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        /* D. 16-bit data under 2M baudrate. */
        case 'D':
        case 'd':
            if (BaudrateTestProc(spiInitStruct, BAUDRATE_2M) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        /* E. 16-bit data under 5M baudrate. */
        case 'E':
        case 'e':
            if (BaudrateTestProc(spiInitStruct, BAUDRATE_5M) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        /* F. 16-bit data under 10M baudrate. */
        case 'F':
        case 'f':
            if (BaudrateTestProc(spiInitStruct, BAUDRATE_10M) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        /* F. 16-bit data under 10M baudrate. */
        case 'G':
        case 'g':
			SYS_TEST("\ninput the baudrate value u want\n");
			gets(s);
			uint32_t baudrate = atoi(s);
			SYS_TEST("baudrate select:%d \n",baudrate);
            if (BaudrateTestProc(spiInitStruct, baudrate) == false)
            {
                SYS_TEST("Test failed, SPI baudrate is too high or to low!\n");
            }
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return SPI_TST_OK;
}
