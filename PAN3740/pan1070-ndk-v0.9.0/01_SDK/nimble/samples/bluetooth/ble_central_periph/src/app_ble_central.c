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
#include "ble_dev_filt.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@brief ble device filter. */
#define APP_BLE_DEV_FLT_EN               1
// filt device name
#define APP_FILT_DEV_NAME_EN             1
#define APP_FILT_DEV_NAME                "ble_hr"
// filt device address
#define APP_FILT_DEV_ADDR_EN             0
#define APP_FILT_DEV_ADDR_LEN            5 //address match length; range 1~6
#define APP_FILT_DEV_ADDR                0x66, 0x66, 0x66, 0x66, 0x33, 0xC2
// filt UUID16
#define APP_FILT_UUID16_EN               0
#define APP_FILT_UUID16                  BLE_SVC_HRS_UUID16

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static void app_ble_connect_create(const struct ble_gap_disc_desc *disc);


/*******************************************************************************
 * Service Discovery(SDP) API
 ******************************************************************************/
static int app_ble_on_read(uint16_t conn_handle,
							const struct ble_gatt_error *error,
							struct ble_gatt_attr *attr,
							void *arg)
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

static int app_ble_on_write(uint16_t conn_handle,
							 const struct ble_gatt_error *error,
							 struct ble_gatt_attr *attr,
							 void *arg)
{
	APP_LOG_INFO("Write complete; status=%d conn_handle=%d attr_handle=%d\n",
	              error->status, conn_handle, attr->handle);
    return 0;
}

static int app_ble_on_subscribe(uint16_t conn_handle,
								 const struct ble_gatt_error *error,
								 struct ble_gatt_attr *attr,
								 void *arg)
{
	APP_LOG_INFO("Subscribe complete; status=%d conn_handle=%d " "attr_handle=%d\n",
	              error->status, conn_handle, attr->handle);

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

    /* Get characteristic of user specific. */
    chr = peer_chr_find_uuid(peer,
                             BLE_UUID16_DECLARE(BLE_SVC_HRS_UUID16),
                             BLE_UUID16_DECLARE(BLE_SVC_HRS_CHR_UUID16_MEASUREMENT));
    if (chr == NULL) {
        APP_LOG_WRN("Peer doesn't support the user specific characteristic\n");
        goto err;
    }

	/* Read characteristic data. */
    rc = ble_gattc_read(peer->conn_handle, chr->chr.val_handle,
                        app_ble_on_read, NULL);
    if (rc != 0) {
        APP_LOG_WRN("Failed to read characteristic; rc=%d\n", rc);
        goto err;
    }

	/* Write characteristic data. */
    memset(value, 0, sizeof(value));
    rc = ble_gattc_write_flat(peer->conn_handle, chr->chr.val_handle,
                              value, sizeof value, app_ble_on_write, NULL);
    if (rc != 0) {
        APP_LOG_WRN("Failed to write characteristic; rc=%d\n", rc);
    }

	/* Get CCCD of user specific. */
    dsc = peer_dsc_find_uuid(peer,
                             BLE_UUID16_DECLARE(BLE_SVC_HRS_UUID16),
                             BLE_UUID16_DECLARE(BLE_SVC_HRS_CHR_UUID16_MEASUREMENT),
                             BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if (dsc == NULL) {
		APP_LOG_WRN("Peer doesn't support the user specific CCCD\n");
        goto err;
    }

	/* Subscribe notify. */
	Uint16ToBytes(value, 0x0001);
    rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle,
                              value, sizeof value, app_ble_on_subscribe, NULL);
    if (rc != 0) {
        APP_LOG_WRN("Failed to subscribe to characteristic; rc=%d\n", rc);
        goto err;
    }

    return;

err:
#if 0
	APP_LOG_WRN("Chr read write err - disconnect\r\n");
    /* Terminate the connection. */
    ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
#else
	APP_LOG("\n");
#endif
}

static void app_ble_sdp_complete_cb(const struct peer *peer, int status, void *arg)
{
    if (status != 0) {
        /* Service discovery failed.  Terminate the connection. */
		APP_LOG_WRN("Service discovery failed; status=%d conn_handle=%d\n", status, peer->conn_handle);
		APP_LOG_INFO("Send disconnect cmd \r\n");
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

    /* Service discovery has completed successfully.  Now we have a complete
     * list of services, characteristics, and descriptors that the peer
     * supports.
     */
    APP_LOG_INFO("Service discovery complete; status=%d conn_handle=%d\n", status, peer->conn_handle);

    /* Now perform three concurrent GATT procedures against the peer: read,
     * write, and subscribe to notifications.
     */
    app_ble_read_write_subscribe(peer);
}

static int app_ble_central_gap_event_cb(struct ble_gap_event *event, void *arg)
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
        if (rc != 0) {
            return 0;
        }

        app_ble_connect_create(&event->disc);
        return 0;

    case BLE_GAP_EVENT_CONNECT:
        APP_LOG_INFO("Central connection %s; status=%d\n",
                        event->connect.status == 0 ? "established" : "failed",
                        event->connect.status);

		if(event->connect.status){
			app_ble_scan_start();
			break;
		}

		rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
		APP_ASSERT(rc == 0);
        APP_LOG("\t-peer_ota_addr: %s(at: %d)\r\n",
                addr_to_str(desc.peer_ota_addr.val),
                desc.peer_ota_addr.type);

        APP_LOG("\t-peer_id_addr: %s(at: %d)\r\n",
                 addr_to_str(desc.peer_id_addr.val),
                 desc.peer_id_addr.type);

		APP_LOG("\t-conn_itvl: %d us\r\n"
                "\t-latency: %d\r\n"
				"\t-timeout: %d ms\r\n",
                desc.conn_itvl*1250,
                desc.conn_latency,
                desc.supervision_timeout*10);

	#if 0
		// start pair
		rc = ble_gap_pair_initiate(event->connect.conn_handle);
		if(rc != 0){
			APP_LOG_WRN("start security failed - rc:%d\n", rc);
		}
	#endif

		/* Remember peer. */
		rc = peer_add(event->connect.conn_handle);
		if (rc != 0) {
			APP_LOG_ERR("Failed to add peer; rc=%d\n", rc);
			return 0;
		}

		/* Perform service discovery(SDP). */
		rc = peer_disc_all(event->connect.conn_handle, app_ble_sdp_complete_cb, NULL);
		if (rc != 0) {
			APP_LOG_ERR("Failed to discover services; rc=%d\n", rc);
			return 0;
		}

        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);

        /* Forget about peer. */
        peer_delete(event->disconnect.conn.conn_handle);

        /* Resume scanning. */
        app_ble_scan_start();

        return 0;

    case BLE_GAP_EVENT_DISC_COMPLETE:
        APP_LOG_INFO("discovery duration expires; reason=%d\n", event->disc_complete.reason);
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        APP_LOG_INFO("encryption change event; status=%d \r\n", event->enc_change.status);
        return 0;

    case BLE_GAP_EVENT_NOTIFY_RX:
        /* Peer sent us a notification or indication. */
		 APP_LOG_INFO("received %s; conn_handle=%d,attr_handle=%d,attr_len=%d\n",
							event->notify_rx.indication ? "indication" : "notification",
							event->notify_rx.conn_handle,
							event->notify_rx.attr_handle,
							OS_MBUF_PKTLEN(event->notify_rx.om));

        /* Attribute data is contained in event->notify_rx.attr_data. */
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
        return 0;
    }

	return 0;
}

/*******************************************************************************
 * Scan API
 ******************************************************************************/
static void app_ble_connect_create(const struct ble_gap_disc_desc *disc)
{
    uint8_t own_addr_type;
    int rc;

    /*Filter adv by rssi*/
    if (disc->rssi < -70){
        return;
    }

	/* The device has to be advertising connectability. */
    if (disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
        disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND) {
        return;
    }

	/* Check if device is match. */
#if APP_BLE_DEV_FLT_EN
	if(!ble_dev_filt_is_match(disc)){
		return;
	}
#else
	return;
#endif

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

    /* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for timeout. */
    rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, NULL,
                         app_ble_central_gap_event_cb, NULL);
    if (rc != 0) {
        APP_LOG_ERR("Error: Failed to connect to device; addr_type=%d addr=%s\n; rc=%d",
                    disc->addr.type, addr_to_str(disc->addr.val), rc);
        return;
    }
}

void app_ble_scan_start(void)
{
    uint8_t own_addr_type;
    struct ble_gap_disc_params disc_params;
    int rc;

    /* get local id address type */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        APP_LOG_ERR("error determining address type; rc=%d\n", rc);
        return;
    }

    disc_params.filter_duplicates 	= 0;
    disc_params.passive 			= 1;
    disc_params.itvl 		 		= BLE_GAP_SCAN_ITVL_MS(60);
    disc_params.window 		  		= BLE_GAP_SCAN_ITVL_MS(50);;
    disc_params.filter_policy 		= 0;
    disc_params.limited 			= 0;

    rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params,
                      app_ble_central_gap_event_cb, NULL);
    if (rc != 0) {
        APP_LOG_ERR("Error initiating GAP discovery procedure; rc=%d\n", rc);
    }

	APP_LOG_INFO("Scan start...\r\n");
}

void app_ble_central_init(void)
{
	/* SDP initialization */
    int rc = peer_init(CONFIG_BT_MAX_NUM_OF_CENTRAL, 32, 32, 32);
    APP_ASSERT(rc == 0);

		/* BLE Device Filter initialization. */
#if APP_BLE_DEV_FLT_EN
	ble_dev_filt_init();

	// Set Filter RSSI
	ble_dev_filt_set_rssi(-60);

	// Set Filter Device Name
  #if APP_FILT_DEV_NAME_EN
	ble_dev_filt_set_device_name(APP_FILT_DEV_NAME, strlen(APP_FILT_DEV_NAME));
  #endif

	// Set Filter Device Address
  #if APP_FILT_DEV_ADDR_EN
	ble_dev_filt_add_addr((uint8_t[]){APP_FILT_DEV_ADDR}, APP_FILT_DEV_ADDR_LEN);
  #endif

	// Set Filter UUID16
  #if APP_FILT_UUID16_EN
	ble_dev_filt_set_uuid16(APP_FILT_UUID16);
  #endif
#endif
}
