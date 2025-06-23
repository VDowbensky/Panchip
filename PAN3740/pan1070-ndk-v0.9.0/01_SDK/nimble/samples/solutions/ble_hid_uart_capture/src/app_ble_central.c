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
#include "uart_AT.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/


/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
uint16_t central_conn_handle;
uint16_t central_chr_write_handle;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static int app_ble_central_gap_event(struct ble_gap_event *event, void *arg);


/*******************************************************************************
 * Function Define
 ******************************************************************************/
static int app_ble_on_write(uint16_t conn_handle,
							 const struct ble_gatt_error *error,
							 struct ble_gatt_attr *attr,
							 void *arg)
{
	APP_LOG_INFO("Write complete; status=0x%02x conn_handle=%d attr_handle=%d\n",
	              error->status, conn_handle, attr->handle);
    return 0;
}

static int app_ble_on_subscribe(uint16_t conn_handle,
								 const struct ble_gatt_error *error,
								 struct ble_gatt_attr *attr,
								 void *arg)
{
	APP_LOG_INFO("Subscribe complete; status=0x%02x conn_handle=%d attr_handle=%d\n",
	         error->status, conn_handle, attr->handle);

	return 0;
}

static void app_ble_read_write_subscribe(const struct peer *peer)
{
	const struct peer_chr *chr;
	const struct peer_dsc *dsc;
	uint8_t value[2];
	int rc;

	/* Get Write Att Handle. */
	chr = peer_chr_find_uuid(peer,
							 BT_UUID_CUSTOM_SERVICE_VAL,
							 BT_UUID_CUSTOM_CHARAC_VAL2);

	central_chr_write_handle = chr->chr.val_handle;


	/* Enable Notify */
	dsc = peer_dsc_find_uuid(peer,
							 BT_UUID_CUSTOM_SERVICE_VAL,
							 BT_UUID_CUSTOM_CHARAC_VAL1,
							 BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
	if (dsc == NULL) {
		APP_LOG_ERR("Enable notify failed...\r\n");
		goto err;
	}

	value[0] = 1;
	value[1] = 0;
	rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle,
							  value, sizeof value, app_ble_on_subscribe, NULL);
	if (rc != 0) {
		APP_LOG_ERR("Failed to subscribe to characteristic; rc=%d\n", rc);
		goto err;
	}

	return;

err:
	/* Terminate the connection. */
	ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
}

void app_ble_on_sdp_complete(const struct peer *peer, int status, void *arg)
{
	if (status != 0) {
		/* Service discovery failed.  Terminate the connection. */
		APP_LOG_ERR("Error: Service discovery failed; status=%d conn_handle=%d\n", status, peer->conn_handle);
		//ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
		return;
	}

	/* Service discovery has completed successfully.  Now we have a complete
	 * list of services, characteristics, and descriptors that the peer
	 * supports.
	 */
	APP_LOG_INFO("Service discovery complete; status=0x%02x conn_handle=%d\n", status, peer->conn_handle);

	/* Now perform three concurrent GATT procedures against the peer: read,
	 * write, and subscribe to notifications.
	 */
	app_ble_read_write_subscribe(peer);

	TGT_SendMultiData("CONN DONE\n", sizeof("CONN DONE\n") - 1);
}


static int app_ble_is_match_device(const struct ble_gap_disc_desc *disc)
{
	struct ble_hs_adv_fields fields;
	int rc;
	int i;

	/* The device has to be advertising connectability. */
	if (disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
	    disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND) {
		return 0;
	}

	rc = ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);
	if (rc != 0) {
		return rc;
	}

#if 1
	for (i = 0; i < fields.num_uuids128; i++) {
		if (!memcmp(BLE_UUID128(&fields.uuids128[i])->value, BLE_UUID128(BT_UUID_CUSTOM_SERVICE_VAL)->value, 16)) {
			// if (!memcmp(fields.uuids128[i].value, uuid->value, 16)) {
			return 1;
		}
	}
#endif
	return 0;
}

char *app_ble_addr_to_str(const void *addr)
{
    static char buf[6 * 2 + 5 + 1];
    const uint8_t *u8p;

    u8p = addr;
    sprintk(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
            u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    return buf;
}

void app_ble_connect_create(const struct ble_gap_disc_desc *disc)
{
	uint8_t own_addr_type;
	int rc;

	/*Filter adv by rssi*/
	if (disc->rssi < -70) {
		return;
	}

	/* Don't do anything if we don't care about this advertiser. */
	if (!app_ble_is_match_device(disc)) {
		return;
	}

	/* Scanning must be stopped before a connection can be initiated. */
	rc = ble_gap_disc_cancel();
	if (rc != 0) {
		APP_LOG_ERR("Failed to cancel scan; rc=%d\n", rc);
		return;
	}

	/* Figure out address to use for connect (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		APP_LOG_ERR("error determining address type; rc=%d\n", rc);
		return;
	}

	/* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for
	 * timeout.
	 */
	rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, NULL, app_ble_central_gap_event, NULL);
	if (rc != 0) {
		APP_LOG_ERR("Error: Failed to connect to device; addr_type=%d, addr=%s\n; rc=%d",
		       disc->addr.type, app_ble_addr_to_str(disc->addr.val), rc);
		return;
	}
}

void app_ble_scan_start(void)
{
	uint8_t own_addr_type;
	struct ble_gap_disc_params disc_params;
	int rc;

	/* Figure out address to use while advertising (no privacy for now) */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		APP_LOG_ERR("error determining address type; rc=%d\n", rc);
		return;
	}

	disc_params.filter_duplicates = 0;
	disc_params.passive 		  = 1;
	disc_params.itvl 			  = BLE_GAP_SCAN_ITVL_MS(60);
	disc_params.window 			  = BLE_GAP_SCAN_WIN_MS(50);
	disc_params.filter_policy     = 0;
	disc_params.limited 		  = 0;

	rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, app_ble_central_gap_event, NULL);
	if(rc != 0){
		APP_LOG_ERR("scan start failed; rc=%d\n", rc);
		APP_ASSERT(rc == 0);
	}

	APP_LOG_INFO("scan start...\r\n");
}

void app_ble_central_write(uint8_t *data, uint8_t len)
{
	int rc;

	rc = ble_gattc_write_flat(central_conn_handle, central_chr_write_handle,
								data, len, app_ble_on_write, NULL);
	if (rc != 0) {
		APP_LOG_ERR("Error: Failed to write characteristic; rc=%d\n", rc);
	}
}

static int app_ble_central_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc desc;
	struct ble_hs_adv_fields fields;
	int rc;

	switch (event->type)
	{
	case BLE_GAP_EVENT_DISC:
		rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
		if (rc != 0) {
			return 0;
		}

	#if 0
		APP_LOG_INFO("Device Found:\r\n\t-peer_addr_type:%d\r\n\t-peer_addr: ", event->disc.addr.type);
		APP_LOG_DATA(event->disc.addr.val, 6);
	#endif

		app_ble_connect_create(&event->disc);
		return 0;

	case BLE_GAP_EVENT_CONNECT:
		if(event->connect.status){
			APP_LOG_ERR("Error: Connection failed; status=%d\n", event->connect.status);
			app_ble_scan_start();
			central_conn_handle = 0xFFFF;
			return 0;
		}

		central_conn_handle = event->connect.conn_handle;

		APP_LOG_INFO("Central Connection established \r\n");
		rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
		APP_ASSERT(rc == 0);
		APP_LOG("\t-conn_intvl:%d us\r\n\t-latency:%d\r\n\t-to:%d ms\r\n",
			     desc.conn_itvl*1250, desc.conn_latency, desc.supervision_timeout*10);

		/* Remember peer. */
		rc = peer_add(event->connect.conn_handle);
		if (rc != 0) {
			APP_LOG_ERR("Failed to add peer; rc=%d\n", rc);
			return 0;
		}

		/* Perform service discovery. */
		rc = peer_disc_all(event->connect.conn_handle, app_ble_on_sdp_complete, NULL);
		if (rc != 0) {
			APP_LOG_ERR("Failed to discover services; rc=%d\n", rc);
			return 0;
		}

		return 0;

	case BLE_GAP_EVENT_DISCONNECT:
		/* Connection terminated. */
		APP_LOG_INFO("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);

		central_conn_handle = 0xFFFF;

		/* Forget about peer. */
		peer_delete(event->disconnect.conn.conn_handle);

		/* Resume scanning. */
		app_ble_scan_start();
		return 0;

	case BLE_GAP_EVENT_DISC_COMPLETE:
        APP_LOG_INFO("discovery duration expires; reason=%d\n", event->disc_complete.reason);
		return 0;

	case BLE_GAP_EVENT_ENC_CHANGE:
        APP_LOG_INFO("encryption change event; status=%d \r\n", event->enc_change.status);
		return 0;

	case BLE_GAP_EVENT_NOTIFY_RX:
		APP_LOG_INFO("received %s; conn_handle=%d attr_handle=%d "
		       "attr_len=%d\n",
		       event->notify_rx.indication ?
		       "indication" :
		       "notification",
		       event->notify_rx.conn_handle,
		       event->notify_rx.attr_handle,
		       OS_MBUF_PKTLEN(event->notify_rx.om));

		APP_LOG_DATA(event->notify_rx.om->om_data, OS_MBUF_PKTLEN(event->notify_rx.om));

		TGT_SendMultiData("Peri Notify:", sizeof("Peri Notify:") - 1);
		TGT_SendMultiData(event->notify_rx.om->om_data, OS_MBUF_PKTLEN(event->notify_rx.om));

		return 0;

	case BLE_GAP_EVENT_MTU:
		APP_LOG_INFO("mtu update event; conn_handle=%d cid=%d mtu=%d\n",
				   event->mtu.conn_handle,
				   event->mtu.channel_id,
				   event->mtu.value);
		return 0;

	case BLE_GAP_EVENT_REPEAT_PAIRING:
		/* We already have a bond with the peer, but it is attempting to
		 * establish a new secure link.  This app sacrifices security for
		 * convenience: just throw away the old bond and accept the new link.
		 */

		/* Delete the old bond. */
		rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
		APP_ASSERT(rc == 0);
		ble_store_util_delete_peer(&desc.peer_id_addr);

		/* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
		 * continue with the pairing operation.
		 */
		return BLE_GAP_REPEAT_PAIRING_RETRY;

	default:
		break;
	}

	return 0;
}

void app_ble_central_init(void)
{
	int rc = 0;
	(void)rc;

	/* SDP initilaization. */
	rc = peer_init(CONFIG_BT_MAX_NUM_OF_CENTRAL, 32, 32, 32);
	APP_ASSERT(rc == 0);
}
