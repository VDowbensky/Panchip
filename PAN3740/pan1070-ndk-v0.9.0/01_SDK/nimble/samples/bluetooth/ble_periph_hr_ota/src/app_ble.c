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
#include "app.h"
#include "PANSeries.h"

#include <string.h>
#include <stdio.h>

#include "pan_ble.h"
#include "utility.h"
#include "app_log.h"

#include "pan_smp.h"


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
/**@brief Connection handle */
static uint16_t conn_handle = 0xFFFF;
static const char *device_name = "pan_ble_ota";

#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif


/*******************************************************************************
 * Function Declaration
 ******************************************************************************/


/*******************************************************************************
 * Function Define
 ******************************************************************************/
int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;
    int rc = 0;
    
    switch (event->type)
	{
    case BLE_GAP_EVENT_CONNECT:
		APP_LOG_INFO("connection %s; status=%d\n", event->connect.status == 0 ? "established" : "failed",
					event->connect.status);

        if (event->connect.status != 0) {
            app_ble_adv_start();
            conn_handle = 0xFFFF;
			break;
        }

        conn_handle = event->connect.conn_handle;

		ble_gap_conn_find(conn_handle, &out_desc);
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
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n",  event->disconnect.reason & 0xFF);
        conn_handle = 0xFFFF;

        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv duration expired - restart adv\n");
        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        APP_LOG_INFO("subscribe event; cur_notify=%d, val_handle=%d\n",
                    event->subscribe.cur_notify, hrs_hrm_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        APP_LOG_INFO("mtu update event, -conn_handle=%d mtu=%d\n",
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

void app_ble_adv_start(void)
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


    /* Start Advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(30);
    adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(35);

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, app_ble_gap_event, NULL);
    if(rc != 0){
        APP_LOG_ERR("error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

#if APP_USE_RANDOM_ADDR
static void app_ble_set_random_addr(void)
{
    int rc = 0;
    ble_addr_t addr;

    /* generate new non-resolvable private address */
    rc = ble_hs_id_gen_rnd(0, &addr);
    app_app_assert(rc == 0);

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
	/* Set MAC address initialization. */
    app_ble_set_mac_addr();

	/* BLE Service initialization. */
    app_ble_svc_init();
    ble_svc_gap_device_name_set(device_name);

	/* DFU initialization. */
    pan_dfu_init();
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
