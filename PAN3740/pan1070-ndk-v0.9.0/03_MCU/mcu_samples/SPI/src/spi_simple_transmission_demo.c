/**************************************************************************//**
 * @file     spi_simple_transmission_demo.c
 * @version  V1.0
 * $Date:    20/03/03 16:50 $
 * @brief    SPI test case 6, simple transmission demo.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"

static uint8_t RecvBuffer[32] = {0};

static const uint8_t StrMasterSend[] = "Master Send, Slave Recv.";
static const uint8_t StrSlaveSend[] = "Slave Send, Master Recv.";

static void SPI_PrintInfoCase6(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|    (Just use Target SPI to demonstrate simple transmission)     | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Send data, act as Master.                       | \n");
    SYS_TEST("|    Input 'B'    Recv data, act as Slave.                        | \n");
    SYS_TEST("|    Input 'C'    Send data, act as Slave.                        | \n");
    SYS_TEST("|    Input 'D'    Recv data, act as Master.                       | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

static void SPI_RecvMultiByteSync(SPI_T* SPIx, uint8_t* data, size_t expect_recv_size)
{
    size_t i = 0;

    while (i < expect_recv_size)
    {
        if (!SPI_IsRxFifoEmpty(SPIx))
        {
            data[i++] = SPI_ReceiveData(SPIx);
        }
    }
}

static void SPI_MasterRecvMultiByteSync(SPI_T* SPIx, uint8_t* data, size_t expect_recv_size)
{
    size_t i = 0;

    while (i < expect_recv_size)
    {
        if (SPI_IsTxFifoEmpty(SPIx) && SPI_IsRxFifoEmpty(SPIx))
        {
            SPI_SendData(SPIx, '\0');   // Send dummy data to generate CLK for receving
            while (SPI_IsBusy(SPIx));   // Make sure enough clock pulses have been sent out
            data[i++] = SPI_ReceiveData(SPIx);
        }
        else
        {
            SYS_TEST("Warning, is not expected running to here!\n");
        }
    }
}

static void SPI_SendMultiByteSync(SPI_T* SPIx, const uint8_t* data, size_t expect_send_size)
{
    size_t i = 0;

    while (i < expect_send_size)
    {
        if (!SPI_IsTxFifoFull(SPIx))
        {
            SPI_SendData(SPIx, data[i++]);
        }
    }
    while(SPI_IsBusy(SPIx));    // Make sure all data in FIFO are sent out successfully
}


T_SPI_TEST_RESULT SPI_SimpleTransmissionDemoCase6(void)
{
    char c;
    SPI_InitTypeDef spiInitStruct =
    {
        .SPI_dataFrameSize = SPI_DataFrame_8b,
        .SPI_baudRateDiv = SPI_BaudRateDiv_250,
        .SPI_format = SPI_FormatMotorola,
        .SPI_CPOL = SPI_ClockPolarityLow,
        .SPI_CPHA = SPI_ClockPhaseFirstEdge,
    };

    while(1)
    {
        SPI_PrintInfoCase6();
        switch(c = getchar())
        {
        /* A. Send data, act as Master. */
        case 'A':
        case 'a':
            SYS_TEST("Send data, act as Master.\n");
            SPI_DisableSpi(TGT_SPI);
            spiInitStruct.SPI_role = SPI_RoleMaster;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Transmit data */
            SPI_SendMultiByteSync(TGT_SPI, StrMasterSend, strlen((const char*)StrMasterSend));
            while(!SPI_IsTxFifoEmpty(TGT_SPI)); // Wait for sending done
            break;
        /* B. Recv data, act as Slave. */
        case 'B':
        case 'b':
            SYS_TEST("Recv data, act as Slave...\n");
            SPI_DisableSpi(TGT_SPI);
            spiInitStruct.SPI_role = SPI_RoleSlave;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Receive data */
            SPI_RecvMultiByteSync(TGT_SPI, RecvBuffer, strlen((const char*)StrMasterSend));
            SYS_TEST("Data rcvd: \"%s\"\n", RecvBuffer);
            break;
        /* C. Send data, act as Slave. */
        case 'C':
        case 'c':
            SYS_TEST("Send data, act as Slave....\n");
            SPI_DisableSpi(TGT_SPI);
            spiInitStruct.SPI_role = SPI_RoleSlave;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Transmit data */
            SPI_SendMultiByteSync(TGT_SPI, StrSlaveSend, strlen((const char*)StrSlaveSend));
            while(!SPI_IsTxFifoEmpty(TGT_SPI)); // Wait for sending done
            break;
        /* D. Recv data, act as Master. */
        case 'D':
        case 'd':
            SYS_TEST("Recv data, act as Master.\n");
            SPI_DisableSpi(TGT_SPI);
            spiInitStruct.SPI_role = SPI_RoleMaster;
            SPI_Init(TGT_SPI, &spiInitStruct);
            SPI_EnableSpi(TGT_SPI);
            /* Receive data */
            SPI_MasterRecvMultiByteSync(TGT_SPI, RecvBuffer, strlen((const char*)StrMasterSend));
            SYS_TEST("Data rcvd: \"%s\"\n", RecvBuffer);
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    SPI_DisableSpi(TGT_SPI);
    return SPI_TST_OK;
}
