/**
 *******************************************************************************
 * @file     app_log.c
 * @create   2024-11-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app_log.h"

#if PAN_LOG_ENABLE && APP_LOG_EN
void app_log_print_data(uint8_t *pdata, uint32_t len)
{
	for(uint32_t i=0; i<len; i++){
		if( i != 0 && (i%16) == 0){
			APP_LOG_PRINT("\r\n");
		}
		APP_LOG_PRINT("%02X ", pdata[i]);
	}
	APP_LOG_PRINT("\r\n");
}
#endif

