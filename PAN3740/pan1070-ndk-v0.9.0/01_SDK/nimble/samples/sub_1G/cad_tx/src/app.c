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

/**@brief tx detect flag. */
volatile uint32_t cad_tx_detect_flag = MAC_EVT_TX_CAD_NONE;

/**@brief tx data length. */
static uint8_t tx_test_len = TX_LEN;

/**@brief tx transfer test data. */
static uint8_t tx_test_buf[TX_LEN] = { 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa };

/**@brief tx transfer interval time. */
uint32_t current_period_time = 500;


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


void trx_param_change(uint32_t period_time)
{
	if (period_time <= PREAM_TIME_MS + 10) {
		period_time = PREAM_TIME_MS + 10;
	}
	current_period_time = period_time;
	APP_LOG_INFO("period = %d\n", current_period_time);
}

void GPIO3_IRQHandlerOverlay(void)
{
	GPIO_ClrAllIntFlag(P3);

	cad_tx_detect_flag = MAC_EVT_TX_CAD_ACTIVE;
	APP_LOG_INFO("cad active\n");
}

void trx_gpio_init(void)
{
	SYS_SET_MFP(P3, 1, GPIO);
	GPIO_SetMode(P3, BIT1, GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P3, BIT1);
	GPIO_EnablePulldownPath(P3, BIT1);
	GPIO_EnableInt(P3, 1, GPIO_INT_RISING);
	GPIO_ClrAllIntFlag(P3);
	__NVIC_ClearPendingIRQ(GPIO3_IRQn);
	NVIC_EnableIRQ(GPIO3_IRQn);
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

	ret = rf_init();
	if (ret != OK) {
		APP_LOG_ERR(" 3029 radio init fail\n");
		return;
	}
	APP_LOG_INFO("3029 radio init ok\n");
	rf_set_default_para();
	rf_set_bw(BW_500K);
	rf_set_sf(SF_5);

	rf_set_preamble_time(PREAM_TIME_MS);
	APP_LOG_INFO("pream:%d\n", rf_get_preamble());
	rf_set_transmit_flag(RADIO_FLAG_IDLE);
	rf_enter_continous_tx();
}

uint32_t rf_listen_before_talk_start(void)
{
	uint32_t bw, sf;
	uint32_t one_chirp_time;

	bw = rf_get_bw();
	sf = rf_get_sf();
	one_chirp_time = rf_get_chirp_time(bw, sf); // us

	RF_ASSERT(rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3));
	RF_ASSERT(rf_write_spec_page_reg(PAGE1_SEL, 0x35, 0xf6));

	RF_ASSERT(rf_enter_continous_rx());
	rf_delay_us(one_chirp_time * 1.5 + 360);
	return OK;
}


void single_tx(void)
{
	static uint8_t cnt_value = 0;

	rf_listen_before_talk_start();
	if (cad_tx_detect_flag == MAC_EVT_TX_CAD_ACTIVE) {
		APP_LOG_INFO("cad active, tx cancel, try again later\n");
		cad_tx_detect_flag = MAC_EVT_TX_CAD_NONE;
		rf_set_mode(RF_MODE_STB3);
		rf_refresh();
		rf_set_transmit_flag(RADIO_FLAG_TXDONE);
		return;
	} else if (cad_tx_detect_flag == MAC_EVT_TX_CAD_TIMEOUT) {
		P23 = 1;
		cad_tx_detect_flag = MAC_EVT_TX_CAD_NONE;
		rf_enter_continous_tx();
		if (rf_continous_tx_send_data(tx_test_buf, tx_test_len) != OK) {
			APP_LOG_ERR("tx fail\n");
		} else {
			APP_LOG_INFO("Tx cnt %d\n", cnt_value++);
			if (cnt_value >= 0xff) {
				cnt_value = 0;
			}
			tx_test_buf[0] = cnt_value;
		}
	}

	while (rf_get_transmit_flag() == RADIO_FLAG_IDLE) {
		rf_irq_process();
	}

	if (rf_get_transmit_flag() == RADIO_FLAG_TXDONE) {
		P23 = 0;
		rf_set_transmit_flag(RADIO_FLAG_IDLE);
		rf_set_mode(RF_MODE_STB3);
	}
}
