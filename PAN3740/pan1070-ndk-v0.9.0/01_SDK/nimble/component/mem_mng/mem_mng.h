/**
 *******************************************************************************
 * @file     mem_mng.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c)  Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef MEM_MNG_H_
#define MEM_MNG_H_

#include <stdint.h>

void *mem_alloc(uint32_t size);
void mem_free(void *p);

#endif /* MEM_MNG_H_ */
