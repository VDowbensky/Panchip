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
#include "PANSeries.h"
#include "pan_ble.h"
#include "app.h"
#include "app_log.h"
#include "utility.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/**@brief Enable security request. */
#define APP_SEND_SEC_REQ_EN              1

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif

uint16_t conn_handle = 0xFFFF;

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static int app_ble_gap_event(struct ble_gap_event *event, void *arg);
static void app_ble_advertise_start(void);


/*******************************************************************************
 * Function Define
 ******************************************************************************/
static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc out_desc;
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
		APP_LOG_INFO("connection %s; status=%d\n", event->connect.status == 0 ? "established" : "failed",
					event->connect.status);

		if (event->connect.status != 0) {
			app_ble_advertise_start();
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

        /* Send Security Request. */
	#if APP_SEND_SEC_REQ_EN
		ble_gap_security_initiate(conn_handle);
	#endif
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        APP_LOG_INFO("disconnect; reason=0x%02x\n", event->disconnect.reason & 0xff);
        conn_handle = 0xFFFF;

        app_ble_advertise_start();
        break;

	case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv complete\n");
        app_ble_advertise_start();
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
		ble_gap_conn_find(event->conn_update.conn_handle, &out_desc);
		APP_LOG_INFO("conn upd cmpl: conn_handle:%d, itvl:%d us, latency:%d, to:%d ms\n",
					out_desc.conn_handle,
					out_desc.conn_itvl * 1250,
					out_desc.conn_latency,
					out_desc.supervision_timeout*10);
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        APP_LOG_INFO("encryption change event; status=%d \r\n", event->enc_change.status);
        break;

    case BLE_GAP_EVENT_NOTIFY_TX:
        APP_LOG_INFO("notify_tx event; conn_handle=%d, attr_handle=%d "
                    "status=%d, is_indication=%d\r\n",
                    event->notify_tx.conn_handle,
                    event->notify_tx.attr_handle,
                    event->notify_tx.status,
                    event->notify_tx.indication);
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        APP_LOG_INFO("subscribe event; conn_handle=%d, attr_handle=%d, "
                    "reason=%d, prevn=%d, curn=%d, previ=%d, curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);
        break;

    case BLE_GAP_EVENT_MTU:
        APP_LOG_INFO("mtu update event; conn_handle=%d mtu=%d\n",
                    event->mtu.conn_handle,
                    event->mtu.value);
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

    case BLE_GAP_EVENT_PASSKEY_ACTION:
	{
        APP_LOG_INFO("PASSKEY_ACTION_EVENT started \n");
        struct ble_sm_io pkey = {0};

        if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
            pkey.action = event->passkey.params.action;
            pkey.passkey = 123456; // This is the passkey to be entered on peer
            APP_LOG_INFO("Enter passkey %u on the peer side\n", pkey.passkey);
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            APP_LOG_INFO("ble_sm_inject_io result: %d\n", rc);
        }
		else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
            APP_LOG_INFO("Passkey on device's display: %u\n" , event->passkey.params.numcmp);
            APP_LOG_INFO("Accept or reject the passkey through console in this format -> key Y or key N\r\n");
            pkey.action = event->passkey.params.action;

         #if 0//MYNEWT_VAL_BLE_SM_MITM
            if (scli_receive_key(&key)) {
                pkey.numcmp_accept = key;
            } else {
                pkey.numcmp_accept = 0;
                APP_LOG_INFO("Timeout! Rejecting the key\r\n");
            }
         #endif

           /*shell shoud support input num here*/

            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            APP_LOG_INFO("ble_sm_inject_io result: %d\r\n", rc);
        }
		else if (event->passkey.params.action == BLE_SM_IOACT_OOB) {
            static uint8_t tem_oob[16] = {0};
            pkey.action = event->passkey.params.action;
            for (int i = 0; i < 16; i++) {
                pkey.oob[i] = tem_oob[i];
            }
            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            APP_LOG_INFO("ble_sm_inject_io result: %d\r\n", rc);
        }
		else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
            APP_LOG_INFO("Enter the passkey through console in this format-> key 123456\r\n");
            pkey.action = event->passkey.params.action;

         #if 0//MYNEWT_VAL_BLE_SM_MITM
            if (scli_receive_key(&key)) {
                pkey.passkey = key;
            } else {
                pkey.passkey = 0;
                APP_LOG_INFO("Timeout! Passing 0 as the key\r\n");
            }
         #endif

            /*shell shoud support input num here*/

            rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
            APP_LOG_INFO("ble_sm_inject_io result: %d\n", rc);
        }
        break;
	}
	default:
		break;
    }

    return 0;
}

static void app_ble_advertise_start(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;
	(void)rc; //remove compiler warning

	/* set advertising data. */
    memset(&fields, 0, sizeof fields);

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(BLE_SVC_ANS_UUID16)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_ERR("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* set advertising parameter. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, app_ble_gap_event, NULL);
    if (rc != 0) {
        APP_LOG_ERR("error enabling advertisement; rc=%d\n", rc);
        return;
    }

	APP_LOG_INFO("adv starting\r\n");
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

	/* Set the default device name. */
    int rc = ble_svc_gap_device_name_set("pan_ble_periph_enc");
    APP_ASSERT(rc == 0);
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
