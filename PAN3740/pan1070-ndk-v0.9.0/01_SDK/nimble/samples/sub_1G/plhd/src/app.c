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

#include "sub_1g_base.h"
#include "sub_1g_port.h"

/**@brief tx transfer test data. */
static uint8_t tx_test_buf[TX_LEN] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
				       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, };

/**@brief tx transfer interval time. */
uint32_t current_period_time = 1000;

/**@brief payload header length. */
#ifdef RX_DEVICE
static uint8_t payload_header_len = PLHD_LEN8;
#endif

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

void trx_param_change(uint32_t plhd_val)
{
	#ifdef RX_DEVICE
	if (plhd_val == 8) {
		payload_header_len = PLHD_LEN8;
	} else if (plhd_val == 16) {
		payload_header_len = PLHD_LEN16;
	} else {
		payload_header_len = PLHD_LEN8;
	}
	rf_set_plhd_rx_on(5, payload_header_len);
	APP_LOG_INFO("Set payload header len %d\n", plhd_val);
	#endif
}

void GPIO1_IRQHandlerOverlay(void)
{
	GPIO_ClrAllIntFlag(P1);
	rf_irq_process();

	if (rf_get_recv_flag() != RADIO_FLAG_IDLE) {
		single_rx();
	}
}

void trx_gpio_init(void)
{
	SYS_SET_MFP(P1, 7, GPIO);
	GPIO_SetMode(P1, BIT7, GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P1, BIT7);
	GPIO_EnablePulldownPath(P1, BIT7);
	GPIO_EnableInt(P1, 7, GPIO_INT_RISING);
	GPIO_ClrAllIntFlag(P1);
	__NVIC_ClearPendingIRQ(GPIO1_IRQn);
	NVIC_EnableIRQ(GPIO1_IRQn);
}

void trx_spi_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);

	/* 3740AU33BBA */
	SYS_SET_MFP(P1, 4, GPIO);                       // PAD_CSN_3V
	SYS_SET_MFP(P1, 5, GPIO);                       // PAD_SCK_3V
	SYS_SET_MFP(P3, 0, GPIO);                       // PAD_MISO_3V
	SYS_SET_MFP(P1, 6, GPIO);                       // PAD_MOSI_3V

	SYS_SET_MFP(P1, 3, GPIO);                       // reset pin
	SYS_SET_MFP(P1, 7, GPIO);                       // IRQ pin

	GPIO_SetMode(P1, BIT3, GPIO_MODE_OUTPUT);       // reset

	P14 = 1;                                        // cs = 1;
	P13 = 1;
	P15 = 0;                                        // sck = 0;

	GPIO_SetMode(P1, BIT4, GPIO_MODE_OUTPUT);       // cs
	GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT);       // sck
	GPIO_SetMode(P1, BIT6, GPIO_MODE_OUTPUT);       // mosi

	GPIO_SetMode(P3, BIT0, GPIO_MODE_INPUT);        // miso
	GPIO_SetMode(P1, BIT7, GPIO_MODE_INPUT);        // irq

	SYS_SET_MFP(P2, 3, GPIO);                       // test pin
	GPIO_SetMode(P2, BIT3, GPIO_MODE_OUTPUT);       // test
	P23 = 0;
}

void trx_init(void)
{
	uint32_t ret = 0;

	trx_spi_init();

	for (uint8_t i = 0; i < TX_LEN - 0x10; i++) {
		tx_test_buf[0x10 + i] = 0x30 + i;
	}

	ret = rf_init();
	if (ret != OK) {
		APP_LOG_ERR(" 3029 radio init fail\n");
		return;
	}
	APP_LOG_INFO("3029 radio init ok\n");
	rf_set_default_para();
	#ifdef RX_DEVICE
	rf_set_plhd_rx_on(5, payload_header_len);
	rf_enter_continous_rx();
	#endif
	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	rf_set_recv_flag(RADIO_FLAG_IDLE);
}

void single_rx(void)
{
	uint32_t i;

	if (rf_get_recv_flag() == RADIO_FLAG_PLHDRXDONE) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		APP_LOG("###Plhd Rev :  ");
		for (i = 0; i < RxDoneParams.PlhdSize; i++) {
			APP_LOG("0x%02x ", RxDoneParams.PlhdPayload[i]);
		}
		APP_LOG("\n");
	}

	if (rf_get_recv_flag() == RADIO_FLAG_RXDONE) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		APP_LOG("###Rx len  %d: ", RxDoneParams.Size);
		for (i = 0; i < RxDoneParams.Size; i++) {
			APP_LOG("0x%02x ", RxDoneParams.Payload[i]);
		}
		APP_LOG("\n");
		return;
	}
	if ((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR)) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		APP_LOG_ERR("Rxerr\n");
		return;
	}
}

void single_tx(void)
{
	static uint8_t tx_count = 0;
	uint32_t tx_time = 0;

	rf_set_transmit_flag(RADIO_FLAG_IDLE);

	if (rf_single_tx_data(tx_test_buf, TX_LEN, &tx_time) != OK) {
		APP_LOG_ERR("tx fail \r\n");
	}

	while (rf_get_transmit_flag() != RADIO_FLAG_TXDONE) {
		rf_irq_process();
	}

	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	APP_LOG_INFO("Tx cnt %d\r\n", tx_count++);
}
