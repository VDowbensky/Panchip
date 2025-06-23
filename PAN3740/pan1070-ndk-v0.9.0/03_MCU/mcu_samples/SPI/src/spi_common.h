/**************************************************************************//**
 * @file     spi_common.h
 * @version  V1.0
 * $Date:    19/10/22 14:34 $
 * @brief    Common header file for SPI test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __SPI_COMMON_H__
#define __SPI_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CHAR_NONE       (-1)

typedef enum _T_SPI_TST_RESULT
{
    SPI_TST_OK,
} T_SPI_TEST_RESULT;

// Config SPI software buffer size
#define SPI_RX_BUF_SIZE         256
#define SPI_TX_BUF_SIZE         256

// Config UART for Debug/Log
#define DEBUG_PORT      UART0   //Note this define should be same with that in retarget.c

// Config Target SPI for Test
#define TGT_SPI_ID      TEST_SPI0


#define TEST_SPI0       0
#define TEST_SPI1       1

#if TGT_SPI_ID == TEST_SPI0
#define TGT_SPI             SPI0
#define TGT_SPI_IRQn        SPI0_IRQn
#define TGT_SPI_DMA_SRC     DMAC_Peripheral_SPI0_Rx
#define TGT_SPI_DMA_DST     DMAC_Peripheral_SPI0_Tx

#define AUX_SPI_ID          TEST_SPI1
#define AUX_SPI             SPI1
#define AUX_SPI_IRQn        SPI1_IRQn
#define AUX_SPI_DMA_SRC     DMAC_Peripheral_SPI1_Rx
#define AUX_SPI_DMA_DST     DMAC_Peripheral_SPI1_Tx

#elif TGT_SPI_ID == TEST_SPI1
#define TGT_SPI             SPI1
#define TGT_SPI_IRQn        SPI1_IRQn
#define TGT_SPI_DMA_SRC     DMAC_Peripheral_SPI1_Rx
#define TGT_SPI_DMA_DST     DMAC_Peripheral_SPI1_Tx

#define AUX_SPI_ID          TEST_SPI0
#define AUX_SPI             SPI0
#define AUX_SPI_IRQn        SPI0_IRQn
#define AUX_SPI_DMA_SRC     DMAC_Peripheral_SPI0_Rx
#define AUX_SPI_DMA_DST     DMAC_Peripheral_SPI0_Tx
#else
#error "Error, invalid target SPI ID, please check test config."
#endif


// Test Case Functions
extern T_SPI_TEST_RESULT SPI_RegisterDefaultValueCheckCase0(void);
extern T_SPI_TEST_RESULT SPI_DataSizeSelectTestCase1(void);
extern T_SPI_TEST_RESULT SPI_BaudrateTestCase2(void);
extern T_SPI_TEST_RESULT SPI_FrameFormatTestCase3(void);
extern T_SPI_TEST_RESULT SPI_InterruptTestCase4(void);
extern T_SPI_TEST_RESULT SPI_TransferWithDmaTestCase5(void);
extern T_SPI_TEST_RESULT SPI_SimpleTransmissionDemoCase6(void);
extern T_SPI_TEST_RESULT SPI_DataLsbSendRcvTestCase7(void);
extern T_SPI_TEST_RESULT SPI_ThreeWireDmaTransferTestCase8(void);
extern T_SPI_TEST_RESULT SPI_ThreeWireSimpleTransmissionCase9(void);

extern void SPI_TestFunctionEnter(uint16_t TcIdx);
extern void SPI_TestModuleInit(void);

extern size_t SPI_RecvMultiDataAsync(SPI_T* SPIx, uint16_t* data, size_t expect_recv_size);
extern size_t SPI_SendMultiDataAsync(SPI_T* SPIx, const uint16_t* data, size_t expect_send_size);
extern void SPI_MultiDataSendReceiveAsync(SPI_T* SPIx, const uint16_t* data_to_send, size_t expect_send_size, uint16_t* buf_to_recv, size_t expect_recv_size);
extern void SPI_RecvMultiDataSync(SPI_T* SPIx, uint16_t* data, size_t expect_recv_size);
extern void SPI_SendMultiDataSync(SPI_T* SPIx, const uint16_t* data, size_t expect_send_size);
extern void SPI_Set3WireConfig(SPI_T* SPIx, 
						SPI_WriteReadSelDef wr,
						SPI_CmdDatDef dc);

extern uint16_t spiRxBuffer[SPI_RX_BUF_SIZE];
extern uint16_t spiTxBuffer[SPI_TX_BUF_SIZE];

extern volatile uint32_t spiRxBufIdx;
extern uint32_t spiTxBufIdx;

#ifdef __cplusplus
}
#endif

#endif //__SPI_COMMON_H__
