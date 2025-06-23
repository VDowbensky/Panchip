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
#include "peer.h"
#include "pan_ble.h"
#include "app_log.h"

void setup(void);
void loop(void);


/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "app_ble_svr.h"

void app_ble_init(void);
void app_ble_task_start(void);


static int app_ble_gap_event(struct ble_gap_event *event, void *arg);


// BLE Central API
void app_ble_central_init(void);
void app_ble_scan_start(void);
void app_ble_central_write(uint8_t *data, uint8_t len);


// BLE Peripheral API
void app_ble_periph_init(void);
void app_ble_adv_start(void);
void app_ble_periph_notify(uint8_t *data, uint8_t len);

#endif

