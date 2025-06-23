/**
 *******************************************************************************
 * @file     app.c
 * @create   2024-12-10
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "FreeRTOS.h"
#include "soc_api.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "app.h"
#include "app_log.h"


#define UART_RECV_BUF_SIZE      128
#define UART_SEND_BUF_SIZE      128
#define UART_TEST_SIZE          32

uint8_t revDataBuf[UART_RECV_BUF_SIZE];
uint8_t sendDataBuf[UART_SEND_BUF_SIZE];

void app_uart_rx_dma_callback(UART_Cb_Flag_Opt flag, uint8_t *pOutPtr, uint16_t size)
{
    APP_LOG_INFO("UART Rx with DMA Done, recv data size = %d.\n", size);

    // Copy data from UART Rx buffer to Tx buffer
    memcpy(sendDataBuf, pOutPtr, size);

    // Start UART Tx with DMA
    HAL_Status sts = HAL_UART_SendData_DMA(&UART1_OBJ, sendDataBuf, UART_TEST_SIZE);
    if (sts != HAL_OK)
    {
        APP_LOG_ERR("Error: HAL Uart Send Data with DMA Fail, error code = %d!\n", sts);
    }
}

void app_uart_tx_dma_callback(UART_Cb_Flag_Opt flag, uint8_t *pOutPtr, uint16_t size)
{
    APP_LOG_INFO("UART Tx with DMA Done, sent data size = %d.\n", size);

    // Start UART Rx with DMA
    HAL_Status sts = HAL_UART_ReceiveData_DMA(&UART1_OBJ, revDataBuf, UART_TEST_SIZE);
    if (sts != HAL_OK)
    {
        APP_LOG_ERR("Error: HAL Uart Receive Data with DMA Fail, error code = %d!\n", sts);
    }
}

static void uart1_init(void)
{
    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);

    UART1_OBJ.initObj.baudRate = 115200;
    UART1_OBJ.initObj.format = HAL_UART_FMT_8_N_1;
    if (HAL_UART_Init(&UART1_OBJ) != HAL_OK) {
        APP_LOG_ERR("Error: HAL Uart Init Fail!\n");
    }

    HAL_DMA_Init();

    if (HAL_UART_Init_DMA(&UART1_OBJ, HAL_UART_DMA_TX, app_uart_tx_dma_callback) != HAL_OK) {
        APP_LOG_ERR("Error: HAL Uart Tx DMA Init Fail!\n");
    }
    if (HAL_UART_Init_DMA(&UART1_OBJ, HAL_UART_DMA_RX, app_uart_rx_dma_callback) != HAL_OK) {
        APP_LOG_ERR("Error: HAL Uart Rx DMA Init Fail!\n");
    }
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    // Init peripheral module
    uart1_init();

    // Start UART Rx with DMA
    HAL_Status sts = HAL_UART_ReceiveData_DMA(&UART1_OBJ, revDataBuf, UART_TEST_SIZE);
    if (sts != HAL_OK)
    {
        APP_LOG_ERR("Error: HAL Uart Receive Data with DMA Fail, error code = %d!\n", sts);
    }
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    // Do nothing here.
    // We just omit the main task loop and let main() function return
    // back to the main_task() in os_setup.c and destroy itself then.
}
