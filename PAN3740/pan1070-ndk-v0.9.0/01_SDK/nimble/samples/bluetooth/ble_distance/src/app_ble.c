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
#include "app.h"
#include "PANSeries.h"
#include "pan_ble.h"
#include "utility.h"
#include "app_log.h"
#include "app_ui.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
const char *device_name = "ble_distance";
uint16_t conn_handle = 0xFFFF;
bool notify_state = false;

/* Sending notify data timer */
static struct ble_npl_callout 	hr_tx_timer;

/* Variable to simulate heart beats */
static uint8_t heartrate = 90;

#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static void app_ble_adv_start(void);


/*******************************************************************************
 * Function Define
 ******************************************************************************/
void app_ble_hr_tx_timer_stop(void)
{
    ble_npl_callout_stop(&hr_tx_timer);
}

void app_ble_hr_tx_timer_start(void)
{
    int rc;
    rc = ble_npl_callout_reset(&hr_tx_timer, OS_MS_TO_TICK(1000));
    APP_ASSERT(rc == 0);
}

void app_ble_hr_tx_cb(struct ble_npl_event *ev)
{
    static uint8_t hrm[2];
    int rc;

    if(!notify_state) {
        app_ble_hr_tx_timer_stop();
        heartrate = 90;
        return;
    }

    hrm[0] = 0x06;
    hrm[1] = heartrate;

    heartrate++;
    if (heartrate == 160) {
        heartrate = 90;
    }

    rc = ble_svc_hrs_notify(conn_handle, hrm, sizeof(hrm));
    APP_ASSERT(rc == 0);

    app_ble_hr_tx_timer_start();
}

void app_ble_hr_tx_timer_init(void)
{
	ble_npl_callout_init(&hr_tx_timer, nimble_port_get_dflt_eventq(), app_ble_hr_tx_cb, NULL);
}


static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;
    int rc = 0;

    switch (event->type)
	{
    case BLE_GAP_EVENT_CONNECT:
		if(event->connect.status){
			app_ble_adv_start();
            conn_handle = 0xFFFF;
			break;
		}

        APP_LOG_INFO("===Connection Cmpl===\n");

		ble_gap_conn_find(event->conn_update.conn_handle, &out_desc);
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

		conn_handle = event->connect.conn_handle;

        break;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n", event->disconnect.reason & 0xFF);
        conn_handle = 0xFFFF;

		app_ble_hr_tx_timer_stop();

        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv duration expired - restart adv\r\n");
        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        APP_LOG_INFO("subscribe event; cur_notify=%d; value handle; val_handle=%d\n",
                      event->subscribe.cur_notify, hrs_hrm_handle);

		if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_WRITE){
			if(event->subscribe.attr_handle == hrs_hrm_handle) {
				notify_state = event->subscribe.cur_notify;

				if(notify_state){
					app_ble_hr_tx_timer_start();
				}
				else{
					app_ble_hr_tx_timer_stop();
				}
			}
		}
		else if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_TERM){
			// indicate user connection terminate.
		}
        break;

    case BLE_GAP_EVENT_MTU:
        APP_LOG_INFO("mtu update event; peri_conn_handle=%d; mtu=%d\n",
                      event->mtu.conn_handle, event->mtu.value);
        break;

	case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:
		APP_LOG_INFO("PHY update cmpl; status:%d; txPhy=%d; rxPhy=%d\n",
					event->phy_updated.status,
					event->phy_updated.tx_phy,
					event->phy_updated.rx_phy);
		break;
    
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
		break;
    }
    return 0;
}

static void app_ble_adv_start(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if(rc != 0){
        APP_LOG_ERR("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, app_ble_gap_event, NULL);
    if(rc != 0){
        APP_LOG_ERR("error enabling advertisement; rc=%d\n", rc);
        return;
    }
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
    APP_LOG_WRN("Device Id Address: %s\r\n", addr_to_str(addr));

    /* start advertising */
    app_ble_adv_start();
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

	/* BLE Services initialization. */
	app_ble_svc_init();

	/* BLE Application initialization. */
	app_ui_init();

	app_ble_hr_tx_timer_init();
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
