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
#include <string.h>
#include "app_ble_svr.h"
#include "pan_ble.h"

#include "app.h"
#include "app_log.h"

/**@brief ble security enable */
#define APP_BLE_SEC_EN   1

/*** Maximum number of characteristics with the notify flag ***/
#define MAX_NOTIFY 5

static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/* A characteristic that can be subscribed to */
static uint8_t gatt_svr_chr_val;
static uint16_t gatt_svr_chr_val_handle;
static const ble_uuid128_t gatt_svr_chr_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

/* A custom descriptor */
static uint8_t gatt_svr_dsc_val;
static const ble_uuid128_t gatt_svr_dsc_uuid =
    BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);

static int app_ble_svc_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_custom_def[] =
{
    {
        /*** Custom Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {
			{
                /*** This characteristic can be subscribed to by writing 0x00 and 0x01 to the CCCD ***/
                .uuid = &gatt_svr_chr_uuid.u,
                .access_cb = app_ble_svc_access_cb,
			#if APP_BLE_SEC_EN
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
						 BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_WRITE_ENC |
						 BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE,
			#else
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE,
			#endif
                .val_handle = &gatt_svr_chr_val_handle,
                .descriptors = (struct ble_gatt_dsc_def[])
				 {
					{
					  .uuid = &gatt_svr_dsc_uuid.u,
					#if APP_BLE_SEC_EN
					  .att_flags = BLE_ATT_F_READ | BLE_ATT_F_READ_ENC,
					#else
					  .att_flags = BLE_ATT_F_READ,
					#endif
					  .access_cb = app_ble_svc_access_cb,
					},
					{
						0, /* No more descriptors in this characteristic */
					}
				 },
            },
			{
                0, /* No more characteristics in this service. */
            }
        },
    },
    {
        0, /* No more services. */
    },
};

static int app_ble_custom_svr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
									void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}


static int app_ble_svc_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid_t *uuid;
    int rc;

    switch (ctxt->op)
	{
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            APP_LOG_INFO("Characteristic read; conn_handle=%d attr_handle=%d\n", conn_handle, attr_handle);
        } else {
            APP_LOG_WRN("Characteristic read by NimBLE stack; attr_handle=%d\n", attr_handle);
        }

        uuid = ctxt->chr->uuid;
        if (attr_handle == gatt_svr_chr_val_handle) {
            rc = os_mbuf_append(ctxt->om, &gatt_svr_chr_val, sizeof(gatt_svr_chr_val));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            APP_LOG_INFO("Characteristic write; conn_handle=%d attr_handle=%d", conn_handle, attr_handle);
        } else {
            APP_LOG_WRN("Characteristic write by NimBLE stack; attr_handle=%d", attr_handle);
        }

        uuid = ctxt->chr->uuid;
        if (attr_handle == gatt_svr_chr_val_handle) {
            rc = app_ble_custom_svr_write(ctxt->om, sizeof(gatt_svr_chr_val), sizeof(gatt_svr_chr_val),
                                &gatt_svr_chr_val, NULL);

            ble_gatts_chr_updated(attr_handle);

            APP_LOG_INFO("Notification/Indication scheduled for all subscribed peers.\n");
            return rc;
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            APP_LOG_INFO("Descriptor read; conn_handle=%d attr_handle=%d\n", conn_handle, attr_handle);
        } else {
            APP_LOG_WRN("Descriptor read by NimBLE stack; attr_handle=%d\n", attr_handle);
        }

        uuid = ctxt->dsc->uuid;

        if (ble_uuid_cmp(uuid, &gatt_svr_dsc_uuid.u) == 0) {
            rc = os_mbuf_append(ctxt->om, &gatt_svr_dsc_val, sizeof(gatt_svr_chr_val));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto unknown;

    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        goto unknown;

    default:
        goto unknown;
    }

unknown:
    /* Unknown characteristic/descriptor;
     * The NimBLE host should not have called this function;
     */
    APP_LOG_ERR("Unrecognized GATT Access OP Code: %d\n", ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
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
        APP_LOG_INFO("  -registering characteristic %s with "
                    "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        APP_LOG_INFO("    *registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        APP_LOG_ERR("Unrecognized GATT Register OP Code: %d\n", ctxt->op);
    }
}

static int ble_svc_custom_init(void)
{
	int rc;

    rc = ble_gatts_count_cfg(ble_svc_custom_def);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(ble_svc_custom_def);
    if (rc != 0) {
        return rc;
    }

	/* Setting a value for the read-only descriptor */
    gatt_svr_dsc_val = 0x99;

	return rc;
}

void app_ble_svc_init(void)
{
	/* Register GAP service */
    ble_svc_gap_init();

	/* Register GATT service */
    ble_svc_gatt_init();

	/* Register ANS service */
    ble_svc_ans_init();

	/* Register user custom service */
	ble_svc_custom_init();

	/* Register service call-back */
	ble_hs_cfg.gatts_register_cb = app_ble_svr_register_cb;
	ble_hs_cfg.gatts_register_arg = NULL;
}
