/**
 *******************************************************************************
 * @file     gw.h
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef GW_H_
#define GW_H_

#include <stdint.h>

#define BLE_SVC_GW_UUI128   		0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E
#define BLE_SVC_GW_CHR_UUID128_RX   0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E
#define BLE_SVC_GW_CHR_UUID128_TX   0x79, 0x41, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E

#define BLE_GATT_USR_DSC_CHR_UUID16  0x2901

extern uint16_t gwRxValHandle;
extern uint16_t gwTxValHandle;

int  ble_svc_gw_init(void);

int ble_svc_gw_notify(uint16_t conn_handle, uint8_t *pdata, uint32_t len);

#endif /* GW_H_ */
