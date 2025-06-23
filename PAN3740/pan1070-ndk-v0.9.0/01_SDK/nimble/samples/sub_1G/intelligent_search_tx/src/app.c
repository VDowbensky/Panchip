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

/**@brief tx transfer test data. */
static uint8_t tx_test_buf[TX_LEN] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
				       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, };

/**@brief search frequecy range. */
static int sf_range[] = { SF_5, SF_6, SF_7, SF_8, SF_9 };

/**@brief search frequecy range. */
static int chirp_counts[5];

/**@brief tx transfer interval time. */
uint32_t current_period_time = 1000;

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

void trx_param_change(uint32_t sf_val)
{
	uint8_t size = sizeof(sf_range) / sizeof(sf_range[0]);

	if ((sf_val < 5) || (sf_val > 9)) {
		APP_LOG_INFO("Set the sf value between 5 and 9 \n");
		return;
	}

	rf_set_sf(sf_val);
	APP_LOG_INFO("Set the sf value %d\n", sf_val);
	rf_set_auto_sf_tx_preamble(DEFAULT_SF, sf_range, size, chirp_counts);
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
	uint8_t size = sizeof(sf_range) / sizeof(sf_range[0]);

	trx_spi_init();

	ret = rf_init();
	if (ret != OK) {
		APP_LOG_ERR(" 3029 radio init fail\n");
		return;
	}
	APP_LOG_INFO("3029 radio init ok\n");
	rf_set_default_para();
	rf_set_bw(BW_500K);
	rf_set_sf(SF_9);

	calculate_chirp_count(sf_range, size, chirp_counts);
	rf_set_auto_sf_tx_preamble(DEFAULT_SF, sf_range, size, chirp_counts);
}


void single_tx(void)
{
	static uint8_t cnt_value = 0;

	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	rf_enter_continous_tx();
	if (rf_continous_tx_send_data(tx_test_buf, 10) != OK) {
		APP_LOG_ERR("tx fail \r\n");
	}

	while (rf_get_transmit_flag() != RADIO_FLAG_TXDONE) {
		rf_irq_process();
	}

	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	APP_LOG_INFO("Tx cnt %d\n", cnt_value++);
	if (cnt_value >= 0xff) {
		cnt_value = 0;
	}
	tx_test_buf[0] = cnt_value;
}
