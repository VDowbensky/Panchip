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

#include "accelerometer.h"
#include "pan_hal.h"
#include "tft_lcd_128x160.h"

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
/*Connection handle */
static uint16_t conn_handle = 0xFFFF;

/**@brief BLE device name. */
static const char *device_name = "b+spi lcd";

static bool notify_state = 0;

static uint8_t mu_data[] = {0xD1, 0x07, 0xc9, 0x7a, 0xbb, 0x8f, 0xdd, 0x4b, 0x00, 0x11};

#define ADV_UUID    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x20, 0x40, 0x6e


TimerHandle_t conn_upd_timer;

#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
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

/* This functions simulates heart beat and notifies it to the client */
void notify_data(uint8_t *data, uint16_t len)
{
    int rc;
    struct os_mbuf *om;

	if (notify_state) {
		om = ble_hs_mbuf_from_flat(data, len);

		rc = ble_gatts_notify_custom(conn_handle, c2003_handle, om);
		assert(rc == 0);
	}
}

static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;

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

        notify_state = event->subscribe.cur_notify;
        break;

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
    int rc;
	(void)rc; //remove compiler warning

	/* set advertising data. */
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

	fields.mfg_data = mu_data;
	fields.mfg_data_len = sizeof(mu_data);
	fields.mfg_data_len = sizeof(mu_data);

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_ERR("setting advertisement data failed; rc=%d\n", rc);
        APP_ASSERT(rc == 0);
    }

	memset(&fields, 0, sizeof(fields));

	fields.uuids128 = (ble_uuid128_t[]) {
		BLE_UUID128_INIT(ADV_UUID)
	};
	fields.num_uuids128 = 1;
	fields.name_is_complete = 1;

	rc = ble_gap_adv_rsp_set_fields(&fields);
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
	init_tft_lcd_128x160();

#if APP_CONN_UPD_EN
	app_ble_conn_upd_timer_init();
#endif
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
