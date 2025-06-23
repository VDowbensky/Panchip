/**
 *******************************************************************************
 * @file     kv_store.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef KV_STORE_API_H
#define KV_STORE_API_H

#include "mtb_kvstore.h"

void kv_init(void);

cy_rslt_t app_kv_write(const char* key, const uint8_t* data, uint32_t size);

cy_rslt_t app_kv_read(const char* key, uint8_t* data, uint32_t* size);

cy_rslt_t app_kv_delete(const char* key);

cy_rslt_t app_kv_key_exists(const char *key);

#endif 
