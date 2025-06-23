/**
 *******************************************************************************
 * @file     app_ble_central.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app.h"
#include "pan_ble.h"
#include "app_log.h"
#include "utility.h"

/*******************************************************************************
 * Service Discovery(SDP) API
 ******************************************************************************/
int app_ble_on_read(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg)
{
	APP_LOG_INFO("Read complete; status=%d conn_handle=%d\n", error->status, conn_handle);

	uint8_t buf[250];
	uint16_t len = 0;

    if (error->status == 0) {
        APP_LOG_INFO(" attr_handle=%d len:%d value=\n", attr->handle, OS_MBUF_PKTLEN(attr->om));

		int rc = ble_hs_mbuf_to_flat(attr->om, buf, sizeof(buf), &len); //must
		if(rc == 0){
			APP_LOG_DATA(buf, len);
		}
    }

	return 0;
}

int app_ble_on_write(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr, void *arg)
{
	APP_LOG_INFO("Write complete; status=%d conn_handle=%d attr_handle=%d\n", error->status, conn_handle,
					attr->handle);
	return 0;
}

int app_ble_on_subscribe(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr,
				void *arg)
{
	APP_LOG_INFO("Subscribe complete; status=%d conn_handle=%d attr_handle=%d\n", error->status, conn_handle,
					attr->handle);

	return 0;
}

/**
 * Performs three concurrent GATT operations against the specified peer:
 * 1. Reads the ANS Supported New Alert Category characteristic.
 * 2. Writes the ANS Alert Notification Control Point characteristic.
 * 3. Subscribes to notifications for the ANS Unread Alert Status
 *    characteristic.
 *
 * If the peer does not support a required service, characteristic, or
 * descriptor, then the peer lied when it claimed support for the alert
 * notification service!  When this happens, or if a GATT procedure fails,
 * this function immediately terminates the connection.
 */
static void app_ble_read_write_subscribe(const struct peer *peer)
{
	const struct peer_chr *chr;
	const struct peer_dsc *dsc;
	uint8_t value[2];
	int rc;
	conn_cb_t *pCcb = NULL;

	/* Get Rx characteristic */
    chr = peer_chr_find_uuid(peer,
    				BLE_UUID128_DECLARE(BLE_SVC_GW_UUI128),
					BLE_UUID128_DECLARE(BLE_SVC_GW_CHR_UUID128_RX));
    if (chr == NULL) {
        APP_LOG_WRN("Peer doesn't support the user specific characteristic\n");
        goto err;
    }

    pCcb = app_get_conn(peer->conn_handle);
    if(pCcb == NULL){
    	APP_LOG_ERR("No Found GW Charic \n");
    	goto err;
    }
	// Get Rx att Handle
    pCcb->gwAttHandle = chr->chr.val_handle; //record gwRxAttHandle.


	/* Get Tx CCCD */
	dsc = peer_dsc_find_uuid(peer,
					BLE_UUID128_DECLARE(BLE_SVC_GW_UUI128),
					BLE_UUID128_DECLARE(BLE_SVC_GW_CHR_UUID128_TX),
					BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
	if(dsc == NULL){
        APP_LOG_WRN("Peer doesn't support the user specific CCCD\n");
		goto err;
	}

	/* Enable notify */
	Uint16ToBytes(value, 0x0001);
	rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, value, sizeof value, app_ble_on_subscribe, NULL);
	if(rc != 0){
		APP_LOG_ERR("Failed to subscribe to characteristic; rc=%d\n", rc);
		goto err;
	}

	pCcb->notifyEn = true; // flag subscribe successfully, peripheral can send notify to central.

	return;

err:
#if 1
	APP_LOG_WRN("central can not find specific chr\n");
#else
	APP_LOG_ERR("Chr read write err - disconnect\r\n");
	/* Terminate the connection. */
	ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
#endif
}

/**
 * Called when service discovery of the specified peer has completed.
 */
void app_ble_sdp_complete_cb(const struct peer *peer, int status, void *arg)
{
	if (status != 0)
	{
		/* Service discovery failed.  Terminate the connection. */
		APP_LOG_WRN("Service discovery failed; status=%d " "conn_handle=%d\n", status, peer->conn_handle);
		APP_LOG_INFO("Send disconnect cmd \r\n");
		ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
		return;
	}

	/* Service discovery has completed successfully.  Now we have a complete
	 * list of services, characteristics, and descriptors that the peer
	 * supports.
	 */
	APP_LOG_INFO("Service discovery complete; status=%d " "conn_handle=%d\n", status, peer->conn_handle);

	/* Now perform three concurrent GATT procedures against the peer: read,
	 * write, and subscribe to notifications.
	 */
	app_ble_read_write_subscribe(peer);

	/* Restart Scan */
	if(appCb.numMstConn < CONFIG_BT_MAX_NUM_OF_CENTRAL){
		app_ble_scan_start();
	}
}



/*******************************************************************************
 * Scan API
 ******************************************************************************/
static int app_ble_is_match_device(const struct ble_gap_disc_desc *disc)
{
	if (disc->rssi < -60){
		return 0;
	}

	/* The device has to be advertising connectability. */
	if(disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
	   disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND){
		return 0;
	}

	/* Address Filter */
	if(!app_is_addr_contain((uint8_t *)disc->addr.val)) {
		return 0;
	}
	return 1;
}

void app_ble_connect_create(const struct ble_gap_disc_desc *disc)
{
	uint8_t own_addr_type;
	int rc;

	/* Don't do anything if we don't care about this advertiser. */
	if (!app_ble_is_match_device(disc)){
		return;
	}

	/* Scanning must be stopped before a connection can be initiated. */
	rc = ble_gap_disc_cancel();
	if (rc != 0){
		APP_LOG_ERR("Failed to cancel scan; rc=%d\n", rc);
		return;
	}

	/* Figure out address to use for connect (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		APP_LOG_ERR("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for timeout. */
	struct ble_gap_conn_params conn_params = {
		.scan_itvl   		 = 0x0010,
		.scan_window 		 = 0x0010,
		.itvl_min 			 = APP_MST_CONN_INTERVAL,
		.itvl_max 			 = APP_MST_CONN_INTERVAL,
		.latency  			 = 0,
		.supervision_timeout = APP_CONN_TIMEOUT,
		.min_ce_len 		 = 0,
		.max_ce_len 		 = 0,
	};
	rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, &conn_params, app_ble_gap_event_cb, NULL);
	if (rc != 0){
		APP_LOG_ERR("create connect failed; addr_type=%d addr=%s; rc=%d\n", disc->addr.type,
						addr_to_str(disc->addr.val), rc);
		app_ble_scan_start();
	}
}

void app_ble_scan_start(void)
{
	if(ble_gap_disc_active()){
		return;
	}

	uint8_t own_addr_type;
	struct ble_gap_disc_params disc_params;
	int rc;

	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0){
		APP_LOG_ERR("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Set Scan parameter. */
	disc_params.filter_duplicates = 1;
	disc_params.passive 		  = 1;
	disc_params.itvl 			  = 0;
	disc_params.window 		  	  = 0;
	disc_params.filter_policy 	  = 0;
	disc_params.limited 		  = 0;

	rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, app_ble_gap_event_cb, NULL);
	if(rc != 0){
		APP_LOG_ERR("start scan failed; rc=%d\n", rc);
	}

	APP_LOG_INFO("Starting Scan...\n");
}

void app_ble_central_init(void)
{
	/* SDP initialization. */
    int rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 10, 32, 32);
    APP_ASSERT(rc == 0);
}
