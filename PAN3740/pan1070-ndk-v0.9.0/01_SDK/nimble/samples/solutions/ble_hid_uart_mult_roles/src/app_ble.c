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
#include "pan_ble.h"

#include "uart_AT.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/


/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
char device_name[40] = "pan_spp";


/*******************************************************************************
 * Function Define
 ******************************************************************************/
static void app_ble_set_mac_addr(void)
{
    /** set public address*/
    uint8_t pub_mac[6]={8,2,3,4,5,6};

#if CONFIG_USER_CHIP_MAC_ADDR
	pan10x_mac_addr_get(pub_mac);
#endif

	memcpy(pub_mac, g_fmc_config_data.own_mac, 6);
	memcpy(device_name, g_fmc_config_data.device_name, sizeof(g_fmc_config_data.device_name));

    pan_misc_set_bd_addr(pub_mac);
}

/**
 * @brief: BLE Stack Enabled call-back.
 */
static void app_ble_enabled_cb(void)
{
	ble_att_set_preferred_mtu(247);

#if 1
	app_ble_adv_start();
#endif

#if 0
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

	/* Set the default device name. */
	int rc = ble_svc_gap_device_name_set(device_name);
	APP_ASSERT(rc == 0);
}

void app_ble_init(void)
{
	/* BLE Stack Initialization. */
	pan_ble_stack_init(app_ble_pre_init_cb, app_ble_enabled_cb);
}
