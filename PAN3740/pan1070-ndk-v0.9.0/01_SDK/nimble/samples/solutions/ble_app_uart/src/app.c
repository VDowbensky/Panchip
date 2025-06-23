/**
 *******************************************************************************
 * @file     app.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app.h"
#include "app_log.h"



uint8_t revDataBuf[UART_RECV_BUF_SIZE];
uint16_t rx_data_size;

static void uart1_init(void);

void uart_data_send(HAL_UART_HandleTypeDef *pUart, uint8_t *Buf, size_t Size)
{
	// Start UART Tx with Interrupt
	HAL_Status sts = HAL_UART_SendData_INT(pUart, Buf, Size);
	if (sts != HAL_OK) {
		APP_LOG_ERR("Error: HAL Uart Send Data with Interrupt Fail, error code = %d!\n", sts);
	}
}

static void app_uart_int_callback(HAL_UART_HandleTypeDef* pUart, HAL_UART_EventOpt event, uint8_t *pBuf, uint16_t xferCount)
{
    // Handle Tx Finish Event
    if (event == HAL_UART_EVT_TX_FINISH) {
        APP_LOG_INFO("UART Tx done, sent cnt: %d\n", xferCount);
    }

    // Handle Rx Finish Event
    if (event == HAL_UART_EVT_RX_FINISH) {
        APP_LOG_INFO("UART Rx done, rcvd cnt: %d\n", xferCount);
    }

    // Handle Continuous Rx Timeout Event
    if (event == HAL_UART_EVT_CONTI_RX_TIMEOUT) {
        APP_LOG_INFO("UART Continuous Rx Timeout, rcvd cnt: %d\n", xferCount);

        extern void ble_app_user_evt_post(void);

		rx_data_size = xferCount;
		ble_app_user_evt_post();

		// Start UART Continuous Rx with Interrupt
		HAL_Status sts = HAL_UART_ReceiveDataContinuously_INT(&UART1_OBJ, revDataBuf, UART_RECV_BUF_SIZE);
		if (sts != HAL_OK) {
			APP_LOG_ERR("Error: HAL Uart Receive Data Continuously with Interrupt Fail, error code = %d!\n", sts);
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
	#ifdef IP_101x
	SYS_SET_MFP(P0, 1, UART1_TX);
    SYS_SET_MFP(P0, 0, UART1_RX);
    GPIO_EnableDigitalPath(P0, BIT1);
	#else
	SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);
	#endif

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

	// Start UART Continuous Rx with Interrupt
    HAL_Status sts = HAL_UART_ReceiveDataContinuously_INT(&UART1_OBJ, revDataBuf, UART_RECV_BUF_SIZE);
    if (sts != HAL_OK) {
        APP_LOG_ERR("Error: HAL Uart Receive Data Continuously with Interrupt Fail, error code = %d!\n", sts);
    }
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void setup(void)
{
	APP_LOG_INFO("app started\n");

	/* ble stack initialization. */
	app_ble_init();

	/**
	 * TODO: user add application initialization code in here.
	 */
	uart1_init();
}

/**
 *******************************************************************************
 * @brief this is main thread entry if CONFIG_OS_EN=1;
 *        this is main loop entry if CONFIG_OS_EN=0
 *******************************************************************************
 */
void loop(void)
{

}
