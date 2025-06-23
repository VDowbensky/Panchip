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


/* BLE Service API */
void app_ble_svc_init(void);

int ble_svc_throughput_send_notify(uint16_t connHandle, uint8_t *pdata, uint32_t len);

#endif

