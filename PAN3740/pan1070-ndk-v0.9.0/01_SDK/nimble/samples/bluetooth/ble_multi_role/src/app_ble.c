/**
 *******************************************************************************
 * @file     app_ble.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include <assert.h>
#include <string.h>

#include "pan_ble.h"

#include "utils/bd_addr.h"

#include "app.h"
#include "app_log.h"
#include "utility.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		0

#define APP_DATA_TEST_EN     		1
#define APP_DATA_TOKEN       		0xAA /* Rx data token. */
#define APP_DATA_SEND_ITVL_MS       300

/**@brief Test Config */
/* Enable Slave Test. */
#define APP_SLV_TEST_EN      		0 /* Need to set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 1 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 0. */
/* Select Slave ID. */
#define APP_SLV_DEVICE_ID    		0 /* Slave device can be 0,1,2 */


/* Enable Master Test */
#define APP_MST_TEST_EN      		0 /* Need to set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 0 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 1. */
/* Set Master Address Filt Length. */
#define APP_ADDR_MATCH_LEN   		4 /* use for master address filter */


/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
#if APP_MST_TEST_EN
bdAddr_t addrFilterTable [] = {
	{0xD0, 0x00, 0x00, 0x00, 0x01, 0x39},
};
#else
bdAddr_t addrFilterTable [] ={
	{0x66, 0x66, 0x66, 0x66, 0x01, 0xC2},
	{0x66, 0x66, 0x66, 0x66, 0x03, 0xC2},
	{0x66, 0x66, 0x66, 0x66, 0x04, 0xC2},
};
#endif

/* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 1 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 0. */
#if APP_SLV_TEST_EN
	static const char *device_name = "mutil_conn_test_S";
	#undef APP_DATA_TOKEN
	#define APP_DATA_TOKEN       0x77
/* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 0 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 1. */
#elif APP_MST_TEST_EN
	static const char *device_name = "mutil_conn_test_M";
	#undef APP_DATA_TOKEN
	#define APP_DATA_TOKEN       0x77
/* Need Set CONFIG_BT_MAX_NUM_OF_PERIPHERAL = 2 && CONFIG_BT_MAX_NUM_OF_CENTRAL = 3. */
#else
	static const char *device_name = "mutil_conn_107";
#endif

app_cb_t appCb;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static void app_conn_upd_cb(struct ble_npl_event *ev);
extern void app_ble_rx(uint16_t connHandle, struct os_mbuf *om);

/*******************************************************************************
 * Function Define
 ******************************************************************************/

/*******************************************************************************
 * Connection Manager
 ******************************************************************************/
conn_cb_t *app_new_conn(uint16_t connHandle, uint8_t role)
{
	conn_cb_t *pCcb = NULL;
	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->inUse == false){
			pCcb->inUse = true;
			pCcb->connHandle = connHandle;
			pCcb->role = role;
			pCcb->isEncCmpl = false;
			pCcb->notifyEn = false;
			pCcb->gwAttHandle = 0; // invalid handle
			pCcb->txPktCnt = 0;
			pCcb->rxPktCnt = 0;

			if(pCcb->connUpdTimer.handle == NULL){
				ble_npl_callout_init(&pCcb->connUpdTimer, nimble_port_get_dflt_eventq(),
				                     app_conn_upd_cb, &pCcb->connHandle);
			}

			if(role == BLE_GAP_ROLE_MASTER){
				appCb.numMstConn++;
			}else{
				appCb.numSlvConn++;
			}
			return pCcb;
		}
	}

	return NULL;
}

void app_release_conn(uint16_t connHandle)
{
	conn_cb_t *pCcb = NULL;
	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->connHandle == connHandle){
			pCcb->inUse = false;
			pCcb->connHandle = 0xFFFF;
			pCcb->isEncCmpl = false;
			pCcb->notifyEn = false;
			pCcb->gwAttHandle = 0; // invalid handle
			pCcb->txPktCnt = 0;
			pCcb->rxPktCnt = 0;

			if(ble_npl_callout_is_active(&pCcb->connUpdTimer)){
				ble_npl_callout_stop(&pCcb->connUpdTimer);
			}

			if(pCcb->role == BLE_GAP_ROLE_MASTER){
				if(appCb.numMstConn>0) appCb.numMstConn--;
			}else{
				if(appCb.numSlvConn>0) appCb.numSlvConn--;
			}
			return;
		}
	}
}

conn_cb_t *app_get_conn(uint16_t connHandle)
{
	conn_cb_t *pCcb = NULL;
	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->connHandle == connHandle){
			return pCcb;
		}
	}

	return NULL;
}

/*******************************************************************************
 * Address Filter
 ******************************************************************************/
bool app_is_addr_contain(uint8_t addr[6])
{
	for(int i=0;i<COUNTOF(addrFilterTable); i++)
	{
		if(!memcmp(addr, &addrFilterTable[i], APP_ADDR_MATCH_LEN)){
			return true;
		}
	}
	return false;
}

/*******************************************************************************
 * Connection Update
 ******************************************************************************/
void app_conn_upd_cb(struct ble_npl_event *ev)
{
	uint16_t connHandle = *(uint16_t *)ev->arg;

	struct ble_gap_upd_params new_param;
	new_param.itvl_max = APP_SLV_CONN_INTERVAL;
	new_param.itvl_min = APP_SLV_CONN_INTERVAL;
	new_param.latency  = 0;
	new_param.supervision_timeout = APP_CONN_TIMEOUT;
	new_param.max_ce_len = 0;
	new_param.min_ce_len = 0;

	int rc = ble_gap_update_params(connHandle, &new_param);
	if(rc){
		APP_LOG_WRN("conn update failed - connHandle:%d, rc:%02x\r\n", connHandle, rc);
	}
}

void app_conn_upd_timer_start(conn_cb_t *pCcb)
{
	APP_LOG_WRN("Conn Update Timer start...\n");
	ble_npl_callout_reset(&pCcb->connUpdTimer, pdMS_TO_TICKS(1000));
}


/*******************************************************************************
 * Data Send Test
 ******************************************************************************/
#if APP_DATA_TEST_EN
static volatile uint8_t data_send_enable = 0;
static volatile uint8_t is_data_send_ready = 0; // Indicates whether the user can send data.

static uint8_t buf[256] = {0};
static uint8_t data[244]= {0};

void app_ble_rx(uint16_t connHandle, struct os_mbuf *om)
{
	if(om == NULL || connHandle == 0xFFFF)
		return;

	uint8_t *pdata = buf;
	uint16_t len = 0;
	conn_cb_t *pCcb = NULL;

	int rc = ble_hs_mbuf_to_flat(om, buf, sizeof(buf), &len);
	if(rc){
		return;
	}

	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		pCcb = &appCb.ccb[i];
		if(pCcb->inUse && pCcb->connHandle == connHandle){
			APP_LOG_INFO("Rx: connHandle:%d, dataLen:%d, rxCnt:%d\n", connHandle, len, pCcb->rxPktCnt);
			//APP_LOG_DATA(pdata, len);

			uint32_t rxCnt = 0;
			rxCnt = (pdata[3]<<24) | (pdata[2]<<16) | (pdata[1]<<8) | pdata[0];

			/* Check if packet id is valid. */
			if(pCcb->rxPktCnt != rxCnt){
				APP_LOG_ERR("rxCnt error\n");
				goto done;
			}

			/* Check if data is valid. */
			uint8_t token = APP_DATA_TOKEN;
			for(int i=4; i<len; i++)
			{
				if(pdata[i] != token){
					APP_LOG_ERR("data error\n");
					goto done;
				}
			}

			pCcb->rxPktCnt++; //
			break;
		}
	}

	return;

done:
	APP_LOG_ERR("Rx error(handle:%d, rxLen:%d, rxCnt:%d):\n", connHandle, len, pCcb->rxPktCnt);
	APP_LOG_DATA(pdata, len);
}

void app_ble_data_tx(void)
{
	int rc = 0;

	if(data_send_enable == false || is_data_send_ready == false){
		return;
	}

	for(int i=0; i<COUNTOF(appCb.ccb); i++)
	{
		conn_cb_t *pCcb = &appCb.ccb[i];
		//if(pCcb->inUse && pCcb->gwAttHandle != 0 && pCcb->notifyEn && pCcb->isEncCmpl)
		if(pCcb->inUse && pCcb->gwAttHandle != 0 && pCcb->notifyEn)
		{
			Uint32ToBytes(data, pCcb->txPktCnt);

			if(pCcb->role == BLE_GAP_ROLE_MASTER){
				rc = ble_gattc_write_no_rsp_flat(pCcb->connHandle, pCcb->gwAttHandle, data, sizeof data);
			}
			else{
				rc = ble_svc_gw_notify(pCcb->connHandle, data, sizeof data);
			}
			if(rc == 0){
				pCcb->txPktCnt++;
				APP_LOG_INFO("Tx: connHandle:%d, TxCnt:%d\n", pCcb->connHandle, pCcb->txPktCnt);
			}
			else{
				return;
			}
		}
	}
}

void key_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);

	GPIO_SetMode(P0, BIT(6), GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P0, BIT(6));
	GPIO_EnablePullupPath(P0, BIT(6));

	GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

    NVIC_EnableIRQ(GPIO0_IRQn);
}

void GPIO0_IRQHandlerOverlay(void)
{
	if(GPIO_GetIntFlag(P0,  BIT(6)))
	{
		GPIO_ClrIntFlag(P0, BIT(6));
		data_send_enable = !data_send_enable;
		APP_LOG_WRN("data send %s...\n", data_send_enable ? "enable" : "disable");
	}
}
#else
void app_ble_data_tx(void)
{

}
#endif

/*******************************************************************************
 * BLE Event
 ******************************************************************************/
int app_ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc desc;
	struct ble_hs_adv_fields fields;
	int rc;

	switch (event->type)
	{
	case BLE_GAP_EVENT_DISC:
	#if 0
		APP_LOG_INFO("Device Found peer_addr_type:%d peer_addr: %s\r\n", event->disc.addr.type, addr_to_str(event->disc.addr.val));
	#endif
	
		rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
		if (rc != 0){
			return 0;
		}

		app_ble_connect_create(&event->disc);

		return 0;

	case BLE_GAP_EVENT_CONNECT:
	{
		if(event->connect.status)
		{
			/* Connection attempt failed */
			APP_LOG_WRN("Connection failed; status=%d\n", event->connect.status);

			if(appCb.numMstConn < CONFIG_BT_MAX_NUM_OF_CENTRAL){
				app_ble_scan_start();
			}

			if(appCb.numSlvConn < CONFIG_BT_MAX_NUM_OF_PERIPHERAL)
			{
				app_ble_adv_start();
			}
			return 0;
		}

		rc = ble_gap_conn_find(event->connect.conn_handle, &desc);

		/* Connection successfully established. */
		APP_LOG_INFO("=== Connection established [%s]===\n", desc.role == BLE_GAP_ROLE_MASTER? "central":"periph");

		APP_ASSERT(rc==0);
        APP_LOG("-peer_ota_addr: %s(at: %d)\r\n",
                addr_to_str(desc.peer_ota_addr.val),
                desc.peer_ota_addr.type);

        APP_LOG("-peer_id_addr: %s(at: %d)\r\n",
                 addr_to_str(desc.peer_id_addr.val),
                 desc.peer_id_addr.type);

		APP_LOG("-conn_itvl: %d us\r\n"
                "-latency: %d\r\n"
				"-timeout: %d ms\r\n",
			     desc.conn_itvl*1250,
				 desc.conn_latency,
				 desc.supervision_timeout*10);

		conn_cb_t *pCcb = app_new_conn(desc.conn_handle, desc.role);
		APP_ASSERT(pCcb != NULL);

		rc = ble_gap_set_data_len(desc.conn_handle, MYNEWT_VAL_BLE_TRANSPORT_ACL_SIZE, 2120);
		APP_ASSERT(rc==0);

	#if APP_DATA_TEST_EN
		is_data_send_ready = true;
	#endif

		rc = ble_gattc_exchange_mtu(desc.conn_handle, NULL, NULL);
		APP_ASSERT(rc == 0);

		// Master
		if(desc.role == BLE_GAP_ROLE_MASTER)
		{
			/* Remember peer. */
			rc = peer_add(event->connect.conn_handle);
			if (rc != 0){
				APP_LOG_ERR("Failed to add peer; rc=%d\n", rc);
				return 0;
			}

			/* Perform service discovery. */
			rc = peer_disc_all(event->connect.conn_handle, app_ble_sdp_complete_cb, NULL);
			if (rc != 0){
				APP_LOG_ERR("Failed to discover services; rc=%d\n", rc);
				return 0;
			}
			else{
				APP_LOG_WRN("SDP start\n");
			}
		}
		else //slave
		{
		#if !APP_SLV_TEST_EN
			if(desc.conn_itvl < APP_SLV_CONN_INTERVAL || desc.supervision_timeout < APP_CONN_TIMEOUT){
				app_conn_upd_timer_start(pCcb);
			}
		#endif

			if(appCb.numSlvConn < CONFIG_BT_MAX_NUM_OF_PERIPHERAL){
				app_ble_adv_start();
			}
		}
		return 0;
	}
	case BLE_GAP_EVENT_DISCONNECT:
	{
		/* Connection terminated. */
		APP_LOG_INFO("=== Disconnect Cmpl ===\n -reason:0x%02x \n", event->disconnect.reason);

		app_release_conn(event->disconnect.conn.conn_handle);

		/* Resume scanning. */
		if(event->disconnect.conn.role == BLE_GAP_ROLE_MASTER)
		{
			/* Forget about peer. */
			peer_delete(event->disconnect.conn.conn_handle);

			if(appCb.numMstConn < CONFIG_BT_MAX_NUM_OF_CENTRAL){
				app_ble_scan_start();
			}
		}
		else{
			if(appCb.numSlvConn < CONFIG_BT_MAX_NUM_OF_PERIPHERAL){
				app_ble_adv_start();
			}
		}

	#if APP_DATA_TEST_EN
		if(appCb.numMstConn == 0 && appCb.numSlvConn == 0){
			is_data_send_ready = false;
		}
	#endif
		return 0;
	}
	case BLE_GAP_EVENT_DISC_COMPLETE:
		APP_LOG_INFO("scan discovery complete; reason=%d\n", event->disc_complete.reason);
		app_ble_scan_start();
		return 0;

	case BLE_GAP_EVENT_ENC_CHANGE:
	{
		/* Encryption has been enabled or disabled for this connection. */
		APP_LOG_WRN("encryption change event; status=%d \n", event->enc_change.status);

		conn_cb_t *pCcb = app_get_conn(event->subscribe.conn_handle);
		if(pCcb == NULL){
			APP_LOG_ERR("Invalid connHandle\n");
			return 0;
		}

		pCcb->isEncCmpl = true;
		return 0;
	}
	case BLE_GAP_EVENT_SUBSCRIBE:
	{
		if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_WRITE)
		{
			conn_cb_t *pCcb = app_get_conn(event->subscribe.conn_handle);
			if(pCcb == NULL){
				APP_LOG_ERR("subscribe failed\n");
				return 0;
			}

			/* save slave tx chr attr handle */
			pCcb->gwAttHandle = event->subscribe.attr_handle; // record gwTxAttHandle.
			pCcb->notifyEn = event->subscribe.cur_notify;     // flag peripheral notify(CCCD) state: enable/disable.
		}

		return 0;
	}
	case BLE_GAP_EVENT_NOTIFY_RX:
	{
		struct os_mbuf *om = event->notify_rx.om;
		app_ble_rx(event->notify_rx.conn_handle, om);
		return 0;
	}
	case BLE_GAP_EVENT_NOTIFY_TX:
		//APP_LOG_INFO("notify send ok(connHandle:%d, attHandle:%d)\n",
		//				event->notify_tx.conn_handle, event->notify_tx.attr_handle);
		return 0;

	case BLE_GAP_EVENT_MTU:
		APP_LOG_INFO("MTU update event \r\n" " -conn_handle=%d\n" " -cid=%d\n" " -mtu=%d\n", event->mtu.conn_handle,
						event->mtu.channel_id, event->mtu.value);
		return 0;

	case BLE_GAP_EVENT_REPEAT_PAIRING:
		/* We already have a bond with the peer, but it is attempting to
		 * establish a new secure link.  This app sacrifices security for
		 * convenience: just throw away the old bond and accept the new link.
		 */

		/* Delete the old bond. */
		rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
		APP_ASSERT(rc==0);
		ble_store_util_delete_peer(&desc.peer_id_addr);

		/* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
		 * continue with the pairing operation.
		 */
		return BLE_GAP_REPEAT_PAIRING_RETRY;

	case BLE_GAP_EVENT_CONN_UPDATE:
		ble_gap_conn_find(event->conn_update.conn_handle, &desc);
		APP_LOG_INFO("conn upd cmpl: conn_handle:%d, itvl:%d us, latency:%d, to:%d ms\n",
						desc.conn_handle,
						desc.conn_itvl * 1250,
						desc.conn_latency,
						desc.supervision_timeout*10);
		return 0;

	default:
		break;
	}

	return 0;
}

static void app_ble_set_mac_addr(void)
{
    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};

#if CONFIG_USER_CHIP_MAC_ADDR
	pan10x_mac_addr_get(pub_mac);
#endif

#if APP_USER_ROLL_MAC_ADDR
	pan10x_roll_mac_addr_get(pub_mac);
#endif

#if APP_SLV_TEST_EN
	memcpy(pub_mac, &addrFilterTable[APP_SLV_DEVICE_ID], 6);
#endif

    pan_misc_set_bd_addr(pub_mac);
}

/**
 * @brief: BLE Stack Enabled call-back.
 */
static void app_ble_enabled_cb(void)
{
    int rc;
	(void)rc;

#if CONFIG_BT_MAX_NUM_OF_PERIPHERAL
	/* Adv */
	app_ble_adv_start();
#endif

#if CONFIG_BT_MAX_NUM_OF_CENTRAL
	/* Scan */
	app_ble_scan_start();
#endif
}

/**
 * @brief: ble stack pre init call-back
 *         !!!Note: 1. The Bluetooth MAC Address must be set in this callback.
 *                  2. The BLE Services must be registered in this callback.
 *                  3. the SDP initialzation must be executed in this callback.
 */
void app_ble_pre_init_cb(void)
{
	int rc;
   (void)rc;

	/* Set MAC address. */
    app_ble_set_mac_addr();

	/* ble peripheral initialzation. */
	app_ble_periph_init();

	/* ble central initialzation. */
	app_ble_central_init();

	/* BLE Service initialzation */
    app_ble_svc_init();

	/* Set device name. */
    rc = ble_svc_gap_device_name_set(device_name);
    APP_ASSERT(rc == 0);

	/* BLE Application initialization. */
#if APP_DATA_TEST_EN
	memset(&data[0], 0x77, sizeof(data));

#if APP_SLV_TEST_EN
	memset(&data[0], 0xAA, sizeof(data));
#endif

#if APP_MST_TEST_EN
	memset(&data[0], 0xAA, sizeof(data));
#endif

	key_init();
#endif
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
