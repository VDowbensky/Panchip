/**
 *******************************************************************************
 * @file     app.h
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
#include "utility.h"

void setup(void);
void loop(void);

/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "app_ble_svr.h"

//BLE base API
void app_ble_init(void);
void app_ble_task_start(void);

//BLE Central API
void app_ble_central_init(void);
void app_ble_scan_start(void);

//BLE Periph API
void app_ble_periph_init(void);
void app_ble_adv_start(void);

#endif
