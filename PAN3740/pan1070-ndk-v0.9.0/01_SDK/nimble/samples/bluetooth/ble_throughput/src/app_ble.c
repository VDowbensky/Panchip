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
#include "utility.h"
#include "soc_api.h"

#include "app_log.h"
#include "app.h"


/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@berif Use random address */
#define APP_USE_RANDOM_ADDR              0

/**@brief Use roll MAC address. */
#define APP_USER_ROLL_MAC_ADDR		     0

/**@brief Test parameter config */
#define APP_TEST_TIME_SEC                10
#define APP_TEST_DATA_LEN 		  		 247
#define APP_MTU_SIZE              		 247

#define APP_TEST_PHY_2M                  0 // 0: 1M PHY; 1: 2M PHY
#if APP_TEST_PHY_2M
#define APP_TEST_CONN_ITVL_MIN           9
#define APP_TEST_CONN_ITVL_MAX           9
#else
#define APP_TEST_CONN_ITVL_MIN           12
#define APP_TEST_CONN_ITVL_MAX           12
#endif

#define APP_SINGLE_DIRECTION_RATE_EN      1

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
static const char *device_name = "pan_ble_throughput";

static SemaphoreHandle_t subscribe_sem;

static uint16_t conn_handle = 0xFFFF;
static bool notify_enable = 0;

static uint8_t  data[APP_TEST_DATA_LEN] = {0};
static uint32_t start_tick = 0;
static uint32_t end_tick = 0;
static uint32_t time_us  = 0;
static uint32_t data_cnt = 0;

#if APP_USE_RANDOM_ADDR
static uint8_t own_addr_type = BLE_ADDR_RANDOM;
#else
static uint8_t own_addr_type = BLE_ADDR_PUBLIC;
#endif

static struct ble_gap_upd_params conn_params = {
    .itvl_min 			 = APP_TEST_CONN_ITVL_MIN,
    .itvl_max 			 = APP_TEST_CONN_ITVL_MAX,
    .latency 			 = 0,
    .supervision_timeout = 400,
    .min_ce_len 		 = 0,
    .max_ce_len 		 = 0,
};

extern uint16_t notify_handle;


/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
static void app_ble_adv_start(void);
static int app_ble_gap_event(struct ble_gap_event *event, void *arg);


/*******************************************************************************
 * Function Define
 ******************************************************************************/
void key_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);

	GPIO_SetMode(P0, BIT(6), GPIO_MODE_INPUT);
	GPIO_EnableDigitalPath(P0, BIT(6));
	GPIO_EnablePullupPath(P0, BIT(6));

	GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

    NVIC_EnableIRQ(GPIO0_IRQn);
}

void GPIO0_IRQHandlerOverlay(void)
{
	if(GPIO_GetIntFlag(P0,  BIT(6)))
	{
		GPIO_ClrIntFlag(P0, BIT(6));
		xSemaphoreGive(subscribe_sem);
		APP_LOG_INFO("Data Send Start...\r\n");
	}
}

void app_ble_data_test(void)
{
	APP_LOG_INFO("wait start data send...\r\n");

	xSemaphoreTake(subscribe_sem, portMAX_DELAY);

	start_tick = 0;
	end_tick = 0;
	time_us = 0;
	data_cnt = 0;

	start_tick = pan_misc_get_cur_sys_tick();
	while(1)
	{
		int rc = ble_svc_throughput_send_notify(conn_handle, data, sizeof data);
		if(rc == 0){
			data_cnt++;
		}
		else{
			OS_DelayMs(OS_MS_TO_TICK(5));
		}
		end_tick = pan_misc_get_cur_sys_tick();

		time_us = (uint32_t)(end_tick - start_tick) / soc_32k_clock_freq_get() * 1000;
		if(time_us >= APP_TEST_TIME_SEC*1000){
			break;
		}

		if(notify_enable == false){
			break;
		}
	}

	if (data_cnt) {
		APP_LOG("\n*********************************************************\n");
		APP_LOG_INFO("Notify throughput = %d Kbps, count = %d",
				  (data_cnt * (APP_TEST_DATA_LEN-3) * 8) / APP_TEST_TIME_SEC / 1024, data_cnt);
		APP_LOG("\n*********************************************************\n");
		APP_LOG_INFO("Notification test complete for stipulated time of %d sec\n\n", APP_TEST_TIME_SEC);
	}
	else{
		APP_LOG_WRN("data test parameter invalid, please check parameter...\n");
	}
}

static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
	{
    case BLE_GAP_EVENT_CONNECT:
		if(event->connect.status)
		{
			APP_LOG_WRN("Connection failed; status=%d\n", event->connect.status);

			conn_handle = 0xffff;
			app_ble_adv_start();

			return 0;
		}

		/* Connection successfully established. */
		APP_LOG_INFO("=== Connection established ===\n");

		/* Start MTU Update. */
        rc = ble_att_set_preferred_mtu(APP_MTU_SIZE);
        if(rc != 0) {
            APP_LOG_ERR("Failed to set preferred MTU; rc = %d", rc);
        }

		rc = ble_gattc_exchange_mtu(event->connect.conn_handle, NULL, NULL);
        if(rc != 0) {
            APP_LOG_ERR("Failed to update params; rc = %d", rc);
        }


        conn_handle = event->connect.conn_handle;

		/* update connetion parameter */
		ble_gap_update_params(conn_handle, &conn_params);

        break;

    case BLE_GAP_EVENT_DISCONNECT:
		APP_LOG_INFO("=== Disconnect Cmpl ===\n -reason:0x%02x \n", event->disconnect.reason);
		conn_handle = 0xffff;
        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_CONN_UPDATE:
		ble_gap_conn_find(event->conn_update.conn_handle, &desc);
		APP_LOG_INFO("conn upd cmpl: conn_handle:%d, itvl:%d us, latency:%d, to:%d ms\n",
						desc.conn_handle,
						desc.conn_itvl * 1250,
						desc.conn_latency,
						desc.supervision_timeout*10);

	#if APP_TEST_PHY_2M
		ble_gap_set_prefered_le_phy(conn_handle,BLE_GAP_LE_PHY_2M_MASK,BLE_GAP_LE_PHY_2M_MASK,BLE_GAP_LE_PHY_CODED_ANY);
	#endif
        return 0;

	case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:
		APP_LOG_INFO("PHY update cmpl\r\n\t-tx_phy:%d\n", event->phy_updated.tx_phy);
		return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("adv duration expired\r\n");
        app_ble_adv_start();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        //APP_LOG_INFO("subscribe event; cur_notify=%d; value handle; "
        //        "val_handle = %d\r\n",
        //         event->subscribe.cur_notify, event->subscribe.attr_handle);

        if(event->subscribe.attr_handle == notify_handle)
		{
            notify_enable = event->subscribe.cur_notify;

			if(notify_enable) {
				xSemaphoreGive(subscribe_sem);
				APP_LOG_INFO("Data Send Start ...\n");
			}
        }
        break;

    case BLE_GAP_EVENT_NOTIFY_TX:
        if ((event->notify_tx.status == 0) || (event->notify_tx.status == BLE_HS_EDONE)) {
            /* Send new notification i.e. give Semaphore. By definition,
             * sending new notifications should not be based on successful
             * notifications sent, but let us adopt this method to avoid too
             * many `BLE_HS_ENOMEM` errors because of continuous transfer of
             * notifications.XXX */
            //xSemaphoreGive(notify_sem);
        }
        break;

    case BLE_GAP_EVENT_MTU:
		APP_LOG_INFO("MTU update event \r\n" "\t-conn_handle=%d\n" "\t-cid=%d\n" "\t-mtu=%d\n",
	                  event->mtu.conn_handle, event->mtu.channel_id, event->mtu.value);

        break;
    
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
        break;
    }
    return 0;
}

static void app_ble_adv_start(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

	/* Set Adv Data */
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_ERR("Error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* start advertising */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, app_ble_gap_event, NULL);
    if (rc != 0) {
        APP_LOG_ERR("Error enabling advertisement; rc=%d\n", rc);
        return;
    }

	APP_LOG_INFO("adv starting...\n");
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

#if APP_SLV_TEST_EN
	memcpy(pub_mac, &addrFilterTable[APP_SLV_DEVICE_ID], 6);
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

	/* Set Random Addr */
#if APP_USE_RANDOM_ADDR
	app_ble_set_random_addr();
#endif

    uint8_t addr[6] = {0};

	/* Print Local Id address. */
    rc = ble_hs_id_copy_addr(own_addr_type, addr, NULL);
    APP_ASSERT(rc == 0);
    APP_LOG_INFO("Device ID Address: ");
    APP_LOG_DATA(addr, 6);

    /* Start advertising */
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
	int rc = 0;
	(void)rc;

	/* Set MAC address. */
    app_ble_set_mac_addr();

	/* BLE Service Register */
    app_ble_svc_init();

	/* Set the default device name */
    rc = ble_svc_gap_device_name_set(device_name);
    APP_ASSERT(rc == 0);

#if APP_SINGLE_DIRECTION_RATE_EN
	extern void pan_misc_set_ll_rx_time_enable(uint8_t en);
	pan_misc_set_ll_rx_time_enable(0);
#endif

	subscribe_sem = xSemaphoreCreateCounting(1, 0);
	key_init();
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
