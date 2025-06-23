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

#include "PanSeries.h"
#include "utility.h"
#include "pan_ble.h"

#include "rf_test.h"

#include "app_log.h"
#include "app.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/**@brief Enable non-adv */
#define APP_NON_CONN_ADV_EN              0

/**@brief Enable PHY Update */
#define APP_PHY_UPD_EN                   0

/**@brief Power Test mode */
#if CONFIG_POWER_TEST_EN
#define APP_CONN_UPD_EN                  1
#endif

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
/**@brief Connection handle */
static uint16_t conn_handle = 0xFFFF;

/**@brief BLE device name. */
static const char *device_name = "ble_hr";

/**@brief Heart-rate data send timer. */
static struct ble_npl_callout  hr_tx_timer;

/**@brief Variable to simulate heart rate */
static uint8_t heartrate = 90;

static bool notify_state = 0;

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

// hr data tx API
static void app_ble_hr_tx_timer_start(void);
static void app_ble_hr_tx_timer_stop(void);
static void app_ble_hr_tx_timer_cb(struct ble_npl_event *ev);

// connection parameter update API
void app_ble_conn_upd_timer_start(void);

/*******************************************************************************
 * Function Define
 ******************************************************************************/
static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble_gap_conn_desc out_desc;
    int rc= 0;

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

	#if APP_CONN_UPD_EN
		app_ble_conn_upd_timer_start();
    #endif

	#if APP_PHY_UPD_EN
		//ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_1M_MASK, BLE_GAP_LE_PHY_1M_MASK, BLE_GAP_LE_PHY_CODED_ANY);
		//ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_2M_MASK, BLE_GAP_LE_PHY_CODED_ANY);
		ble_gap_set_prefered_le_phy(conn_handle, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_ANY);
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
                    event->subscribe.cur_notify, hrs_hrm_handle);

		if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_WRITE)
		{
			if (event->subscribe.attr_handle == hrs_hrm_handle) {
				notify_state = event->subscribe.cur_notify;
				app_ble_hr_tx_timer_start();
			}
			else if (event->subscribe.attr_handle != hrs_hrm_handle) {
				notify_state = event->subscribe.cur_notify;
				app_ble_hr_tx_timer_stop();
			}
		}
		// indicate user that connection terminate.
		else if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_TERM){
			notify_state = false;
			app_ble_hr_tx_timer_stop();
		}
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
 * Connection parameter update
 ******************************************************************************/
void app_ble_conn_upd_timer_cb(TimerHandle_t xTimer)
{
#if CONFIG_CONN_LATENCY_EN
	#if CONFIG_CONN_INTRVL_MS == 100
		struct ble_gap_upd_params upd_params = {
			.itvl_min = BLE_GAP_CONN_ITVL_MS(10),
			.itvl_max = BLE_GAP_CONN_ITVL_MS(10),
			.latency  = 9,
			.supervision_timeout = 500,
		};
	#elif CONFIG_CONN_INTRVL_MS == 1000
		struct ble_gap_upd_params upd_params = {
			.itvl_min = BLE_GAP_CONN_ITVL_MS(10),
			.itvl_max = BLE_GAP_CONN_ITVL_MS(10),
			.latency  = 99,
			.supervision_timeout = 500,
		};
	#endif
#else
	struct ble_gap_upd_params upd_params = {
		.itvl_min = BLE_GAP_CONN_ITVL_MS(CONFIG_CONN_INTRVL_MS),
		.itvl_max = BLE_GAP_CONN_ITVL_MS(CONFIG_CONN_INTRVL_MS),
		.latency  = 0,
		.supervision_timeout = 500,
	};
#endif

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

    rc = ble_npl_callout_reset(&hr_tx_timer, configTICK_RATE_HZ);
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

#if 0
	//static uint8_t dataBuf[3] = {0x11, 0x22, 0x33,}; // 16B
	static uint8_t dataBuf[18] = {0x11, 0x22, 0x33, 0x44, 0x55,}; // 31B

	fields.mfg_data = dataBuf;
	fields.mfg_data_len = sizeof(dataBuf);
#endif

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_ERR("setting advertisement data failed; rc=%d\n", rc);
        APP_ASSERT(rc == 0);
    }

    /* set advertising parameter. */
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
#if APP_NON_CONN_ADV_EN
	 adv_params.conn_mode = 0;
	 adv_params.disc_mode = 0;
#endif

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
    ble_svc_gap_device_name_set(device_name);
    ble_svc_gap_device_appearance_set(0x340);

	/* BLE Application */
	app_ble_hr_tx_timer_init();

#if APP_CONN_UPD_EN
	app_ble_conn_upd_timer_init();
#endif
    
#if CONFIG_PA_ENABLE 
    pan_misc_set_pa_enable(1);
#endif
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
