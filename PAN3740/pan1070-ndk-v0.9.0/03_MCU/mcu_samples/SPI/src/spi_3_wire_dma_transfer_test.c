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

extern volatile bool gTfrFlagRx;
extern volatile bool gTfrFlagTx;

extern uint32_t RxChNum;
extern uint32_t TxChNum;

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


static void MultiDataSendReceiveDma(uint32_t recv_size_in_byte, uint32_t send_size_in_byte)
{

    /*dma init*/
    DMAC_Init(DMA);
	NVIC_EnableIRQ(DMA_IRQn);

	memcpy(DmaSendDataBuf, ConstDataForTgtSpiSend, sizeof(ConstDataForTgtSpiSend));
    DMAC_ChannelConfigTypeDef TxConfigTmp =
    {
        /* Set dma channel control & config register */
        .TransferType    = DMAC_TransferType_Mem2Per,
        // Destination config (Peripheral)
        .AddrChangeDst   = DMAC_AddrChange_NoChange,
        .BurstLenDst     = DMAC_BurstLen_1,     //According to peripheral FIFO threshold
        .DataWidthDst    = DMAC_DataWidth_16,   //According to peripheral FIFO width
        .HandshakeDst    = DMAC_Handshake_Hardware,
        .PeripheralDst   = TGT_SPI_DMA_DST,     //Config to corresponding peripheral
        // Source config (Memory)
        .AddrChangeSrc   = DMAC_AddrChange_Increment,
        .BurstLenSrc     = DMAC_BurstLen_1,
        .DataWidthSrc    = DMAC_DataWidth_32,
        .HandshakeSrc    = DMAC_Handshake_Default,
        // General config
        .FlowControl     = DMAC_FlowControl_DMA,
        .IntEnable       = ENABLE,
        /* Items in TxConfigTmp which are not influenced here is initialized as 0 by compiler. */
    };

    /*get free dma channel;*/
	TxChNum = DMAC_AcquireChannel(DMA);
	/*enable dma transfer interrupt*/
	DMAC_ClrIntFlagMsk(DMA,TxChNum,DMAC_FLAG_INDEX_TFR);
	DMAC_SetChannelConfig(DMA,TxChNum,&TxConfigTmp);

    /* Condition check */
    uint32_t DataWidthInByteSrc = 1 << TxConfigTmp.DataWidthSrc;
    uint32_t IsNotDivisible = send_size_in_byte % DataWidthInByteSrc;
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

	/*start dma tx channel*/
    DMAC_StartChannel(DMA, TxChNum, DmaSendDataBuf, (void*)&(TGT_SPI->DR), BlockSizeTx);

//	while(!(gTfrFlagRx && gTfrFlagTx));
	while(!gTfrFlagTx){}
    gTfrFlagRx = false;
    gTfrFlagTx = false;

	/*release tx dma channel*/
	DMAC_ReleaseChannel(DMA,TxChNum);

    /*disable dma*/
    DMAC_DeInit(DMA);

}


T_SPI_TEST_RESULT SPI_ThreeWireDmaTransferTestCase8(void)
{
    char dc_line,c;
	char s[32];
    SPI_InitTypeDef spiInitStruct =
    {
        .SPI_format = SPI_FormatMotorola,
        .SPI_CPOL = SPI_ClockPolarityLow,
        .SPI_CPHA = SPI_ClockPhaseFirstEdge,
        .SPI_dataFrameSize = SPI_DataFrame_8b,
        .SPI_baudRateDiv = SPI_BaudRateDiv_250,
    };

	uint32_t baudrate = 20;
	SYS_TEST("\nspi baudrate value: %d\n",FREQ_32MHZ/(baudrate*2));
	spiInitStruct.SPI_baudRateDiv = baudrate;
    /* Init Target SPI */
    spiInitStruct.SPI_role = SPI_RoleMaster;
    SPI_Init(TGT_SPI, &spiInitStruct);

    /* Enable SPI, start to generate CLK Signal by master */
    SPI_EnableSpi(TGT_SPI);


	SYS_TEST("\ndc output line enable or not\n");
	SYS_TEST("1-->dc output line enable\n");
	SYS_TEST("0-->dc output line disable\n");
	dc_line = getchar();
	switch(dc_line)
	{
	case '1':
		SPI_DataCmdLineEnable(TGT_SPI,ENABLE);
//		SPI_DataCmdLineEnable(AUX_SPI,ENABLE);
		break;
	case '0':
	default:
        SPI_DataCmdLineEnable(TGT_SPI,DISABLE);
//		SPI_DataCmdLineEnable(AUX_SPI,DISABLE);
        break;
	}
	SPI_SetWireNum(TGT_SPI,SPI_WireNumThree);
	SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_CommandSelect);
	TGT_SPI->DR = 0xa; //must set a invalid data
	SYS_delay_10nop(20);
	SPI_Set3WireConfig(TGT_SPI,SPI_3WireWrite,SPI_DataSelect);

    while(1)
    {
        SPI_PrintInfoCase5();
		c = getchar();
        switch(c)
        {
        /* A. Burst transaction. */
        case 'A':
        case 'a':
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
