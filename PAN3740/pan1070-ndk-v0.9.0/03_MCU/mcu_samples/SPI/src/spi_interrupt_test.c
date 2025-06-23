/**************************************************************************//**
 * @file     spi_interrupt_test.c
 * @version  V1.0
 * $Date:    19/10/25 13:30 $
 * @brief    SPI test case 4, test transfer data with interrupt enabled.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"

#define PREFILLED_DATA_SIZE     4   //SPI Slave Tx FIFO Prefilled Data Size

static const uint16_t ConstDataForTgtSpiSend[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
};

static const uint16_t ConstDataForAuxSpiSend[] =
{
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
    0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
    0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
};

static uint16_t TestRxDataBuf[SPI_RX_BUF_SIZE];

static void SPI_PrintInfoCase4(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Send/Recv data with master interrupt enabled.   | \n");
    SYS_TEST("|    Input 'B'    Rx FIFO overrun test.                           | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}


static void InterruptSendRecvTestProc(SPI_InitTypeDef spiInitStruct)
{
    // Copy data to tx buffer
    memcpy(spiTxBuffer, ConstDataForTgtSpiSend, sizeof(ConstDataForTgtSpiSend));
    spiTxBufIdx = sizeof(ConstDataForTgtSpiSend) / sizeof(ConstDataForTgtSpiSend[0]);

    /* Init Target SPI */
    spiInitStruct.SPI_role = SPI_RoleMaster;
    SPI_Init(TGT_SPI, &spiInitStruct);

    /* Init Auxiliary SPI */
    spiInitStruct.SPI_role = SPI_RoleSlave;
    SPI_Init(AUX_SPI, &spiInitStruct);

    /* Enable SPI, start to generate CLK Signal by master */
    SPI_EnableSpi(TGT_SPI);
    SPI_EnableSpi(AUX_SPI);

    /* Prefill data in Tx FIFO of Auxiliary SPI before enable IRQ to insure Target
       SPI could receive data timely */
    SPI_SendMultiDataAsync(AUX_SPI, ConstDataForAuxSpiSend, PREFILLED_DATA_SIZE);

    /* Enable Interrupt of target SPI */
    SPI_EnableIrq(TGT_SPI, SPI_IRQ_ALL);
    NVIC_EnableIRQ(TGT_SPI_IRQn);

    /* Auxiliary SPI works at polling mode */
    SPI_MultiDataSendReceiveAsync(AUX_SPI, ConstDataForAuxSpiSend + PREFILLED_DATA_SIZE, sizeof(ConstDataForAuxSpiSend)/sizeof(ConstDataForAuxSpiSend[0]) - PREFILLED_DATA_SIZE, TestRxDataBuf, spiTxBufIdx);

    /* Wait for master receiving all data */
    while(!SPI_IsRxFifoEmpty(TGT_SPI));

    /* Disable Interrupts of target SPI */
    SPI_DisableIrq(TGT_SPI, SPI_IRQ_ALL);
    NVIC_DisableIRQ(TGT_SPI_IRQn);

    /* Disable SPI after use */
    SPI_DisableSpi(TGT_SPI);
    SPI_DisableSpi(AUX_SPI);

    SYS_TEST("\nSend data by Target SPI with Tx INT enabled:");
    for (size_t i = 0; i < spiTxBufIdx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", spiTxBuffer[i]);
    }
    SYS_TEST("\n");

    SYS_TEST("\nData received by Auxiliary SPI:");
    for (size_t i = 0; i < spiTxBufIdx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", TestRxDataBuf[i]);
    }
    SYS_TEST("\n");

    SYS_TEST("\nSend data by Auxiliary SPI:");
    for (size_t i = 0; i < sizeof(ConstDataForAuxSpiSend)/sizeof(ConstDataForAuxSpiSend[0]); i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", ConstDataForAuxSpiSend[i]);
    }
    SYS_TEST("\n");

    SYS_TEST("\nData received by Target SPI with Rx INT enabled:");
    for (size_t i = 0; i < spiRxBufIdx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", spiRxBuffer[i]);
    }
    SYS_TEST("\n");

    /* Reset Rx buf index */
    spiRxBufIdx = 0;
}

static void InterruptOverrunTestProc(SPI_InitTypeDef spiInitStruct)
{
    // Copy data to tx buffer
    memcpy(spiTxBuffer, ConstDataForTgtSpiSend, sizeof(ConstDataForTgtSpiSend));
    spiTxBufIdx = sizeof(ConstDataForTgtSpiSend) / sizeof(ConstDataForTgtSpiSend[0]);

    /* Init Target SPI */
    spiInitStruct.SPI_role = SPI_RoleMaster;
    SPI_Init(TGT_SPI, &spiInitStruct);

    /* Init Auxiliary SPI */
    spiInitStruct.SPI_role = SPI_RoleSlave;
    SPI_Init(AUX_SPI, &spiInitStruct);

    /* Enable SPI, start to generate CLK Signal by master */
    SPI_EnableSpi(TGT_SPI);
    SPI_EnableSpi(AUX_SPI);

    /* Only Enable Tx and Overrun Interrupt of target SPI */
    SPI_EnableIrq(TGT_SPI, SPI_IRQ_TX_HALF_EMPTY);
    SPI_EnableIrq(TGT_SPI, SPI_IRQ_RX_OVERRUN);
    NVIC_EnableIRQ(TGT_SPI_IRQn);

    SYS_delay_10nop(10000); // Delay for overrun INT log printing

    /* Disable Interrupts of target SPI */
    SPI_DisableIrq(TGT_SPI, SPI_IRQ_RX_OVERRUN);
    NVIC_DisableIRQ(TGT_SPI_IRQn);

    /* Disable SPI after use */
    SPI_DisableSpi(TGT_SPI);
    SPI_DisableSpi(AUX_SPI);
}

T_SPI_TEST_RESULT SPI_InterruptTestCase4(void)
{
    char c;
    SPI_InitTypeDef spiInitStruct =
    {
        .SPI_format = SPI_FormatMotorola,
        .SPI_CPOL = SPI_ClockPolarityLow,
        .SPI_CPHA = SPI_ClockPhaseSecondEdge,
        .SPI_dataFrameSize = SPI_DataFrame_8b,
        .SPI_baudRateDiv = SPI_BaudRateDiv_250,
    };

    while(1)
    {
        SPI_PrintInfoCase4();
        switch(c = getchar())
        {
        /* A. Transmit and receive data with master interrupt enabled. */
        case 'A':
        case 'a':
            InterruptSendRecvTestProc(spiInitStruct);
            break;
        /* B. Rx FIFO overrun test. */
        case 'B':
        case 'b':
            InterruptOverrunTestProc(spiInitStruct);
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
