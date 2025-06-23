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

#define TX_LEN  		(10)
#define MAC_EVT_TX_CAD_NONE     0x00
#define MAC_EVT_TX_CAD_TIMEOUT  0x01
#define MAC_EVT_TX_CAD_ACTIVE   0x02
#define PREAM_TIME_MS                   100

struct RxPktMsg {
	uint16_t crc_err_cnt;
	uint16_t rx_timeout_cnt;
	uint16_t rx_cnt;
};

extern uint32_t current_period_time;
extern volatile uint32_t cad_tx_detect_flag;

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

