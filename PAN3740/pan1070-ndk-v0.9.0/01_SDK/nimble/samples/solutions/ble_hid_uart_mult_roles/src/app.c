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

#include "uart_AT.h"

#if CONFIG_PM
volatile uint8_t userSleepEnable = 1;

void GPIO0_IRQHandlerOverlay(void)
{
	if (GPIO_GetIntFlag(P0, BIT(6))) {
		GPIO_ClrIntFlag(P0, BIT(6));

		if(P06 == 0){
			userSleepEnable = 0;
			//APP_LOG_INFO("user busy\r\n");
		}
		else{
			userSleepEnable = 1;
			//APP_LOG_INFO("user idle\r\n");
		}
	}
}

void app_gpio_wakeup_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);

	SYS_SET_MFP(P0, 6, GPIO);

	GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
	GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);
	GPIO_EnableDebounce(P0, BIT6);

	GPIO_EnablePullupPath(P0, BIT6);
	GPIO_EnableInt(P0, 6, GPIO_INT_BOTH_EDGE);

	NVIC_EnableIRQ(GPIO0_IRQn);
}

CONFIG_RAM_CODE uint8_t user_is_sleep_allow(void)
{
	return userSleepEnable;
}
#endif

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void setup(void)
{
	APP_LOG_INFO("app started\n");

	/* UART AT initialization. */
	uart_at_init();

	/* ble stack initialization. */
	app_ble_init();

	/**
	 * TODO: user add application initialization code in here.
	 */
#if CONFIG_PM
	/* GPIO wakeup initialization. */
	app_gpio_wakeup_init();
#endif
}

/**
 *******************************************************************************
 * @brief this is main thread entry if CONFIG_OS_EN=1;
 *        this is main loop entry if CONFIG_OS_EN=0
 *******************************************************************************
 */
void loop(void)
{
	uart_at_handler();
}
