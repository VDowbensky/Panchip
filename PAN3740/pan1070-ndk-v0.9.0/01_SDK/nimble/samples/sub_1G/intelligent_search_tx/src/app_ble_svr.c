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
#include "app_ble_svr.h"
#include "pan_ble.h"
#include "app.h"
#include "app_log.h"

uint16_t sub_1g_handle;
uint16_t sub_1g_handle_1;

static int gatt_svr_chr_access_trx(uint16_t conn_handle, uint16_t attr_handle,
				   struct ble_gatt_access_ctxt *ctxt, void *arg);


static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
	{
		/* Service: Heart-rate */
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID128_DECLARE(TRX_SERVICE_UUID),
		.characteristics = (struct ble_gatt_chr_def[]) { {
									 /* Characteristic: C2003 characteristic */
									 .uuid = BLE_UUID128_DECLARE(TRX_SERVICE_UUID1),
									 .access_cb = gatt_svr_chr_access_trx,
									 .val_handle = &sub_1g_handle,
									 .flags = BLE_GATT_CHR_F_NOTIFY,
								 },
								 {
									 /* Characteristic: C2002 characteristic */
									 .uuid = BLE_UUID128_DECLARE(TRX_SERVICE_UUID2),
									 .access_cb = gatt_svr_chr_access_trx,
									 .val_handle = &sub_1g_handle_1,
									 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
								 },
								 {
									 0, /* No more characteristics in this service */
								 }, }
	},

	{
		0, /* No more services */
	},
};

static int gatt_svr_chr_access_trx(uint16_t conn_handle, uint16_t attr_handle,
				   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	/* Sensor location, set to "Chest" */
	int rc;
	uint16_t om_len;
	uint32_t buf[20] = { 0, };

	if (attr_handle == sub_1g_handle) {
		return 0; /* do not implement read and write */
	} else if (attr_handle == sub_1g_handle_1) {

		om_len = OS_MBUF_PKTLEN(ctxt->om);

		if (om_len > 128 / 4) {
			om_len = 128 / 4;
		}

		rc = ble_hs_mbuf_to_flat(ctxt->om, buf, sizeof(buf), &om_len);
		extern void trx_param_change(uint32_t period_time);
		trx_param_change(buf[0]);
		if (rc != 0) {
			return BLE_ATT_ERR_UNLIKELY;
		}
	}

	return rc;
}

int gatt_svr_init(void)
{
	int rc;

	rc = ble_gatts_count_cfg(gatt_svr_svcs);
	if (rc != 0) {
		return rc;
	}

	rc = ble_gatts_add_svcs(gatt_svr_svcs);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

void ble_svc_sub_1g_init(void)
{
    int rc;

    rc = gatt_svr_init();
    assert(rc == 0);
}




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
        break;
    }
}

void app_ble_svc_init(void)
{
	/* Register DIS service */
	ble_svc_gap_init();

	/* Register HRS service */
	ble_svc_sub_1g_init();

	/* register call-back */
	ble_hs_cfg.gatts_register_cb  = app_ble_svr_register_cb;
	ble_hs_cfg.gatts_register_arg = NULL;
}
