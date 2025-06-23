/**
 *******************************************************************************
 * @file     app_ble_svr.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#pragma diag_suppress 111

#include "app_ble_svr.h"
#include "pan_ble.h"
#include "app.h"
#include "accelerometer.h"
#include "app_log.h"

void app_ble_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        APP_LOG_INFO("registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        APP_LOG_INFO("  registering characteristic %s with "
                           "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        APP_LOG_INFO("    registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        APP_LOG_ERR("Unrecognized GATT Register OP Code: %d\n", ctxt->op);
    }
}

void app_ble_svc_init(void)
{
	/* Register DIS service */
	ble_svc_gap_init();

	/* Register HRS service */
	ble_svc_accelerometer_init();

	/* register call-back */
	ble_hs_cfg.gatts_register_cb  = app_ble_svr_register_cb;
	ble_hs_cfg.gatts_register_arg = NULL;
}
