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


void setup(void);
void loop(void);


/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "nimble/ble.h"
#include "modlog/modlog.h"

void app_ble_init(void);
void app_ble_task_start(void);


#endif

