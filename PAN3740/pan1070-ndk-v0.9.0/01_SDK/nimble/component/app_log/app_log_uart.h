/**
 *******************************************************************************
 * @file     log_uart.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef _APP_LOG_UART_H_
#define _APP_LOG_UART_H_

#include <stdint.h>
#include "sdk_config.h"

#if PAN_LOG_ENABLE && CONFIG_UART_LOG_ENABLE
void debug_uart_init(void);
#endif

#endif // _APP_LOG_UART_H_
