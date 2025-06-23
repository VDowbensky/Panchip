/**************************************************************************//**
 * @file     uart_transfer_with_dma_test.c
 * @version  V1.0
 * $Date:    19/10/11 10:30 $
 * @brief    UART test case 7, test UART Tx/Rx with DMA.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "uart_common.h"

static volatile bool gTfrFlag = false;
static volatile bool gErrFlag;

// DMA interrupt trigger counter for debugging
static volatile uint32_t idx_tfr = 0, idx_srctfr = 0, idx_dsttfr = 0, idx_blk = 0, idx_err = 0;

static const uint8_t Const8BitData[] =
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
};

static void UART_PrintInfoCase7(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    < Baudrate=115200, Parity=NONE, DataBits=8, StopBits=1 >     | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Burst transaction receiving end (uart2mem).     | \n");
    SYS_TEST("|    Input 'B'    Burst transaction transmitting end (mem2uart).  | \n");
    SYS_TEST("|    Input 'C'    Single transaction receiving end (uart2mem).    | \n");
    SYS_TEST("|    Input 'D'    Single transaction transmitting end (mem2uart). | \n");
    SYS_TEST("|    Input 'E'    Both transactions receiving end (uart2mem).     | \n");
    SYS_TEST("|    Input 'F'    Both transactions transmitting end (mem2uart).  | \n");
    SYS_TEST("|    Input 'G'    DMA with UART AFC receiving end (uart2mem).     | \n");
    SYS_TEST("|    Input 'H'    DMA with UART AFC transmitting end (mem2uart).  | \n");
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
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR)){
            //SYS_TEST("0_TRF0 \r\n");
            gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR)){
            //SYS_TEST("1_TRF1 \r\n");
            gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR)){
            //SYS_TEST("2_TRF2 \r\n");
            gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR);          
        }
    }
    /* Handle Block Transfer Complete Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_BLK))
    {
        idx_blk++;
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_BLK)){
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_BLK);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_BLK)){
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_BLK);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_BLK)){
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_BLK);
        }
    }
    /* Handle Source Transation Complete Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_SRCTFR))
    {
        idx_srctfr++;
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_SRCTFR)){
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_SRCTFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_SRCTFR)){
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_SRCTFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_SRCTFR)){
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_SRCTFR);
        }
    }
    /* Handle Destination Transaction Complete Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_DSTTFR))
    {
        idx_dsttfr++;
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_DSTTFR)){
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_DSTTFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_DSTTFR)){
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_DSTTFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_DSTTFR)){
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_DSTTFR);
        }
    }
    /* Handle Error Interrupt */
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_ERR))
    {
        idx_err++;
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_ERR)){
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_ERR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_ERR)){
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_ERR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_ERR)){
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_ERR);
        }
    }
}

static void MultiDataReceiveDma(uint32_t recv_size_in_byte)
{
    uint32_t RxChNum = 0xFF;
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};   //Init config struct and set items as all 0s
    uint8_t RcvDataBuf[512];

    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    /* Set dma channel control & config register */
    RxConfigTmp.TransferType    = DMAC_TransferType_Per2Mem;
    // Destination config (Memory)
    RxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Increment;
    RxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    RxConfigTmp.DataWidthDst    = DMAC_DataWidth_32;
    RxConfigTmp.HandshakeDst    = DMAC_Handshake_Default;
    // Source config (Peripheral)
    RxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    RxConfigTmp.BurstLenSrc     = DMAC_BurstLen_8;  //According to peripheral FIFO threshold
    RxConfigTmp.DataWidthSrc    = DMAC_DataWidth_8; //According to peripheral FIFO width
    RxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    RxConfigTmp.PeripheralSrc   = TGT_UART_DMA_SRC; //Config to corresponding peripheral

    RxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    RxConfigTmp.IntEnable       = ENABLE;

    /* Get free DMA channel */
    RxChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_TFR);
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_SRCTFR);
#if 0
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_DSTTFR);   //DSTTFR INT is meaningless if destination is memory
#endif
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_BLK);
    DMAC_ClrIntFlagMsk(DMA, RxChNum, DMAC_FLAG_INDEX_ERR);

    DMAC_SetChannelConfig(DMA, RxChNum, &RxConfigTmp);

    /* Condition check */
    uint32_t DataWidthInByteSrc = 1 << RxConfigTmp.DataWidthSrc;
    uint32_t IsNotDivisible = recv_size_in_byte % DataWidthInByteSrc;
    uint32_t BlockSize = recv_size_in_byte / DataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

    /* To check if send_size_in_byte is integer multiples of DataWidthInByte */
    if (IsNotDivisible)
    {
        SYS_TEST("Error, data to receive is not integer multiples of DataWidthInByte!\n");
        return;
    }

    /* To check if block size is not 0 */
    if (!BlockSize)
    {
        SYS_TEST("Error, block size is 0!\n");
        return;
    }

    SYS_TEST("Start to receive data by DMA, wait for data...\n");

    /* Start DMA Rx channel */
    DMAC_StartChannel(DMA, RxChNum, (void*)&(TGT_UART->RBR_THR_DLL), RcvDataBuf, BlockSize);

    // Wait for DMA transfer done
    while(!gTfrFlag);
    gTfrFlag = false;

    /* Release DMA Rx channel */
    DMAC_ReleaseChannel(DMA, RxChNum);
    /* Disable DMA */
    DMAC_DeInit(DMA);

    // Print out all received data
    SYS_TEST("Data received done (channel=%d, data_len=%d):\n", RxChNum, recv_size_in_byte);
    SYS_TEST("DMA Interrupt Trigger Count: tfr=%d, blk=%d, srctfr=%d, dsttfr=%d, err=%d\n",
        idx_tfr, idx_blk, idx_srctfr, idx_dsttfr, idx_err);
    // Clear flags
    idx_tfr = idx_blk = idx_srctfr = idx_dsttfr = idx_err = 0;

    for (size_t i = 0; i < recv_size_in_byte; i++)
    {
        if (i % 8 == 0)
            SYS_TEST("\n");
        SYS_TEST(" 0x%02x", RcvDataBuf[i]);
    }
    SYS_TEST("\n");
}

static void MultiDataSendDma(uint32_t send_size_in_byte)
{
    uint32_t TxChNum = 0xFF;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};   //Init config struct and set items as all 0s
    static uint8_t SendDataBuf[512];

    memcpy(SendDataBuf, Const8BitData, sizeof(Const8BitData));

    // Initialize the DMA controller
    DMAC_Init(DMA);
    NVIC_EnableIRQ(DMA_IRQn);

    /* Set dma channel control & config register */
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    // Destination config (Peripheral)
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_8;  //According to peripheral FIFO threshold
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_8; //According to peripheral FIFO width
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = TGT_UART_DMA_DST; //Config to corresponding peripheral
    // Source config (Memory)
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Default;

    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.IntEnable       = ENABLE;

    /* Get free DMA channel */
    TxChNum = DMAC_AcquireChannel(DMA);
    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_TFR);
#if 0
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_SRCTFR);   //SRCTFR INT is meaningless if source is memory
#endif
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_DSTTFR);
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_BLK);
    DMAC_ClrIntFlagMsk(DMA, TxChNum, DMAC_FLAG_INDEX_ERR);
    DMAC_SetChannelConfig(DMA, TxChNum, &TxConfigTmp);

    /* Condition check */
    uint32_t DataWidthInByteSrc = 1 << TxConfigTmp.DataWidthSrc;
    uint32_t IsNotDivisible = send_size_in_byte % DataWidthInByteSrc;
    uint32_t BlockSize = send_size_in_byte / DataWidthInByteSrc;    //BlockSize = DataLen / DataWidthInByteSrc

    /* To check if send_size_in_byte is integer multiples of DataWidthInByte */
    if (IsNotDivisible)
    {
        SYS_TEST("Error, data to send is not integer multiples of DataWidthInByte!\n");
        return;
    }

    /* To check if block size is not 0 */
    if (!BlockSize)
    {
        SYS_TEST("Error, block size is 0!\n");
        return;
    }

    SYS_TEST("Start to send data by DMA...\n");

    /* Start DMA Tx channel */
    DMAC_StartChannel(DMA, TxChNum, SendDataBuf, (void*)&(TGT_UART->RBR_THR_DLL), BlockSize);

    // Wait for DMA transfer done
    while(!gTfrFlag);
    gTfrFlag = false;

    /* Release DMA Rx channel */
    DMAC_ReleaseChannel(DMA, TxChNum);
    /* Disable DMA */
    DMAC_DeInit(DMA);

    SYS_TEST("Send data done (channel=%d, data_len=%d).\n", TxChNum, send_size_in_byte);
    SYS_TEST("DMA Interrupt Trigger Count: tfr=%d, blk=%d, srctfr=%d, dsttfr=%d, err=%d\n",
        idx_tfr, idx_blk, idx_srctfr, idx_dsttfr, idx_err);
    // Clear debugging flags
    idx_tfr = idx_blk = idx_srctfr = idx_dsttfr = idx_err = 0;
}

T_UART_TEST_RESULT UART_TransferWithDmaTestCase7(void)
{
    char c;
    UART_InitTypeDef Init_Struct;
    Init_Struct.UART_LineCtrl = Uart_Line_8n1;
    Init_Struct.UART_BaudRate = 115200;
    UART_Init(TGT_UART, &Init_Struct);
    UART_EnableFifo(TGT_UART);

    while(1)
    {
        UART_PrintInfoCase7();
        switch(c = getchar())
        {
        /* A. Burst transaction receiving end (uart2mem). */
        case 'A':
        case 'a':
            UART_SetRxTrigger(TGT_UART, UART_RX_FIFO_HALF_FULL);
            MultiDataReceiveDma(128);
            break;
        /* B. Burst transaction transmitting end (mem2uart). */
        case 'B':
        case 'b':
            UART_SetTxTrigger(TGT_UART, UART_TX_FIFO_HALF_FULL);
            UART_EnablePtime(TGT_UART); //Enable Programmable THRE Interrupt Mode
            MultiDataSendDma(128);
            break;
        /* C. Single transaction receiving end (uart2mem). */
        case 'C':
        case 'c':
            UART_SetRxTrigger(TGT_UART, UART_RX_FIFO_HALF_FULL);
            MultiDataReceiveDma(4); //Receive 4 bytes (< burst len 8 bytes)
            break;
        /* D. Single transaction transmitting end (mem2uart). */
        case 'D':
        case 'd':
            UART_SetTxTrigger(TGT_UART, UART_TX_FIFO_HALF_FULL);
            UART_EnablePtime(TGT_UART); //Enable Programmable THRE Interrupt Mode
            MultiDataSendDma(4);    //Send 1 word (< burst len 4 word)
            break;
        /* E. Both transactions receiving end (uart2mem). */
        case 'E':
        case 'e':
            UART_SetRxTrigger(TGT_UART, UART_RX_FIFO_HALF_FULL);
            MultiDataReceiveDma(132);
            break;
        /* F. Both transactions transmitting end (mem2uart). */
        case 'F':
        case 'f':
            UART_SetTxTrigger(TGT_UART, UART_TX_FIFO_HALF_FULL);
            UART_EnablePtime(TGT_UART); //Enable Programmable THRE Interrupt Mode
            MultiDataSendDma(132);
            break;
        /* G. DMA with UART Auto Flow Control receiving end (uart2mem). */
        case 'G':
        case 'g':
            UART_ResetRxFifo(TGT_UART); //Reset Rx fifo BEFORE (rather than AFTER) enable AFC
            UART_EnableAfc(TGT_UART);   //Enable Auto Flow Control
            UART_EnableRts(TGT_UART);   //Enable Auto Flow Control
            UART_SetRxTrigger(TGT_UART, UART_RX_FIFO_HALF_FULL);
            MultiDataReceiveDma(128);
            UART_DisableRts(TGT_UART);  //Disable Auto Flow Control
            UART_DisableAfc(TGT_UART);  //Disable Auto Flow Control
            break;
        /* H. DMA with UART Auto Flow Control transmitting end (mem2uart). */
        case 'H':
        case 'h':
            UART_ResetTxFifo(TGT_UART); //Reset Tx fifo BEFORE (rather than AFTER) enable AFC
            UART_EnableAfc(TGT_UART);   //Enable Auto Flow Control
            UART_SetTxTrigger(TGT_UART, UART_TX_FIFO_HALF_FULL);
            UART_EnablePtime(TGT_UART); //Enable Programmable THRE Interrupt Mode
            MultiDataSendDma(128);
            UART_DisableAfc(TGT_UART);  //Disable Auto Flow Control
            break;
        case 0x1B:  //Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return UART_TST_OK;
}
