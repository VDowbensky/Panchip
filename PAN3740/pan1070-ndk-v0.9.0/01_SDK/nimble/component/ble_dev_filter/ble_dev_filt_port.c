/**
 *******************************************************************************
 * @file     ble_dev_filt_port.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "ble_dev_filt.h"
#include "FreeRTOS.h"

void *ble_dev_filt_alloc(uint32_t size)
{
	return pvPortMalloc(size);
}

void ble_dev_filt_free(void *p)
{
	vPortFree(p);
}

