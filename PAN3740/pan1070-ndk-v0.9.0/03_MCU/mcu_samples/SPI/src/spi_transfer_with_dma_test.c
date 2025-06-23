/**************************************************************************//**
 * @file     spi_transfer_with_dma_test.c
 * @version  V1.0
 * $Date:    19/10/28 10:30 $
 * @brief    SPI test case 5, test SPI Tx/Rx with DMA.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "spi_common.h"

#define PREFILLED_DATA_SIZE     8   //SPI Slave Tx FIFO Prefilled Data Size

volatile bool gTfrFlagRx = false;
volatile bool gTfrFlagTx = false;

uint32_t RxChNum = 0xFF;
uint32_t TxChNum = 0xFF;

static uint16_t TestRxDataBuf[SPI_RX_BUF_SIZE];

static uint16_t DmaRcvDataBuf[256];
static uint16_t DmaSendDataBuf[256];

// DMA interrupt trigger counter for debugging
static volatile uint32_t idx_tfr = 0, idx_srctfr = 0, idx_srctfr_rx = 0, idx_dsttfr = 0, idx_dsttfr_tx = 0, idx_err_rx = 0, idx_err_tx = 0;

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

static void SPI_PrintInfoCase5(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Burst transaction.                              | \n");
    SYS_TEST("|    Input 'B'    Single transaction.                             | \n");
    SYS_TEST("|    Input 'C'    Both single and burst transactions.             | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}

void DMA_IRQHandler(void)
{
    /* Handle DMA Transfer Complete Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_TFR))
    {
        idx_tfr++;
        // Handle SPI Rx Channel
        if(DMAC_IntFlag(DMA, RxChNum, DMAC_FLAG_INDEX_TFR)){
            gTfrFlagRx = true;
            DMAC_ClrIntFlag(DMA, RxChNum, DMAC_FLAG_INDEX_TFR);
        }
        // Handle SPI Tx Channel
        if(DMAC_IntFlag(DMA, TxChNum, DMAC_FLAG_INDEX_TFR)){
            gTfrFlagTx = true;
            DMAC_ClrIntFlag(DMA, TxChNum, DMAC_FLAG_INDEX_TFR);
        }
    }
    /* Handle Source Transation Complete Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_SRCTFR))
    {
        idx_srctfr++;
        if(DMAC_IntFlag(DMA, RxChNum, DMAC_FLAG_INDEX_SRCTFR)){
            idx_srctfr_rx++;
            DMAC_ClrIntFlag(DMA, RxChNum, DMAC_FLAG_INDEX_SRCTFR);
        }
    }
    /* Handle Destination Transaction Complete Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_DSTTFR))
    {
        idx_dsttfr++;
        if(DMAC_IntFlag(DMA, TxChNum, DMAC_FLAG_INDEX_DSTTFR)){
            idx_dsttfr_tx++;
            DMAC_ClrIntFlag(DMA, TxChNum, DMAC_FLAG_INDEX_DSTTFR);
        }
    }
    /* Handle Error Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_ERR))
    {
        if(DMAC_IntFlag(DMA, RxChNum, DMAC_FLAG_INDEX_ERR)){
            idx_err_rx++;
            DMAC_ClrIntFlag(DMA, RxChNum, DMAC_FLAG_INDEX_ERR);
        }
        if(DMAC_IntFlag(DMA, TxChNum, DMAC_FLAG_INDEX_ERR)){
            idx_err_tx++;
            DMAC_ClrIntFlag(DMA, TxChNum, DMAC_FLAG_INDEX_ERR);
        }
    }
}

static void MultiDataSendReceiveDma(uint32_t recv_size_in_byte, uint32_t send_size_in_byte)
{
    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    /********************** 1. Config DMA Rx Channel *********************/
    DMAC_ChannelConfigTypeDef RxConfigTmp =
    {
        /* Set dma channel control & config register */
        .TransferType    = DMAC_TransferType_Per2Mem,
        // Destination config (Memory)
        .AddrChangeDst   = DMAC_AddrChange_Increment,
        .BurstLenDst     = DMAC_BurstLen_4,
        .DataWidthDst    = DMAC_DataWidth_32,
        .HandshakeDst    = DMAC_Handshake_Default,
        // Source config (Peripheral)
        .AddrChangeSrc   = DMAC_AddrChange_NoChange,
        .BurstLenSrc     = DMAC_BurstLen_4,     //According to peripheral FIFO threshold
        .DataWidthSrc    = DMAC_DataWidth_16,   //According to peripheral FIFO width
        .HandshakeSrc    = DMAC_Handshake_Hardware,
        .PeripheralSrc   = TGT_SPI_DMA_SRC,     //Config to corresponding peripheral
        // General config
        .FlowControl     = DMAC_FlowControl_DMA,
        .IntEnable       = ENABLE,
        /* Items in RxConfigTmp which are not influenced here is initialized as 0 by compiler. */
    };

    /* Get free DMA channel */
    RxChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_SRCTFR);
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_ERR);

    DMAC_SetChannelConfig(DMA, RxChNum, &RxConfigTmp);

    /* Condition check */
    uint32_t DataWidthInByteSrc = 1 << RxConfigTmp.DataWidthSrc;
    uint32_t IsNotDivisible = recv_size_in_byte % DataWidthInByteSrc;
    uint32_t BlockSizeRx = recv_size_in_byte / DataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

    /* To check if send_size_in_byte is integer multiples of DataWidthInByte */
    if (IsNotDivisible)
    {
        SYS_TEST("Error, data to receive is not integer multiples of DataWidthInByte!\n");
        return;
    }

    /* Insure block size is not 0 */
    if (!BlockSizeRx)
    {
        SYS_TEST("Error, block size is 0!\n");
        return;
    }

    /********************** 2. Config DMA Tx Channel *********************/
    memcpy(DmaSendDataBuf, ConstDataForTgtSpiSend, sizeof(ConstDataForTgtSpiSend));

    DMAC_ChannelConfigTypeDef TxConfigTmp =
    {
        /* Set dma channel control & config register */
        .TransferType    = DMAC_TransferType_Mem2Per,
        // Destination config (Peripheral)
        .AddrChangeDst   = DMAC_AddrChange_NoChange,
        .BurstLenDst     = DMAC_BurstLen_4,     //According to peripheral FIFO threshold
        .DataWidthDst    = DMAC_DataWidth_16,   //According to peripheral FIFO width
        .HandshakeDst    = DMAC_Handshake_Hardware,
        .PeripheralDst   = TGT_SPI_DMA_DST,     //Config to corresponding peripheral
        // Source config (Memory)
        .AddrChangeSrc   = DMAC_AddrChange_Increment,
        .BurstLenSrc     = DMAC_BurstLen_4,
        .DataWidthSrc    = DMAC_DataWidth_32,
        .HandshakeSrc    = DMAC_Handshake_Default,
        // General config
        .FlowControl     = DMAC_FlowControl_DMA,
        .IntEnable       = ENABLE,
        /* Items in TxConfigTmp which are not influenced here is initialized as 0 by compiler. */
    };

    /* Get free DMA channel */
    TxChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_DSTTFR);
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_ERR);

    DMAC_SetChannelConfig(DMA, TxChNum, &TxConfigTmp);

    /* Condition check */
    DataWidthInByteSrc = 1 << TxConfigTmp.DataWidthSrc;
    IsNotDivisible = send_size_in_byte % DataWidthInByteSrc;
    uint32_t BlockSizeTx = send_size_in_byte / DataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

    /* To check if send_size_in_byte is integer multiples of DataWidthInByte */
    if (IsNotDivisible)
    {
        SYS_TEST("Error, data to send is not integer multiples of DataWidthInByte!\n");
        return;
    }

    /* To check if block size is not 0 */
    if (!BlockSizeTx)
    {
        SYS_TEST("Error, block size is 0!\n");
        return;
    }

    /***************** 3. Start DMA and SPI send/receive *****************/
    SYS_TEST("Start DMA Rx/Tx...\n");

    /* Prefill data in Tx FIFO of Auxiliary SPI before DMA start to insure Target
       SPI could receive data timely */
    size_t ActualSendSize = 0;
    if (recv_size_in_byte / sizeof(ConstDataForAuxSpiSend[0]) > PREFILLED_DATA_SIZE)
        ActualSendSize = SPI_SendMultiDataAsync(AUX_SPI, ConstDataForAuxSpiSend, PREFILLED_DATA_SIZE);
    else
        ActualSendSize = SPI_SendMultiDataAsync(AUX_SPI, ConstDataForAuxSpiSend, recv_size_in_byte / sizeof(ConstDataForAuxSpiSend[0]));

    /* Start DMA Rx channel */
    DMAC_StartChannel(DMA, RxChNum, (void*)&(TGT_SPI->DR), DmaRcvDataBuf, BlockSizeRx);
    /* Start DMA Tx channel */
    DMAC_StartChannel(DMA, TxChNum, DmaSendDataBuf, (void*)&(TGT_SPI->DR), BlockSizeTx);

    /* Polling data, Auxiliary SPI to send/receive */
    SPI_MultiDataSendReceiveAsync(
                    AUX_SPI,
                    ConstDataForAuxSpiSend + ActualSendSize,
                    recv_size_in_byte / sizeof(ConstDataForAuxSpiSend[0]) - ActualSendSize,
                    TestRxDataBuf,
                    send_size_in_byte / sizeof(ConstDataForTgtSpiSend[0]));

    // Wait for DMA transfer done
    while(!(gTfrFlagRx && gTfrFlagTx));
    gTfrFlagRx = false;
    gTfrFlagTx = false;

    /* Release DMA Rx channel */
    DMAC_ReleaseChannel(DMA, RxChNum);
    DMAC_ReleaseChannel(DMA, TxChNum);
    /* Disable DMA */
    DMAC_DeInit(DMA);

    // Print DMA Tx Channel Info
    SYS_TEST("Send data by DMA done (channel=%d, data_len=%d bytes).\n", TxChNum, send_size_in_byte);
    SYS_TEST("DMA Interrupt Trigger Count: tfr=%d dsttfr=%d, dsttfr_tx=%d, err_tx=%d\n",
        idx_tfr, idx_dsttfr, idx_dsttfr_tx, idx_err_tx);
    // Clear debugging flags
    idx_tfr = idx_dsttfr = idx_dsttfr_tx = idx_err_tx = 0;
    // Print out all sent data of master by DMA
    SYS_TEST("\nData sent by DMA:");
    for (size_t i = 0; i < send_size_in_byte / (1 << TxConfigTmp.DataWidthDst); i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%04x", DmaSendDataBuf[i]);
    }
    // Print out all received data by Auxiliary SPI
    SYS_TEST("\nData received by Auxiliary SPI:");
    for (size_t i = 0; i < send_size_in_byte / (1 << TxConfigTmp.DataWidthDst); i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%04x", TestRxDataBuf[i]);
    }
    SYS_TEST("\n\n");

    // Print DMA Rx Channel Info
    SYS_TEST("Data received by DMA done (channel=%d, data_len=%d bytes):\n", RxChNum, recv_size_in_byte);
    SYS_TEST("DMA Interrupt Trigger Count: tfr=%d, srctfr=%d, srctfr_rx=%d, err_rx=%d\n",
        idx_tfr, idx_srctfr, idx_srctfr_rx, idx_err_rx);
    // Clear flags
    idx_tfr = idx_srctfr = idx_srctfr_rx = idx_err_rx = 0;
    // Print out all sent data by Auxiliary SPI
    SYS_TEST("\nData sent by Auxiliary SPI:");
    for (size_t i = 0; i < BlockSizeRx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%04x", ConstDataForAuxSpiSend[i]);
    }
    SYS_TEST("\n");
    // Print out all received data by DMA
    SYS_TEST("\nData received by DMA:");
    for (size_t i = 0; i < BlockSizeRx; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%04x", DmaRcvDataBuf[i]);
    }
    SYS_TEST("\n");
}


T_SPI_TEST_RESULT SPI_TransferWithDmaTestCase5(void)
{
    char c;
	char s[32];
    SPI_InitTypeDef spiInitStruct =
    {
        .SPI_format = SPI_FormatMotorola,
        .SPI_CPOL = SPI_ClockPolarityLow,
        .SPI_CPHA = SPI_ClockPhaseSecondEdge,
        .SPI_dataFrameSize = SPI_DataFrame_16b,
        .SPI_baudRateDiv = SPI_BaudRateDiv_250,
    };

	SYS_TEST("\ninput the baudrate divider value u want\n");
	gets(s);
	uint32_t baudrate = atoi(s);
	SYS_TEST("\nspi baudrate value: %d\n",FREQ_32MHZ/(baudrate*2));
	spiInitStruct.SPI_baudRateDiv = baudrate;
    /* Init Target SPI */
    spiInitStruct.SPI_role = SPI_RoleMaster;
    SPI_Init(TGT_SPI, &spiInitStruct);
    /* Init Auxiliary SPI */
    spiInitStruct.SPI_role = SPI_RoleSlave;
    SPI_Init(AUX_SPI, &spiInitStruct);
    /* Enable SPI, start to generate CLK Signal by master */
    SPI_EnableSpi(TGT_SPI);
    SPI_EnableSpi(AUX_SPI);

    while(1)
    {
        SPI_PrintInfoCase5();
        switch(c = getchar())
        {
        /* A. Burst transaction. */
        case 'A':
        case 'a':
            SPI_EnableDmaRx(TGT_SPI);
            SPI_EnableDmaTx(TGT_SPI);
            MultiDataSendReceiveDma(128, 128);
            SPI_DisableDmaRx(TGT_SPI);
            SPI_DisableDmaTx(TGT_SPI);
            break;
        /* B. Single transaction. */
        case 'B':
        case 'b':
            SPI_EnableDmaRx(TGT_SPI);
            SPI_EnableDmaTx(TGT_SPI);
            MultiDataSendReceiveDma(4, 4);  //Receive/Send 4 bytes (1 word) (< burst len 8 bytes)
            SPI_DisableDmaRx(TGT_SPI);
            SPI_DisableDmaTx(TGT_SPI);
            break;
        /* C. Burst and Single transaction. */
        case 'C':
        case 'c':
            SPI_EnableDmaRx(TGT_SPI);
            SPI_EnableDmaTx(TGT_SPI);
            MultiDataSendReceiveDma(132, 132);
            SPI_DisableDmaRx(TGT_SPI);
            SPI_DisableDmaTx(TGT_SPI);
            break;
        case 0x1B:  //Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    /* Disable SPI after use */
    SPI_DisableSpi(TGT_SPI);
    SPI_DisableSpi(AUX_SPI);

    return SPI_TST_OK;
}
