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

#define UART_RECV_BUF_SIZE		512

extern uint8_t revDataBuf[UART_RECV_BUF_SIZE];
extern uint16_t rx_data_size;

void setup(void);
void loop(void);


/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "app_ble_svr.h"
#include "nimble/ble.h"
#include "modlog/modlog.h"

void app_ble_init(void);
void app_ble_task_start(void);



#endif

