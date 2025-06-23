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

static uint32_t one_chirp_time;

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

void trx_param_change(uint32_t plhd_val)
{
	APP_LOG_INFO("Maybe we can do something here\n");
}

void GPIO3_IRQHandlerOverlay(void)
{
	GPIO_ClrAllIntFlag(P3);
	cad_tx_detect_flag = MAC_EVT_TX_CAD_ACTIVE;
	single_rx();
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
	uint32_t bw, sf;

	trx_spi_init();

	ret = rf_init();
	if (ret != OK) {
		APP_LOG_ERR(" RF Init Fail\n");
		return;
	}
	APP_LOG_INFO("3029 radio init ok\n");
	rf_set_default_para();
	rf_set_bw(BW_500K);
	rf_set_sf(SF_5);

	sf = rf_get_sf();
	bw = rf_get_bw();
	one_chirp_time = rf_get_chirp_time(bw, sf);

	rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3);
	rf_enter_continous_rx();
}

void single_rx(void)
{
	uint32_t i, rx_max_times = 0;

	if (check_cad_rx_inactive(one_chirp_time) == LEVEL_ACTIVE) {
		APP_LOG_INFO("cad\n");
		rx_max_times = 0;

		while (rf_get_recv_flag() != RADIO_FLAG_RXDONE) {
			rf_irq_process();
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

		rf_delay_us(10);
		rx_max_times++;
		if (rx_max_times < one_chirp_time / 10 * 2) { /*2-chirp-cad-check*/
			if (CHECK_CAD() != 1) {
				APP_LOG_ERR("break cad err\n");
				return;
			}
		}
		if (rx_max_times >= (PREAM_TIME_MS * 100 + one_chirp_time * 30)) { /*chirp-timeout*/
			APP_LOG_WRN("break cad timeout\n");
			return;
		}
	} else {
		APP_LOG_INFO("sleep\n");
		rf_deepsleep();
		rf_delay_us(one_chirp_time);
		rf_init();
		rf_set_default_para();
		rf_set_bw(BW_500K);
		rf_set_sf(SF_5);
		rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3);
		rf_enter_continous_rx();
	}
}
