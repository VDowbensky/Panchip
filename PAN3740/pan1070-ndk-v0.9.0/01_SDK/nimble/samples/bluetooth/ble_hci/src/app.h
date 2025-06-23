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
#include "utility.h"
#include "app_log.h"

void setup(void);
void loop(void);


/******************************************************************************* 
 * BLE API
 ******************************************************************************/
void app_hci_init(void);


#endif

