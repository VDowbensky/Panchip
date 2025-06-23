/**
 *******************************************************************************
 * @file     app.h
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef APP_BLE_H_
#define APP_BLE_H_

#include <stdint.h>
#include "peer.h"

void setup(void);
void loop(void);

/******************************************************************************* 
 * BLE API
 ******************************************************************************/
#include "app_ble_svr.h"

/* Connection parameter config */
#define APP_MST_CONN_INTERVAL  BLE_GAP_CONN_ITVL_MS(50)
#define APP_SLV_CONN_INTERVAL  BLE_GAP_CONN_ITVL_MS(30)
#define APP_CONN_TIMEOUT       BLE_GAP_SUPERVISION_TIMEOUT_MS(2500)

/* connection control block */
typedef struct{
	uint32_t   txPktCnt;
	uint32_t   rxPktCnt;
	uint16_t   connHandle;
	uint8_t    role;
	uint8_t    inUse;

	uint8_t    isEncCmpl;
	uint8_t    notifyEn;
	uint16_t   gwAttHandle;
	
	struct ble_npl_callout   connUpdTimer;
}conn_cb_t;

/* app control block */
typedef struct{
	conn_cb_t    ccb[CONFIG_BT_MAX_NUM_OF_CENTRAL + CONFIG_BT_MAX_NUM_OF_PERIPHERAL];
	uint8_t      numMstConn;
	uint8_t      numSlvConn;
}app_cb_t;

extern app_cb_t appCb;

//connection manager API
conn_cb_t *app_new_conn(uint16_t connHandle, uint8_t role);
void app_release_conn(uint16_t connHandle);
conn_cb_t *app_get_conn(uint16_t connHandle);

void app_ble_data_tx(void);

//address filter
bool app_is_addr_contain(uint8_t addr[6]);

//BLE base API
void app_ble_init(void);
void app_ble_task_start(void);

//BLE event call-back
int app_ble_gap_event_cb(struct ble_gap_event *event, void *arg);

//BLE Central API
void app_ble_central_init(void);
void app_ble_scan_start(void);
void app_ble_connect_create(const struct ble_gap_disc_desc *disc);
void app_ble_sdp_complete_cb(const struct peer *peer, int status, void *arg);

//BLE Periph API
void app_ble_periph_init(void);
void app_ble_adv_start(void);

#endif
