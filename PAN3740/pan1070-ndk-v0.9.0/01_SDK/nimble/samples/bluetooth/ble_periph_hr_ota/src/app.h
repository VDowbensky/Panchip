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
#include "app_ble_svr.h"

void app_ble_init(void);

void app_ble_adv_start(void);


/******************************************************************************* 
 * SMP OTA API
 ******************************************************************************/
extern void img_mgmt_module_init(void);
extern void smp_ble_pkg_init(void);


#endif

