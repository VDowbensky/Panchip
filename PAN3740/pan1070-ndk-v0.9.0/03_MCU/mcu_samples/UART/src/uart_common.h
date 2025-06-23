/**************************************************************************//**
 * @file     uart_common.h
 * @version  V1.0
 * $Date:    19/09/25 14:00 $
 * @brief    Header file for UART test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __UART_COMMON_H__
#define __UART_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CHAR_NONE       (-1)

typedef enum _T_UART_TST_RESULT
{
    UART_TST_OK,
} T_UART_TEST_RESULT;

// Config UART software buffer size
#define UART_RX_BUF_SIZE        256
#define UART_TX_BUF_SIZE        256

// Config UART for Debug/Log
#define DEBUG_PORT      UART0   //Note this define should be same with that in retarget.c

// Config Target UART for Test
#define TGT_UART_ID     TEST_UART1


#define TEST_UART0      0
#define TEST_UART1      1


#if TGT_UART_ID == TEST_UART0
#define TGT_UART            UART0
#define TGT_UART_IRQn       UART0_IRQn
#define TGT_UART_DMA_SRC    DMAC_Peripheral_UART0_Rx
#define TGT_UART_DMA_DST    DMAC_Peripheral_UART0_Tx
#elif TGT_UART_ID == TEST_UART1
#define TGT_UART            UART1
#define TGT_UART_IRQn       UART1_IRQn
#define TGT_UART_DMA_SRC    DMAC_Peripheral_UART1_Rx
#define TGT_UART_DMA_DST    DMAC_Peripheral_UART1_Tx
#endif


// Test Case Functions
extern T_UART_TEST_RESULT UART_RegisterDefaultValueCheckCase0(void);
extern T_UART_TEST_RESULT UART_DataLenSelectAndStopBitTestCase1(void);
extern T_UART_TEST_RESULT UART_ParityCheckTestCase2(void);
extern T_UART_TEST_RESULT UART_BaudrateTestCase3(void);
extern T_UART_TEST_RESULT UART_FifoAndThresholdTestCase4(void);
extern T_UART_TEST_RESULT UART_FifoAndLineStatusInterruptTestCase5(void);
extern T_UART_TEST_RESULT UART_AutoFlowControlTestCase6(void);
extern T_UART_TEST_RESULT UART_TransferWithDmaTestCase7(void);
extern T_UART_TEST_RESULT UART_9BitModeTestCase8(void);

extern void UART_TestFunctionEnter(uint16_t TcIdx);
extern void UART_TestModuleInit(void);
extern void TGT_ClrAllReadyData(void);
extern char TGT_GetChar(uint32_t* line_status);
extern void TGT_ReceiveMultiData(uint8_t* data, size_t expect_size);
//extern bool TGT_ReceiveMultiData9Bit(uint8_t* data, size_t expect_size, uint8_t recv_addr, volatile bool* addr_rcvd_flag);
extern void TGT_SendMultiData(const uint8_t* data, size_t size);
extern char* Hexlify(const uint8_t* bin_in, size_t n, char* hex_out);
extern char* ConvLsrErrFlagToStr(uint32_t line_status, char* str_buf, size_t buf_size);

extern uint8_t uartRxBuffer[UART_RX_BUF_SIZE];
extern uint8_t uartTxBuffer[UART_TX_BUF_SIZE];
extern volatile uint32_t uartRxBufIdx;
extern uint32_t uartTxBufIdx;
extern volatile bool uartTxDone;
extern volatile bool uartAddrRcvd;
extern volatile bool uartRxIntFlag;

#ifdef __cplusplus
}
#endif

#endif //__UART_COMMON_H__
