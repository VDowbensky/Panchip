/**
 *******************************************************************************
 * @file     app_ble.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef APP_BLE_H_
#define APP_BLE_H_

#include <stdint.h>

#define TX_LEN  		(30)
#define TX_PACKET_NUM   (100)
#define PREAM_TIME_MS   (1)

struct RxPktMsg {
	uint16_t crc_err_cnt;
	uint16_t rx_timeout_cnt;
	uint16_t rx_cnt;
};

extern struct RxDoneMsg RxDoneParams;
extern uint32_t current_period_time;

void setup(void);
void loop(void);
void trx_gpio_init(void);
void trx_init(void);
void single_rx(void);
void single_tx(void);
void print_rx_info(void);
extern void app_ble_hr_tx_timer_stop(void);
extern void app_ble_hr_tx_timer_start(void);

/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "app_ble_svr.h"

void app_ble_init(void);
void app_ble_task_start(void);


#endif

