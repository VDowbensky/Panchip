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
void app_ble_init(void);
void app_ble_task_start(void);

void app_ble_user_handler(void);

#endif
