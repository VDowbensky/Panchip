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

#include "pan_ble.h"
#include "app_ble_svr.h"
#include "sub_1g_base.h"
#include "sub_1g_port.h"

/**@brief rate message unit. */
struct RxPktMsg rate_rx_msg;

/**@brief tx transfer test data. */
static uint8_t tx_test_buf[TX_LEN] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
				       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, };

/**@brief tx transfer interval time. */
uint32_t current_period_time = 1000;

/**@brief device match. */
static bool device_match_flag = false;

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

void print_rx_info(void)
{
	device_match_flag = false;
	float rate_percent = (float)rate_rx_msg.rx_cnt / TX_PACKET_NUM * 100;
	APP_LOG_INFO("Rx rev: %d, timeout: %d, crc error: %d, pkt reception rate: %f\n",
	       rate_rx_msg.rx_cnt,
	       rate_rx_msg.rx_timeout_cnt,
	       rate_rx_msg.crc_err_cnt,
	       rate_percent);
	memset(&rate_rx_msg, 0, sizeof(rate_rx_msg));
}


void trx_param_change(uint32_t plhd_val)
{
	APP_LOG_INFO("Maybe we can do something here\n");
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
	rf_set_preamble_time(PREAM_TIME_MS);
	#ifdef RX_DEVICE
	memset(&rate_rx_msg, 0, sizeof(rate_rx_msg));
	rf_enter_continous_rx();
	#endif
	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	rf_set_recv_flag(RADIO_FLAG_IDLE);
}

void single_rx(void)
{
	if (rf_get_recv_flag() == RADIO_FLAG_RXDONE) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		if (!device_match_flag) {
			device_match_flag = true;
			app_ble_hr_tx_timer_start();
		}
		rate_rx_msg.rx_cnt++;
	}
	if (rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		rate_rx_msg.rx_timeout_cnt++;
	}
	if (rf_get_recv_flag() == RADIO_FLAG_RXERR) {
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		rate_rx_msg.crc_err_cnt++;
	}
}

void single_tx(void)
{
	uint32_t tx_count = TX_PACKET_NUM;

	rf_enter_continous_tx();
	while (tx_count--) {
		P23 = 1;
		if (rf_continous_tx_send_data(tx_test_buf, 10) != OK) {
			APP_LOG_ERR("tx fail \r\n");
		}

		while (rf_get_transmit_flag() != RADIO_FLAG_TXDONE) {
		}

		rf_set_transmit_flag(RADIO_FLAG_IDLE);
		P23 = 0;
	}
	APP_LOG_INFO("tx finish, %d packets sent done\n", TX_PACKET_NUM);
}
