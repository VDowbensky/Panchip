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

#define TX_LEN  (10)

extern struct RxDoneMsg RxDoneParams;
extern uint32_t current_period_time;

void setup(void);
void loop(void);
void trx_gpio_init(void);
void trx_init(void);
void single_rx(void);
void single_tx(void);

/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "app_ble_svr.h"

void app_ble_init(void);
void app_ble_task_start(void);


#endif

