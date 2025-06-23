/**
 *******************************************************************************
 * @file     ble_dev_filt.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef BLE_DEV_FILT_H_
#define BLE_DEV_FILT_H_

#include <stdint.h>
#include <stdbool.h>
#include "../../../../host/pan_ble.h"

#define BLE_DEV_FILT_MAX_ADDR_NUM    4

/*! State Code*/
enum{
	BDF_OK  		  =  0,
	BDF_INVALID_PARAM = -1,
	BDF_ALLOC_FAILED  = -2,
	BDF_RES_NO_ENOUGH = -3,
};

enum{
	BLE_DEV_FILT_MODE_ALL      = 0, /*!< All conditions are required to match. default */
	BLE_DEV_FILT_MODE_SINGLE   = 1, /*!< only need to match one condition.     */
};

typedef struct{
	uint16_t val;
}uuid16_t;

typedef struct{
	uint8_t val[128];
}uuid128_t;

void ble_dev_filt_init(void);
void ble_dev_filt_reset(void);

void ble_dev_filt_set_match_mode(uint8_t mode);

int ble_dev_filt_set_device_name(char *name,uint32_t len);
int ble_dev_filt_remove_device_name(void);

int ble_dev_filt_set_rssi(int8_t rssi);
int ble_dev_filt_remove_rssi(void);

int ble_dev_filt_set_addr_type(uint8_t addrType);
int ble_dev_filt_remove_addr_type(void);

int ble_dev_filt_add_addr(uint8_t addr[6], uint8_t matchLen);
int ble_dev_filt_remove_addr(uint8_t addr[6]);
int ble_dev_filt_remove_all_addr(void);

int ble_dev_filt_set_uuid16(uint16_t uuid16);
int ble_dev_filt_remove_uuid16(void);

int ble_dev_filt_set_uuid128(uint8_t uuid[16]);
int ble_dev_filt_remove_uuid128(void);

int ble_dev_filt_set_manu_data(uint8_t *pdata, uint32_t len);
int ble_dev_filt_remove_manu_data(void);

bool ble_dev_filt_is_match(const struct ble_gap_disc_desc *desc);

#endif /* BLE_DEVICE_FILTER_H_ */
