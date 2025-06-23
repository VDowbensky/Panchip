/**
 *******************************************************************************
 * @file     hrs.h
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef HRS_H_
#define HRS_H_

#include <stdint.h>

/* Heart-rate configuration */
#define BLE_SVC_HRS_UUID16                 	          0x180D
#define BLE_SVC_HRS_CHR_UUID16_MEASUREMENT            0x2A37
#define BLE_SVC_HRS_CHR_UUID16_BODY_SENSOR_LOC        0x2A38
#define BLE_SVC_HRS_CHR_UUID16_DEVICE_INFO            0x180A
#define BLE_SVC_HRS_CHR_UUID16_MANUFACTURER_NAME      0x2A29
#define BLE_SVC_HRS_CHR_UUID16_MODEL_NUMBER           0x2A24

extern uint16_t hrs_hrm_handle;

void ble_svc_hrs_init(void);
int ble_svc_hrs_notify(uint16_t conn_handle, uint8_t *pdata, uint32_t len);

#endif /* HRS_H_ */
