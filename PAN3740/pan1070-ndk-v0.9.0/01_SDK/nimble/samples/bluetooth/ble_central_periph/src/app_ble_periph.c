/**
 *******************************************************************************
 * @file     app_ble_periph.c
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

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
/**@brief Connection handle */
static uint16_t conn_handle = 0xFFFF;

/**@brief BLE device name. */
const char *device_name = "pan_ble_cent_periph";

/**@brief Heart-rate data send timer. */
static struct ble_npl_callout  hr_tx_timer;

/**@brief Variable to simulate heart rate */
static uint8_t heartrate = 90;

static bool notify_state = 0;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
// hr data tx API
static void app_ble_hr_tx_timer_start(void);
static void app_ble_hr_tx_timer_stop(void);
static void app_ble_hr_tx_timer_cb(struct ble_npl_event *ev);


/*******************************************************************************
 * Function Define
 ******************************************************************************/
static int app_ble_periph_gap_event_cb(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;
    int rc = 0;
    
    switch (event->type)
	{
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        APP_LOG_INFO("Peripheral connection %s; status=%d\n",
                    event->connect.status == 0 ? "established" : "failed",
                    event->connect.status);

		if (event->connect.status != 0) {
			app_ble_adv_start();
			conn_handle = 0xFFFF;
			break;
		}

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
        APP_LOG_INFO("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
        conn_handle = 0xFFFF; // invalid conn_handle

        /* Connection terminated; resume advertising */
        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv duration expired - restart adv\n");
        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        APP_LOG_INFO("subscribe event; cur_notify=%d value handle; val_handle=%d\n",
                    event->subscribe.cur_notify, hrs_hrm_handle);

		if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_WRITE)
		{
			if (event->subscribe.attr_handle == hrs_hrm_handle) {
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
			notify_state = false;
			app_ble_hr_tx_timer_stop();
		}
        break;

    case BLE_GAP_EVENT_MTU:
        APP_LOG_INFO("mtu update event; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
        break;

	case BLE_GAP_EVENT_NOTIFY_TX:
		APP_LOG_INFO("send notify ok\n");
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

/*******************************************************************************
 * HeartRate Notify
 ******************************************************************************/
static void app_ble_hr_tx_timer_stop(void)
{
    ble_npl_callout_stop(&hr_tx_timer);
}

static void app_ble_hr_tx_timer_start(void)
{
    int rc;
	(void)rc;

    rc = ble_npl_callout_reset(&hr_tx_timer, pdMS_TO_TICKS(1000));
    APP_ASSERT(rc == 0);
}

static void app_ble_hr_tx_timer_cb(struct ble_npl_event *ev)
{
    static uint8_t hrm[2];
    int rc;
	(void)rc;

    if (!notify_state) {
        app_ble_hr_tx_timer_stop();
        heartrate = 90;
        return;
    }

    hrm[0] = 0x06;      /* contact of a sensor */
    hrm[1] = heartrate; /* storing dummy data */

    /* Simulation of heart beats */
    heartrate++;
    if (heartrate == 160) {
        heartrate = 90;
    }

	rc = ble_svc_hrs_notify(conn_handle, hrm, sizeof(hrm));
	if(rc){
		APP_LOG_WRN("hrs send notify failed - rc:%d\n", rc);
	}

	/* restart timer */
    app_ble_hr_tx_timer_start();
}

static void app_ble_hr_tx_timer_init(void)
{
	ble_npl_callout_init(&hr_tx_timer, nimble_port_get_dflt_eventq(),
                          app_ble_hr_tx_timer_cb, NULL);
}

void app_ble_adv_start(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

	/* Set Adv Data */
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_ERR("error setting advertisement data; rc=%d\n", rc);
        return;
    }

	/* Figure out address to use while advertising (no privacy for now) */
	uint8_t own_addr_type = 0;
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        APP_LOG_ERR("error determining address type; rc=%d\n", rc);
        return;
    }

    /* Start Adv */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
	                       app_ble_periph_gap_event_cb, NULL);
    if (rc != 0) {
        APP_LOG_ERR("error enabling advertisement; rc=%d\n", rc);
        return;
    }

	APP_LOG_INFO("Adv Start...\n");
}

void app_ble_periph_init(void)
{
	app_ble_hr_tx_timer_init();
}
