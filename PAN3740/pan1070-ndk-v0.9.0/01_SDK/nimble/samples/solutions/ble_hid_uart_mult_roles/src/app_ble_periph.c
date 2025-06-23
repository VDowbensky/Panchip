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
#include <assert.h>
#include <string.h>

#include "app.h"
#include "pan_ble.h"
#include "uart_AT.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/


/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
extern char device_name[];

uint16_t peri_conn_handle;
uint8_t uart_spp_tx_notify_enabled = 0;


/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static int app_ble_periph_gap_event(struct ble_gap_event *event, void *arg);


/*******************************************************************************
 * Function Define
 ******************************************************************************/
void app_ble_adv_start(void)
{
	struct ble_gap_adv_params adv_params;
	struct ble_hs_adv_fields fields;
	int rc;
	uint8_t own_addr_type = 0;

	memset(&fields, 0, sizeof(fields));

	fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

	fields.name = (uint8_t *)g_fmc_config_data.device_name;
	fields.name_len = g_fmc_config_data.name_length;
	fields.name_is_complete = 1;

	fields.uuids128 = (ble_uuid128_t[]) {
		BLE_UUID128_INIT(BT_UUID_CUSTOM_SERVICE_VAL_ARG)
	};
	fields.num_uuids128 = 1;
	fields.uuids128_is_complete = 1;

	rc = ble_gap_adv_set_fields(&fields);
	if (rc != 0) {
		APP_LOG_ERR("setting advertisement data error; rc=%d\n", rc);
		return;
	}

	/* set advertising parameter. */
	memset(&adv_params, 0, sizeof(adv_params));
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

	/* Get local id address type. */
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	APP_ASSERT(rc == 0);

	/* start adv. */
	rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, app_ble_periph_gap_event, NULL);
	if(rc != 0) {
		APP_LOG_ERR("Adv Start failed; rc=%d\n", rc);
		return;
	}

	APP_LOG_INFO("adv start...\r\n");
}

void app_ble_periph_notify(uint8_t *data, uint8_t len)
{
	struct os_mbuf *om;

	if (!uart_spp_tx_notify_enabled) {
		APP_LOG_WRN("periph notify_state: %s\n", uart_spp_tx_notify_enabled ? "enabled":"disabled");
		return;
	}

	om = ble_hs_mbuf_from_flat(data, len);
	if(om){
		ble_gatts_notify_custom(peri_conn_handle, uart_spp_tx_handle, om);
	}
}

static int app_ble_periph_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc desc;
    struct ble_sm_io pkey = {0};
	int rc = 0;

	switch (event->type)
	{
	case BLE_GAP_EVENT_CONNECT:
		APP_LOG_INFO("connection %s; status=%d\n",
		       event->connect.status == 0 ? "established" : "failed",
		       event->connect.status);

		if (event->connect.status != 0) {
			APP_LOG_ERR("Peripheral Connection failed; status=%d\n", event->connect.status);
			app_ble_adv_start();
			peri_conn_handle = 0xFFFF; //invalid handle
			break;
		}

		peri_conn_handle = event->connect.conn_handle;

		APP_LOG_INFO("Peripheral Connection established \r\n");
		rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
		APP_ASSERT(rc == 0);
		APP_LOG("\t-conn_intvl:%d us\r\n\t-latency:%d\r\n\t-to:%d ms\r\n",
			     desc.conn_itvl*1250, desc.conn_latency, desc.supervision_timeout*10);

		/* Start MTU Update */
		rc = ble_gattc_exchange_mtu(event->connect.conn_handle, NULL, NULL);
		if(rc){
			APP_LOG_WRN("MTU requeset failed...\r\n");
		}
		break;

	case BLE_GAP_EVENT_DISCONNECT:
		APP_LOG_INFO("disconnect; reason=0x%02x\n",  (uint8_t)event->disconnect.reason);
		peri_conn_handle = 0xFFFF; // invalid handle

		app_ble_adv_start();
		break;

	case BLE_GAP_EVENT_ADV_COMPLETE:
		APP_LOG_INFO("adv duration expired - restart adv\n");
		app_ble_adv_start();
		break;

	case BLE_GAP_EVENT_SUBSCRIBE:
		APP_LOG_INFO("subscribe event; cur_notify=%d; val_handle=%d\n",
		       event->subscribe.cur_notify, event->subscribe.attr_handle);

		if(uart_spp_tx_handle == event->subscribe.attr_handle){
			uart_spp_tx_notify_enabled = event->subscribe.cur_notify;
		}
		break;

	case BLE_GAP_EVENT_MTU:
		APP_LOG_INFO("mtu update event; conn_handle=%d mtu=%d\n",
		       event->mtu.conn_handle,
		       event->mtu.value);
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        APP_LOG_INFO("prph encryt change; status=%d\n", event->enc_change.status);
		break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
        if (event->passkey.params.action == BLE_SM_IOACT_DISP)
		{
            pkey.action  = event->passkey.params.action;
            pkey.passkey = g_fmc_config_data.passkey; // This is the passkey to be entered on peer
            APP_LOG_INFO("Enter passkey %" PRIu32 " on the peer side\n", pkey.passkey);

            int rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            APP_LOG_INFO("ble_sm_inject_io result: %d\n", rc);
        }
		break;

	default:
		break;
	}

	return 0;
}

void app_ble_periph_init(void)
{

}
