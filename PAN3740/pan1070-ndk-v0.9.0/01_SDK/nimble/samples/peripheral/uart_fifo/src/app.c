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

static uint8_t recvDataBuf[UART_RECV_BUF_SIZE];
static uint8_t sendDataBuf[UART_SEND_BUF_SIZE];

static void app_uart_int_callback(HAL_UART_HandleTypeDef* pUart, HAL_UART_EventOpt event, uint8_t *pBuf, uint16_t xferCount)
{
    // Handle Tx Finish Event
    if (event == HAL_UART_EVT_TX_FINISH) {
        APP_LOG_INFO("UART Tx done, sent cnt: %d\n", xferCount);

        // Start UART Continuous Rx with Interrupt
        HAL_Status sts = HAL_UART_ReceiveDataContinuously_INT(&UART1_OBJ, recvDataBuf, UART_RECV_BUF_SIZE);
        if (sts != HAL_OK) {
            APP_LOG_ERR("Error: HAL Uart Receive Data with DMA Fail, error code = %d!\n", sts);
        }
    }

    // Handle Rx Finish Event
    if (event == HAL_UART_EVT_RX_FINISH) {
        APP_LOG_INFO("UART Rx done, rcvd cnt: %d\n", xferCount);
    }

    // Handle Continuous Rx Timeout Event
    if (event == HAL_UART_EVT_CONTI_RX_TIMEOUT) {
        APP_LOG_INFO("UART Continuous Rx Timeout, rcvd cnt: %d\n", xferCount);

        // Copy data from UART Rx buffer to Tx buffer
        memcpy(sendDataBuf, pBuf, xferCount);

        // Start UART Tx with Interrupt
        HAL_Status sts = HAL_UART_SendData_INT(&UART1_OBJ, sendDataBuf, xferCount);
        if (sts != HAL_OK) {
            APP_LOG_ERR("Error: HAL Uart Send Data with Interrupt Fail, error code = %d!\n", sts);
        }
    }

    // Handle Continuous Rx Buffer Full Event
    if (event == HAL_UART_EVT_CONTI_RX_BUFF_FULL) {
        // Abort Receiving from UART Rx Line
        HAL_UART_ReceiveDataAbort_INT(pUart);

        APP_LOG_WRN("Error: UART Continuous Rx buffer full!\n");
    }

    // Handle Rx Error Events
    if ((event == HAL_UART_EVT_RX_ERR_PARITY) || (event == HAL_UART_EVT_RX_ERR_FRAME) || (event == HAL_UART_EVT_RX_ERR_OVERRUN)) {
        // Abort Receiving from UART Rx Line
        HAL_UART_ReceiveDataAbort_INT(pUart);

        // Log prints
        if (event == HAL_UART_EVT_RX_ERR_PARITY) {
            APP_LOG_WRN("Error: UART Rx Parity Error!\n");
        }
        if (event == HAL_UART_EVT_RX_ERR_FRAME) {
            APP_LOG_WRN("Error: UART Rx Frame Error!\n");
        }
        if (event == HAL_UART_EVT_RX_ERR_OVERRUN) {
            APP_LOG_WRN("Error: UART Rx FIFO Overrun Error!\n");
        }
        APP_LOG_ERR("Actual received data size: %d\n", xferCount);
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

    UART1_OBJ.interruptObj.txTrigLevel = HAL_UART_TX_FIFO_HALF_FULL;    // Default config
    UART1_OBJ.interruptObj.rxTrigLevel = HAL_UART_RX_FIFO_HALF_FULL;    // Default config
    UART1_OBJ.interruptObj.IrqPriority = 2;                             // Default config
    UART1_OBJ.interruptObj.callbackFunc = app_uart_int_callback;
    if (HAL_UART_Init_INT(&UART1_OBJ) != HAL_OK) {
        APP_LOG_ERR("Error: HAL Uart Interrupt Init Fail!\n");
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

    /* Init peripheral module */
    uart1_init();

    // Start UART Continuous Rx with Interrupt
    HAL_Status sts = HAL_UART_ReceiveDataContinuously_INT(&UART1_OBJ, recvDataBuf, UART_RECV_BUF_SIZE);
    if (sts != HAL_OK) {
        APP_LOG_ERR("Error: HAL Uart Receive Data Continuously with Interrupt Fail, error code = %d!\n", sts);
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
