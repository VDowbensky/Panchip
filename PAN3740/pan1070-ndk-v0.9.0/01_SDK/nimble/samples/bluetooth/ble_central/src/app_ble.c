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
#include <string.h>
#include "pan_ble.h"

#include "ble_dev_filt.h"
#include "app.h"
#include "app_log.h"
#include "utility.h"

#include "../../../../profiles/gw/gw.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/**@brief the number of max connection */
#define APP_MAX_CONN_NUM                 CONFIG_BT_MAX_NUM_OF_CENTRAL

/**@brief Enable print scanned device */
#define APP_PRINT_SCANNED_DEV_EN         0

/**@brief ble device filter. */
#define APP_BLE_DEV_FLT_EN               0
// filt device name
#define APP_FILT_DEV_NAME_EN             0
#define APP_FILT_DEV_NAME                "ble_hr"
// filt device address
#define APP_FILT_DEV_ADDR_EN             1
#define APP_FILT_DEV_ADDR_LEN            5 //address match length, range 1~6
#define APP_FILT_DEV_ADDR                0x66, 0x66, 0x66, 0x66, 0x11, 0xC2
// filt UUID16
#define APP_FILT_UUID16_EN               0
#define APP_FILT_UUID16                  0x180D // HRS service UUID

enum{
	APP_MSG_TYPE_DISCONNECT  = 1,
};

typedef struct{
    uint8_t     type;
    uint16_t    conn_handle;
}__attribute__((packed)) msg_t;

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif

static uint8_t conn_create_start = 0;

static uint16_t conn_handle = 0xFFFF;

static SemaphoreHandle_t semphr_ble_disconnect;
static QueueHandle_t msgQueue;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static int app_ble_gap_event(struct ble_gap_event *event, void *arg);
static void app_ble_scan_start(void);

static void app_ble_connect_create(const struct ble_gap_disc_desc *disc);

void app_ble_disconnect_notify(void);

/*******************************************************************************
 * Function Define
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
    APP_LOG_INFO("Subscribe complete; status=%d conn_handle=%d attr_handle=%d\n",
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

    /* Get characteristic info of user specific. */
	chr = peer_chr_find_uuid(peer,
						 BLE_UUID128_DECLARE(BLE_SVC_GW_UUI128),
						 BLE_UUID128_DECLARE(BLE_SVC_GW_CHR_UUID128_RX));
    if(chr == NULL) {
        APP_LOG_WRN("Peer doesn't support the user specific characteristic\n");
        goto err;
    }

	/* Read characteristic info of user specific. */
    rc = ble_gattc_read(peer->conn_handle, chr->chr.val_handle, app_ble_on_read, NULL);
    if(rc != 0) {
        APP_LOG_WRN("Failed to read characteristic; rc=%d\n", rc);
        goto err;
    }

	/* Write data to characteristic info of user specific. */
	memset(value, 0, sizeof value);
    rc = ble_gattc_write_flat(peer->conn_handle, chr->chr.val_handle, value, sizeof value, app_ble_on_write, NULL);
    if(rc != 0) {
        APP_LOG_WRN("Failed to write characteristic; rc=%d\n", rc);
    }

    /* Subscribe to notifications.
     * A central enables notifications by writing 0x001 to the
     * characteristic's client-characteristic-configuration-descriptor (CCCD).
     */
    dsc = peer_dsc_find_uuid(peer,
                        	 BLE_UUID128_DECLARE(BLE_SVC_GW_UUI128),
                             BLE_UUID128_DECLARE(BLE_SVC_GW_CHR_UUID128_TX),
                             BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16));
    if(dsc == NULL) {
        APP_LOG_WRN("Peer doesn't support the user specific CCCD\n");
        goto err;
    }

	/* Enable Server CCCD of user specific. */
	Uint16ToBytes(value, 0x0001);
    rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle,value, sizeof value,
               		          app_ble_on_subscribe, NULL);
    if (rc != 0) {
        APP_LOG_WRN("Failed to subscribe to characteristic; rc=%d\n", rc);
        goto err;
    }

    return;

err:
#if 1
	APP_LOG("\n");
#else
    /* Terminate the connection. */
	APP_LOG_INFO("send disconnect cmd\r\n");
    ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
#endif
}

static void app_ble_sdp_complete_cb(const struct peer *peer, int status, void *arg)
{
    if (status != 0) {
        /* Service discovery failed.  Terminate the connection. */
        APP_LOG_WRN("Service discovery failed; status=%d, conn_handle=%d\n", status, peer->conn_handle);
		APP_LOG_INFO("Send disconnect cmd \r\n");
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

    /* Service discovery has completed successfully.  Now we have a complete
     * list of services, characteristics, and descriptors that the peer
     * supports.
     */
    APP_LOG_INFO("Service discovery complete; status=%d, conn_handle=%d\n", status, peer->conn_handle);

    /* Now perform three concurrent GATT procedures against the peer: read,
     * write, and subscribe to notifications.
     */
    app_ble_read_write_subscribe(peer);
}

static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc out_desc;
    struct ble_hs_adv_fields fields;
    int rc;

    switch (event->type)
	{
    case BLE_GAP_EVENT_DISC:
    #if APP_PRINT_SCANNED_DEV_EN
        APP_LOG_INFO("Device Found peer_addr_type:%d peer_addr: %s\r\n", event->disc.addr.type, addr_to_str(event->disc.addr.val));
    #endif

        rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
        if (rc != 0) {
            break;
        }

        app_ble_connect_create(&event->disc);
        return 0;

    case BLE_GAP_EVENT_CONNECT:
        conn_create_start = 0;

		if(event->connect.status){
			APP_LOG_WRN("Connection failed; status=%d\n", event->connect.status);
			conn_handle = 0xFFFF;
			app_ble_scan_start();
			break;
		}
		conn_handle = event->connect.conn_handle;

		APP_LOG_INFO("Connection established - handle:%d\r\n", conn_handle);
		rc = ble_gap_conn_find(event->connect.conn_handle, &out_desc);
		APP_ASSERT(rc == 0);
        APP_LOG("\t-peer_ota_addr: %s(at: %d)\r\n",
                addr_to_str(out_desc.peer_ota_addr.val),
                out_desc.peer_ota_addr.type);

        APP_LOG("\t-peer_id_addr: %s(at: %d)\r\n",
                 addr_to_str(out_desc.peer_id_addr.val),
                 out_desc.peer_id_addr.type);

		APP_LOG("\t-conn_itvl: %d us\r\n"
                "\t-latency: %d\r\n"
				"\t-timeout: %d ms\r\n",
			     out_desc.conn_itvl*1250,
				 out_desc.conn_latency,
				 out_desc.supervision_timeout*10);

		/* add peer device */
		rc = peer_add(event->connect.conn_handle);
		if (rc != 0) {
			APP_LOG_ERR("Failed to add peer; rc=%d\r\n", rc);
			break;
		}

		/* Perform service discovery(SDP). */
		rc = peer_disc_all(event->connect.conn_handle, app_ble_sdp_complete_cb, NULL);
		if (rc != 0) {
			APP_LOG_ERR("Failed to discover services; rc=%d\r\n", rc);
			break;
		}

        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n", event->disconnect.reason & 0xff);

        /* delete peer. */
        peer_delete(event->disconnect.conn.conn_handle);

		conn_handle = 0xFFFF;
        conn_create_start = 0;

        app_ble_disconnect_notify();

        app_ble_scan_start();

        return 0;

    case BLE_GAP_EVENT_DISC_COMPLETE:
        APP_LOG_INFO("discovery complete; reason=%d\n", event->disc_complete.reason);
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        APP_LOG_INFO("encryption change event; status=%d \r\n",event->enc_change.status);
        return 0;

    case BLE_GAP_EVENT_NOTIFY_RX:
        APP_LOG_INFO("received %s; conn_handle=%d,attr_handle=%d,attr_len=%d\n",
							event->notify_rx.indication ? "indication" : "notification",
							event->notify_rx.conn_handle,
							event->notify_rx.attr_handle,
							OS_MBUF_PKTLEN(event->notify_rx.om));
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
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &out_desc);
        APP_ASSERT(rc == 0);

        ble_store_util_delete_peer(&out_desc.peer_id_addr);

        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    default:
        return 0;
    }

	return 0;
}

static void app_ble_scan_start(void)
{
    struct ble_gap_disc_params disc_params;
    int rc;
	(void)rc;

    disc_params.passive       = 1;
    disc_params.itvl          = BLE_GAP_SCAN_ITVL_MS(60);
    disc_params.window        = BLE_GAP_SCAN_WIN_MS(50);
    disc_params.filter_policy = 0;
    disc_params.limited       = 0;
	disc_params.filter_duplicates = 0;

    rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, app_ble_gap_event, NULL);
    if (rc != 0) {
        APP_LOG_ERR("Error initiating GAP discovery procedure; rc=%d\r\n", rc);
    }

	APP_LOG_INFO("scan starting\r\n");
}

static void app_ble_connect_create(const struct ble_gap_disc_desc *disc)
{
    int rc;

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
	/* Filter adv by rssi */
    if (disc->rssi < -60){
        return;
    }

	/* create connection if key pressed. */
	if(!conn_create_start){
		return;
	}
	conn_create_start = 0;
#endif

    /* Scanning must be stopped before a connection can be initiated. */
    rc = ble_gap_disc_cancel();
    if (rc != 0) {
        APP_LOG_ERR("Failed to cancel scan; rc=%d\n", rc);
        return;
    }

    /* Try to connect the the advertiser.  Allow 30 seconds (30000 ms) for timeout. */
	struct ble_gap_conn_params conn_param = {
		.scan_itvl 		= BLE_GAP_SCAN_ITVL_MS(50),
		.scan_window 	= BLE_GAP_SCAN_ITVL_MS(50),
		.itvl_min 		= BLE_GAP_CONN_ITVL_MS(15),
		.itvl_max 		= BLE_GAP_CONN_ITVL_MS(15),
		.latency 		= 0,
		.supervision_timeout = 500,
		.min_ce_len 	= 0,
		.max_ce_len 	= 0,
	};
    rc = ble_gap_connect(own_addr_type, &disc->addr, 30000, &conn_param, app_ble_gap_event, NULL);
    if (rc != 0) {
        APP_LOG_ERR("Error: Failed to connect to device; addr_type=%d addr=%s\n; rc=%d",
                    disc->addr.type, addr_to_str(disc->addr.val), rc);
        return;
    }
}

void app_ble_disconnect_notify(void)
{
    xSemaphoreGive(semphr_ble_disconnect);
}

void app_msg_queue_init(void)
{
    msgQueue = xQueueCreate(32, sizeof(msg_t));
}

void app_push_queue_msg(msg_t msg)
{
	BaseType_t token = 0;

    if(IN_ISR()){
        xQueueSendToBackFromISR(msgQueue, &msg, &token);
        portYIELD_FROM_ISR(token);
    }
    else{
        xQueueSendToBack(msgQueue, &msg, OS_MAX_DELAY);
    }
}

void app_queue_msg_handler(void)
{
    int rc = 0;
    msg_t msg;
    xQueueReceive(msgQueue, &msg, OS_MAX_DELAY);

    switch(msg.type)
    {
    case APP_MSG_TYPE_DISCONNECT:
        APP_LOG_INFO("Start disconnection - conn_handle:%d\r\n", conn_handle);

        /* reset semphr_ble_disconnect */
        xSemaphoreTake(semphr_ble_disconnect, 0);

        /* send ble terminate cmd. */
        rc = ble_gap_terminate(msg.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        if(rc == 0){
            /* wait ble terminate complete. */
            xSemaphoreTake(semphr_ble_disconnect, OS_MAX_DELAY);
            APP_LOG_INFO("ble disconnect complete\n");
        }
        conn_create_start = 0;
        break;

    default:
        break;
    }
}

void app_key_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_1024);

	//key1
	GPIO_SetMode(P0, BIT(6), GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P0, BIT(6));
	GPIO_EnablePullupPath(P0, BIT(6));
    GPIO_EnableDebounce(P0, BIT(6));
	GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

	//key3(WAKEUP)
	GPIO_SetMode(P0, BIT(2), GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P0, BIT(2));
	GPIO_EnablePullupPath(P0, BIT(2));
    GPIO_EnableDebounce(P0, BIT(2));
	GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);

    NVIC_EnableIRQ(GPIO0_IRQn);
	NVIC_SetPriority(GPIO0_IRQn, 1);
}

void GPIO0_IRQHandlerOverlay(void)
{
	if(GPIO_GetIntFlag(P0,  BIT(6)))
	{
		GPIO_ClrIntFlag(P0, BIT(6));
		if(conn_handle == 0xFFFF){
			if(ble_gap_disc_active()){
				conn_create_start = 1;
				APP_LOG_INFO("Start connection...\r\n");
			}
		}
	}

	if(GPIO_GetIntFlag(P0,  BIT(2)))
	{
		GPIO_ClrIntFlag(P0, BIT(2));

        if(conn_handle != 0xFFFF){
            msg_t msg = { APP_MSG_TYPE_DISCONNECT, conn_handle};
            app_push_queue_msg(msg);
        }
	}
}

void app_ble_user_handler(void)
{
    app_queue_msg_handler();
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

    pan_misc_set_bd_addr(pub_mac);
}

static void app_ble_enabled_cb(void)
{
    int rc;
	(void)rc;

	/* Set random address. */
#if APP_USE_RANDOM_ADDR
    app_ble_set_random_addr();
#endif

    /* print device id address. */
    uint8_t addr[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr, NULL);
	APP_ASSERT(rc == 0);
    APP_LOG_WRN("Device Id Address: %s\r\n", addr_to_str(addr));

	/* Starting Scan. */
    app_ble_scan_start();
}

/**
 * @brief: ble stack pre init call-back
 *         !!!Note: 1. The Bluetooth MAC Address must be set in this callback.
 *                  2. The BLE Services must be registered in this callback
 *                  3. The SDP initialization must be executed int this callback.
 */
void app_ble_pre_init_cb(void)
{
	/* Set MAC address. */
    app_ble_set_mac_addr();

    /* SDP initialization.*/
    int rc = peer_init(APP_MAX_CONN_NUM, 32, 32, 32);
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

	app_key_init();
    semphr_ble_disconnect = xSemaphoreCreateBinary();
    app_msg_queue_init();
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
