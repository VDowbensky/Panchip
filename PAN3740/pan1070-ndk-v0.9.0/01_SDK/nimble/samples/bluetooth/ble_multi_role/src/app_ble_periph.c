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

void app_ble_adv_start(void)
{
	if(ble_gap_adv_active()){
		return;
	}

    int rc;
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    //struct ble_hs_adv_fields rsp_fields;

    /* set adv parameters */
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min  = ADV_INTR_30_MS;
    adv_params.itvl_max  = ADV_INTR_35_MS;

    /* Set Adv Data */
    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN|BLE_HS_ADV_F_BREDR_UNSUP;

	const char *device_name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    APP_ASSERT(rc == 0);

#if 0
    /* Set Scan Rsp Data */
    memset(&rsp_fields, 0, sizeof(rsp_fields));
    rsp_fields.name = (uint8_t *)device_name;
    rsp_fields.name_len = strlen(device_name);
    rsp_fields.name_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&fields);
    APP_ASSERT(rc == 0);
#endif

    /* As own address type we use hard-coded value, because we generate
       NRPA and by definition it's random */
    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                           &adv_params, app_ble_gap_event_cb, NULL);
    APP_ASSERT(rc == 0);

	APP_LOG_INFO("Starting advertising...\n");
}

void app_ble_periph_init(void)
{

}
