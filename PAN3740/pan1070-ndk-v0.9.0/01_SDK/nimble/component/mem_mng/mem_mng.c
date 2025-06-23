/**
 *******************************************************************************
 * @file     mem_mng.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c)  Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

#include "mem_mng.h"
#include "utility.h"

void *mem_alloc(uint32_t size)
{
	return pvPortMalloc(size);
}

void mem_free(void *p)
{
	vPortFree(p);
}

