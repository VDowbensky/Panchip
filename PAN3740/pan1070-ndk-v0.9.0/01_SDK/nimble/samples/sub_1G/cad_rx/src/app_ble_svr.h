/**
 *******************************************************************************
 * @file     app_ble_svr.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef APP_BLE_SVR_H_
#define APP_BLE_SVR_H_

#include "../../../../profiles/gap/gap.h"
#include "../../../../profiles/gatt/gatt.h"
#include "../../../../profiles/hrs/hrs.h"
#include "../../../../profiles/dis/dis.h"
#include "../../../../profiles/ans/ans.h"
#include "../../../../profiles/gw/gw.h"
#include "../../../../profiles/bas/bas.h"

extern uint16_t sub_1g_handle;/* Custom Service Variables */

#define TRX_SERVICE_UUID        0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x20, 0x40, 0x6e
#define TRX_SERVICE_UUID1       0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x20, 0x40, 0x6e
#define TRX_SERVICE_UUID2       0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x20, 0x40, 0x6e

int gatt_svr_init(void);

/* BLE Service API */
extern uint16_t conn_handle;
void app_ble_svc_init(void);

#endif

