/**
 *******************************************************************************
 * @file     os_wrapper.c
 * @create   2024-11-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "os_wrapper.h"


/**
 * @brief  OS delay ms
 * @param  ms        delay ms
 * @return none.
 */
void OS_DelayMs(uint32_t ms)
{
	vTaskDelay(OS_MS_TO_TICK(ms));
}
