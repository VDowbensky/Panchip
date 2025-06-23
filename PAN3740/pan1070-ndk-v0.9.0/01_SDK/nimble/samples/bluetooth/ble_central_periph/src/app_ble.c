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
#include <assert.h>
#include <string.h>

#include "pan_ble.h"
#include "app.h"
#include "app_log.h"

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
#if CONFIG_BT_MAX_NUM_OF_PERIPHERAL
	/* Adv */
	app_ble_adv_start();
#endif

#if CONFIG_BT_MAX_NUM_OF_CENTRAL 
	/* Scan */
	app_ble_scan_start();
#endif
}

/**
 * @brief: ble stack pre init call-back
 *         !!!Note: 1. The Bluetooth MAC Address must be set in this callback.
 *                  2. The BLE Services must be registered in this callback.
 *                  3. The SDP initialization must be executed in this callback.
 */
void app_ble_pre_init_cb(void)
{
	/* Set MAC address. */
    app_ble_set_mac_addr();

	/* BLE Peripheral initilaization. */
	app_ble_periph_init();

	/* BLE Central initilaization. */
	app_ble_central_init();
	
	/* BLE Services initilaization. */
	app_ble_svc_init();
    extern const char *device_name;
    ble_svc_gap_device_name_set(device_name);
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}


