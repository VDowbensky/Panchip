/**
 *******************************************************************************
 * @file     xcritical.c
 * @create   2024-11-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "utils/critical.h"

uint32_t critical_region_enter(void)
{
	uint32_t state = 0;
	ENTER_CRITICAL(state);
	return state;
}

void critical_region_exit(uint32_t state)
{
	EXIT_CRITICAL(state);
}

