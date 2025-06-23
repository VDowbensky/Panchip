/**
 *******************************************************************************
 * @file     os_wrapper.h
 * @create   2024-11-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef OS_WRAPPER
#define OS_WRAPPER
 
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

/**@brief OS max delay */
#define OS_MAX_DELAY   portMAX_DELAY

/**@brief  convert ms to OS ticks */
#define OS_MS_TO_TICK(ms)       pdMS_TO_TICKS(ms)

/**@brief  convert OS ticks to ms */
#define OS_TICK_TO_MS(t)        ((t) * 1000/configTICK_RATE_HZ)


void OS_DelayMs(uint32_t ms);
 
 
#endif 
