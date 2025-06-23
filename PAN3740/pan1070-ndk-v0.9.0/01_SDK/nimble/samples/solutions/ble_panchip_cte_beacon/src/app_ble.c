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

static const char *adv_name = "PANCHIP-CTE Beacon";

#define COPMANY_ID	0xD1, 0x07 /* Panchip */
#define PACKET_ID	0x01
#define DEVICE_TYPE	0x20 /* Device type */
#define DF_FIELD	0x67, 0xF7, 0xDB, 0x34, 0xC4, 0x03, 0x8E, 0x5C, 0x0B, 0xAA, 0x97, 0x30, 0x56, 0xE6

/* Advertising data */
static uint8_t manuf_data[] = {
	COPMANY_ID,
	PACKET_ID,
	DEVICE_TYPE,
	/* Carries information of Tag's TX rate, TX power and ID type */
	0x15,

	/* Tag ID */
	0xD2, 0x12, 0x03, 0x00, 0x02, 0x23,
	/* Checksum */
	0xC9,

	DF_FIELD
};

static uint8_t own_addr_type;

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
static void ble_cte_beacon_advertise(void);

/*******************************************************************************
 * Function Define
 ******************************************************************************/


static int app_ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {

    case BLE_GAP_EVENT_ADV_COMPLETE:
        APP_LOG_INFO("advertise complete; reason=%d",
                    event->adv_complete.reason);
        ble_cte_beacon_advertise();
        break;
    default:
        break;
    }

	return 0;
}

static void ble_cte_beacon_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;
	(void)rc;

	/* set advertising data. */
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

	fields.mfg_data = manuf_data;
    fields.mfg_data_len = sizeof(manuf_data);

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_INFO("error setting advertisement data; rc=%d\n", rc);
        return;
    }

    memset(&fields, 0, sizeof fields);
    fields.name = (uint8_t *)adv_name;
    fields.name_len = strlen(adv_name);
    fields.name_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&fields);
    if (rc != 0) {
        APP_LOG_INFO("error setting advertisement rsp data; rc=%d\n", rc);
        return;
    }

    /* set advertising parameter. */
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
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
    ble_cte_beacon_advertise();
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
