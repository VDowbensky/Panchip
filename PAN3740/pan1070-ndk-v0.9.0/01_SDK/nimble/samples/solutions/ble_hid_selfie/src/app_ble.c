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

#include <string.h>
#include <stdio.h>

#include "PANSeries.h"
#include "pan_ble.h"

#include "app_log.h"
#include "app.h"

#include "pan_hal.h"
#include "hid.h"


/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/**@brief CONN Param Update. */
#define APP_CONN_UPD_EN                  0

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
/* Connection handle */
uint16_t conn_handle;

static const char *device_name = "PAN Selfie";

static uint8_t own_addr_type;

void ble_store_config_init(void);

TimerHandle_t conn_upd_timer;

#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif

#if MYNEWT_VAL(BLE_POWER_CONTROL)
static struct ble_gap_event_listener power_control_event_listener;
#endif


/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
 // BLE API
static int app_ble_gap_event(struct ble_gap_event *event, void *arg);
static void app_ble_advertise_start(void);

// connection parameter update API
void app_ble_conn_upd_timer_start(void);

/*******************************************************************************
 * Function Define
 ******************************************************************************/

/**
 * Logs information about a connection to the console.
 */

static void print_addr(uint8_t* addr)
{
    for(uint8_t i=0; i<6; i++)
    {
        APP_LOG("%02x ", addr[i]);
    }
    APP_LOG("\n");
}

static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    APP_LOG_INFO("handle=%d our_ota_addr_type=%d our_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    APP_LOG_INFO(" our_id_addr_type=%d our_id_addr=",
                desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    APP_LOG_INFO(" peer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    APP_LOG_INFO(" peer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    APP_LOG_INFO(" conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

#if MYNEWT_VAL(BLE_POWER_CONTROL)
static void app_ble_power_control(uint16_t conn_handle)
{
    int rc;

    rc = ble_gap_read_remote_transmit_power_level(conn_handle, 0x01 );  // Attempting on LE 1M phy
    assert (rc == 0);

    rc = ble_gap_set_transmit_power_reporting_enable(conn_handle, 0x1, 0x1);
    assert (rc == 0);
}
#endif


#if MYNEWT_VAL(BLE_POWER_CONTROL)
static int
app_ble_power_event(struct ble_gap_event *event, void *arg)
{

    switch(event->type) {
    case BLE_GAP_EVENT_TRANSMIT_POWER:
	APP_LOG_INFO("Transmit power event : status=%d conn_handle=%d reason=%d "
                          "phy=%d power_level=%x power_level_flag=%d delta=%d",
		    event->transmit_power.status,
		    event->transmit_power.conn_handle,
		    event->transmit_power.reason,
		    event->transmit_power.phy,
		    event->transmit_power.transmit_power_level,
		    event->transmit_power.transmit_power_level_flag,
		    event->transmit_power.delta);
	return 0;

    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
	APP_LOG_INFO("Pathloss threshold event : conn_handle=%d current path loss=%d "
                          "zone_entered =%d",
		    event->pathloss_threshold.conn_handle,
		    event->pathloss_threshold.current_path_loss,
		    event->pathloss_threshold.zone_entered);
	return 0;

    default:
	return 0;
    }
}
#endif

static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;
	int rc;

    switch (event->type)
	{
    case BLE_GAP_EVENT_CONNECT:
		APP_LOG_INFO("connection %s; status=%d\n", event->connect.status == 0 ? "established" : "failed",
					event->connect.status);

		if (event->connect.status != 0) {
			app_ble_advertise_start();
			conn_handle = 0xFFFF;
			break;
		}

		ble_gap_conn_find(event->conn_update.conn_handle, &out_desc);
		APP_LOG_INFO("\t-conn_intvl:%d us\r\n\t-latency:%d\r\n\t-to:%d ms\r\n",
			               out_desc.conn_itvl*1250, out_desc.conn_latency, out_desc.supervision_timeout*10);

		conn_handle = event->connect.conn_handle;
		ble_gattc_exchange_mtu(event->connect.conn_handle, NULL, NULL);
        if (event->connect.status != 0) {
            /* Connection failed; resume advertising. */
            app_ble_advertise_start();
            conn_handle = 0;
        } else {
            conn_handle = event->connect.conn_handle;
        }

	#if MYNEWT_VAL(BLE_POWER_CONTROL)
		app_ble_power_control(event->connect.conn_handle);

		app_ble_event_listener_register(&power_control_event_listener,
											bleprph_gap_power_event, NULL);
	#endif

	#if APP_CONN_UPD_EN
		app_ble_conn_upd_timer_start();
    #endif
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n", (uint8_t)event->disconnect.reason & 0xff);
        conn_handle = 0xFFFF;

        app_ble_advertise_start();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv duration expired - restart adv\n");
        app_ble_advertise_start();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        APP_LOG_INFO("subscribe event; cur_notify=%d, val_handle=%d\n",
                    event->subscribe.cur_notify, conn_handle);
        break;

	case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        APP_LOG_INFO("encryption change event; status=%d ",
                    event->enc_change.status);
        rc = ble_gap_conn_find(event->enc_change.conn_handle, &out_desc);
        assert(rc == 0);
        bleprph_print_conn_desc(&out_desc);
        APP_LOG_INFO("\n");
        return 0;

	case BLE_GAP_EVENT_NOTIFY_TX:
		APP_LOG_INFO("notify ok\n");
		break;

    case BLE_GAP_EVENT_MTU:
        APP_LOG_INFO("mtu update event; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
		break;

	case BLE_GAP_EVENT_CONN_UPDATE:
	{
		ble_gap_conn_find(event->conn_update.conn_handle, &out_desc);
		APP_LOG_INFO("conn upd cmpl: conn_handle:%d, itvl:%d us, latency:%d, to:%d ms\n",
					out_desc.conn_handle,
					out_desc.conn_itvl * 1250,
					out_desc.conn_latency,
					out_desc.supervision_timeout*10);
        break;
	}

	case BLE_GAP_EVENT_REPEAT_PAIRING:
	{
        /* We already have a bond with the peer, but it is attempting to
         * establish a new secure link.  This app sacrifices security for
         * convenience: just throw away the old bond and accept the new link.
         */

        /* Delete the old bond. */
        rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &out_desc);
        assert(rc == 0);
        ble_store_util_delete_peer(&out_desc.peer_id_addr);

        /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
         * continue with the pairing operation.
         */
        return BLE_GAP_REPEAT_PAIRING_RETRY;
	}

    case BLE_GAP_EVENT_PASSKEY_ACTION:
	{
        APP_LOG_INFO("PASSKEY_ACTION_EVENT started \n");
        struct ble_sm_io pkey = {0};

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            pkey.action = event->passkey.params.action;
            pkey.passkey = 123456; // This is the passkey to be entered on peer
            APP_LOG_INFO("Enter passkey %" PRIu32 "on the peer side", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            APP_LOG_INFO("ble_sm_inject_io result: %d\n", rc);
        } else if (event->passkey.params.action == BLE_SM_IOACT_INPUT ||
                  event->passkey.params.action == BLE_SM_IOACT_NUMCMP ||
                  event->passkey.params.action == BLE_SM_IOACT_OOB ) {
            APP_LOG_INFO("BLE_SM_IOACT_INPUT, BLE_SM_IOACT_NUMCMP, BLE_SM_IOACT_OOB"
                          " bonding not supported!");
        }
        return 0;
	}
	default:
		break;
    }

    return 0;
}

/*******************************************************************************
 * Connection parameter update
 ******************************************************************************/
void app_ble_conn_upd_timer_cb(TimerHandle_t xTimer)
{
	struct ble_gap_upd_params upd_params = {
		.itvl_min = BLE_GAP_CONN_ITVL_MS(CONFIG_CONN_INTRVL_MS),
		.itvl_max = BLE_GAP_CONN_ITVL_MS(CONFIG_CONN_INTRVL_MS),
		.latency  = 0,
		.supervision_timeout = 500,
	};

    ble_gap_update_params(conn_handle, &upd_params);
}


void app_ble_conn_upd_timer_start(void)
{
	xTimerStart(conn_upd_timer, 0);
}

void app_ble_conn_upd_timer_init(void)
{
	conn_upd_timer = xTimerCreate("conn_upd_timer", pdMS_TO_TICKS(2000), pdFALSE, 0, app_ble_conn_upd_timer_cb);
}

static void app_ble_advertise_start(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    struct ble_hs_adv_fields rsp_fields;
    const char *name;
    int rc;
	(void)rc; //remove compiler warning

	/* set advertising data. */
    memset(&fields, 0, sizeof(fields));
	memset(&rsp_fields, 0, sizeof(rsp_fields));

	/* Set the default device name. */
    rc = ble_svc_gap_device_name_set(device_name);
    ble_svc_gap_device_appearance_set(0x03C0);

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

	fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(BT_UUID_HIDS)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

     /*
     * Set appearance.
     */
    fields.appearance = ble_svc_gap_device_appearance();
    fields.appearance_is_present = 1;

    rsp_fields.name = (uint8_t *)device_name;
    rsp_fields.name_len = strlen(device_name);
    rsp_fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
	if (rc != 0) {

        APP_LOG_ERR("error sd setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* set advertising parameter. */
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

	adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(CONFIG_ADV_INTRVL_MS);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(CONFIG_ADV_INTRVL_MS + 100);

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
	                       app_ble_gap_event, NULL);
    if (rc != 0) {
        APP_LOG_ERR("enabling advertisement failed; rc=%d\n", rc);
        return;
    }
	APP_LOG_INFO("BLE adv start...\n");
}

#if APP_USE_RANDOM_ADDR
static void app_ble_set_random_addr(void)
{
    int rc = 0;
    ble_addr_t addr;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    APP_ASSERT(rc == 0);

    /* set generated address */
    rc = ble_hs_id_set_rnd(addr.val);
    APP_ASSERT(rc == 0);
}
#endif


static void app_ble_set_mac_addr(void)
{
    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};

#if CONFIG_USER_CHIP_MAC_ADDR
	extern uint8_t pan10x_mac_addr_get(uint8_t *mac);
	pan10x_mac_addr_get(pub_mac);
#endif

#if APP_USER_ROLL_MAC_ADDR
	extern uint8_t pan10x_roll_mac_addr_get(uint8_t *mac);
	pan10x_roll_mac_addr_get(pub_mac);
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

#if APP_USE_RANDOM_ADDR
    /* Set random address. */
    app_ble_set_random_addr();
#endif

    /* print device id address. */
    uint8_t addr[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr, NULL);
	APP_ASSERT(rc == 0);
    APP_LOG_WRN("Device Id Address: ");
    APP_LOG_DATA(addr, 6);

    /* start advertising */
    app_ble_advertise_start();
}

/**
 * @brief: ble stack pre init call-back
 *         !!!Note: 1. The Bluetooth MAC Address must be set in this callback.
 *                  2. The BLE Services must be registered in this callback.
 *                  3. the SDP initialzation must be executed in this callback.
 */
void app_ble_pre_init_cb(void)
{
	/* Set MAC address. */
    app_ble_set_mac_addr();

	/* BLE Service Register */
    app_ble_svc_init();

	/* BLE Application */

#if APP_CONN_UPD_EN
	app_ble_conn_upd_timer_init();
#endif
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
