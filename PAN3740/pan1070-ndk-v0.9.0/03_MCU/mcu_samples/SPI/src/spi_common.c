/**************************************************************************//**
 * @file     spi_common.c
 * @version  V1.0
 * $Date:    19/10/22 14:47 $
 * @brief    Common source file for SPI test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "spi_common.h"

uint16_t spiRxBuffer[SPI_RX_BUF_SIZE];
uint16_t spiTxBuffer[SPI_TX_BUF_SIZE];

volatile uint32_t spiRxBufIdx = 0;
uint32_t spiTxBufIdx = 0;

volatile bool spiTxDone = false;


T_SPI_TEST_RESULT (*const SPI_TestCase[])(void) = 
{
    SPI_RegisterDefaultValueCheckCase0,
    SPI_DataSizeSelectTestCase1,
    SPI_BaudrateTestCase2,
    SPI_FrameFormatTestCase3,
    SPI_InterruptTestCase4,
    SPI_TransferWithDmaTestCase5,
    SPI_SimpleTransmissionDemoCase6,
	SPI_DataLsbSendRcvTestCase7,
	SPI_ThreeWireDmaTransferTestCase8,
	SPI_ThreeWireSimpleTransmissionCase9
};

void SPI_TestFunctionEnter(uint16_t TcIdx)
{
    T_SPI_TEST_RESULT r = SPI_TST_OK;

    if (TcIdx >= sizeof(SPI_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (SPI_TestCase[TcIdx])();
    if (r != SPI_TST_OK)
    {
        SYS_TEST("SPI Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("SPI Test OK, Success case: %d\n", TcIdx);
    }
}


void SPI_HandleReceivedData(SPI_T* SPIx)
{
    while (!SPI_IsRxFifoEmpty(SPIx))
    {
        spiRxBuffer[spiRxBufIdx++] = SPI_ReceiveData(SPIx);
        if (spiRxBufIdx >= SPI_RX_BUF_SIZE)   //Rx buffer full
        {
            SYS_TEST("Error, too much data received, Rx buffer full!\n");
            spiRxBufIdx = 0;   //Reset Rx buf index
            break;
        }
    }
}

void SPI_HandleTransmittingData(SPI_T* SPIx)
{
    static uint32_t spiTxMark = 0;
    while (!SPI_IsTxFifoFull(SPIx))
    {
        if (spiTxMark < spiTxBufIdx)
        {
            SPI_SendData(SPIx, spiTxBuffer[spiTxMark++]);
        }
        else
        {
            spiTxMark = 0;  //Reset Tx mark while transmitting done
            SPI_DisableIrq(SPIx, SPI_IRQ_TX_HALF_EMPTY);    //Disable tx INT after transmitting done
            break;
        }
    }
}

void SPI_HandleProc(SPI_T* SPIx)
{
    if (SPI_IsIrqActive(SPIx, SPI_IRQ_RX_OVERRUN))
    {
        SPI_ClearIrq(SPIx, SPI_IRQ_RX_OVERRUN); //Clear overrun INT flag manually
        SYS_TEST("SPI Rx Overrun!\n");
        return;
    }

    if (SPI_IsIrqActive(SPIx, SPI_IRQ_RX_HALF_FULL))
    {
        SPI_HandleReceivedData(SPIx);
    }
    else if (SPI_IsIrqActive(SPIx, SPI_IRQ_RX_TIMEOUT))
    {
        SPI_HandleReceivedData(SPIx);
    }

    if (SPI_IsIrqActive(SPIx, SPI_IRQ_TX_HALF_EMPTY))
    {
        SPI_HandleTransmittingData(SPIx);
    }
}

void SPI0_IRQHandler(void)
{
    SPI_HandleProc(SPI0);
}

void SPI1_IRQHandler(void)
{
    SPI_HandleProc(SPI1);
}

void SPI_TestModuleInit(void)
{
    // Enable Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_SPI0, ENABLE);
    // Config Pinmux
	/*SPI0 Master*/
	SYS_SET_MFP(P1, 1, SPI0_MOSI);
	SYS_SET_MFP(P1, 2, SPI0_MISO);
	SYS_SET_MFP(P0, 3, SPI0_CS);
	SYS_SET_MFP(P0, 4, SPI0_CLK);
	GPIO_EnableDigitalPath(P0, BIT4);   //SPI CLK is input pin while act as slave
	GPIO_EnableDigitalPath(P0, BIT3);   //SPI CS is input pin while act as slave
	GPIO_EnableDigitalPath(P1, BIT1);   //SPI MOSI is input pin while act as slave
	GPIO_EnableDigitalPath(P1, BIT2);   //SPI MISO is input pin while act as master


    // Enable Clock
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_SPI1, ENABLE);
    // Config Pinmux
	/*SPI1 Slave*/
	SYS_SET_MFP(P2, 1, SPI1_MOSI);
	SYS_SET_MFP(P2, 4, SPI1_MISO);
	SYS_SET_MFP(P2, 2, SPI1_CLK);
	SYS_SET_MFP(P2, 3, SPI1_CS);
	GPIO_EnableDigitalPath(P2,BIT4 | BIT1 | BIT3 | BIT2);
}


// Receive multiple data from specified SPI, and return the size of actual data received
size_t SPI_RecvMultiDataAsync(SPI_T* SPIx, uint16_t* data, size_t expect_recv_size)
{
    size_t i = 0;
    for (; i < expect_recv_size; i++)
    {
        // Fetch data from Rx FIFO until it's empty
        if (!SPI_IsRxFifoEmpty(SPIx))
        {
            data[i] = SPI_ReceiveData(SPIx);
        }
        else
        {
            break;
        }
    }
    return i;
}

// Send multiple data to specified SPI, and return the size of actual data sent
size_t SPI_SendMultiDataAsync(SPI_T* SPIx, const uint16_t* data, size_t expect_send_size)
{
    size_t i = 0;
    for (; i < expect_send_size; i++)
    {
        // Fill data into Tx FIFO until it's full
        if (!SPI_IsTxFifoFull(SPIx))
        {
            SPI_SendData(SPIx, data[i]);
        }
        else
        {
            break;
        }
    }
    return i;
}

void SPI_MultiDataSendReceiveAsync(SPI_T* SPIx, const uint16_t* data_to_send, size_t expect_send_size, uint16_t* buf_to_recv, size_t expect_recv_size)
{
    size_t actual_sent_size = 0;
    size_t actual_recv_size = 0;

    while (!(actual_sent_size == expect_send_size && actual_recv_size == expect_recv_size))
    {
        actual_sent_size += SPI_SendMultiDataAsync(SPIx, data_to_send + actual_sent_size, expect_send_size - actual_sent_size);
        actual_recv_size += SPI_RecvMultiDataAsync(SPIx, buf_to_recv + actual_recv_size, expect_recv_size - actual_recv_size);
    }
}

void SPI_RecvMultiDataSync(SPI_T* SPIx, uint16_t* data, size_t expect_recv_size)
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

void SPI_SendMultiDataSync(SPI_T* SPIx, const uint16_t* data, size_t expect_send_size)
{
    size_t i = 0;

    while (i < expect_send_size)
    {
        if (!SPI_IsTxFifoFull(SPIx))
        {
            SPI_SendData(SPIx, data[i++]);
        }
    }
}

void SPI_Set3WireConfig(SPI_T* SPIx, 
						SPI_WriteReadSelDef wr,
						SPI_CmdDatDef dc)
{
	SPI_WriteReadCtrl(SPIx,wr);
	SPI_DataCmdSelCmdDat(SPIx,dc);
}

T_SPI_TEST_RESULT SPI_RegisterDefaultValueCheckCase0(void)
{
    SYS_TEST("Target SPI%d Register Default Values:\n", TGT_SPI_ID);
    SYS_TEST("---------------------------\n");
    SYS_TEST("CR0         = 0x%08x\n", TGT_SPI->CR0     );
    SYS_TEST("CR1         = 0x%08x\n", TGT_SPI->CR1     );
    SYS_TEST("DR          = 0x%08x\n", TGT_SPI->DR      );
    SYS_TEST("SR          = 0x%08x\n", TGT_SPI->SR      );
    SYS_TEST("CPSR        = 0x%08x\n", TGT_SPI->CPSR    );
    SYS_TEST("IMSC        = 0x%08x\n", TGT_SPI->IMSC    );
    SYS_TEST("RIS         = 0x%08x\n", TGT_SPI->RIS     );
    SYS_TEST("MIS         = 0x%08x\n", TGT_SPI->MIS     );
    SYS_TEST("ICR         = 0x%08x\n", TGT_SPI->ICR     );
    SYS_TEST("DMACR       = 0x%08x\n", TGT_SPI->DMACR   );
	
    SYS_TEST("\n");
    SYS_TEST("Auxiliary SPI%d Register Default Values:\n", AUX_SPI_ID);
    SYS_TEST("---------------------------\n");
    SYS_TEST("CR0         = 0x%08x\n", AUX_SPI->CR0     );
    SYS_TEST("CR1         = 0x%08x\n", AUX_SPI->CR1     );
    SYS_TEST("DR          = 0x%08x\n", AUX_SPI->DR      );
    SYS_TEST("SR          = 0x%08x\n", AUX_SPI->SR      );
    SYS_TEST("CPSR        = 0x%08x\n", AUX_SPI->CPSR    );
    SYS_TEST("IMSC        = 0x%08x\n", AUX_SPI->IMSC    );
    SYS_TEST("RIS         = 0x%08x\n", AUX_SPI->RIS     );
    SYS_TEST("MIS         = 0x%08x\n", AUX_SPI->MIS     );
    SYS_TEST("ICR         = 0x%08x\n", AUX_SPI->ICR     );
    SYS_TEST("DMACR       = 0x%08x\n", AUX_SPI->DMACR   );

    return SPI_TST_OK;
}

