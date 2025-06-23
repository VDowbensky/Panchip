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

#include "uart_AT.h"

uint16_t uart_spp_tx_handle;

static int gatt_svr_chr_access_uart_notify(uint16_t conn_handle, uint16_t attr_handle,
										   struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_uart_read_write(uint16_t conn_handle, uint16_t attr_handle,
												struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_uart_spp_svr_defs[] =
{
    {
        /* Service: Panchip UART SPP service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BT_UUID_CUSTOM_SERVICE_VAL,
        .characteristics = (struct ble_gatt_chr_def[])
		{
			{
				.uuid = BT_UUID_CUSTOM_CHARAC_VAL1,
				.access_cb = gatt_svr_chr_access_uart_notify,
				.val_handle = &uart_spp_tx_handle,
				.flags = BLE_GATT_CHR_F_NOTIFY,
			},
			{
				.uuid = BT_UUID_CUSTOM_CHARAC_VAL2,
				.access_cb = gatt_svr_chr_access_uart_read_write,
				.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
			},
			{
				0, /* No more characteristics in this service */
			},
		}
    },
	{
		0, /* No more services */
	},
};

static int gatt_svr_chr_access_uart_notify(uint16_t conn_handle, uint16_t attr_handle,
											struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	return 0;
}

static int gatt_svr_chr_access_uart_read_write(uint16_t conn_handle, uint16_t attr_handle,
												struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint8_t rx_data[256];
	uint16_t rx_len;

	rx_len = OS_MBUF_PKTLEN(ctxt->om);
	os_mbuf_copydata(ctxt->om, 0, rx_len, rx_data);

    APP_LOG_INFO("BLE Rx data - len: %d\n", rx_len);

//	APP_LOG_DATA(rx_data, rx_len);
//  TGT_SendMultiData("Cent Notify:", sizeof("Cent Notify:") - 1);

    TGT_SendMultiData(rx_data, OS_MBUF_PKTLEN(ctxt->om));
    return 0;

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
        APP_LOG_INFO("  -registering characteristic %s with def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        APP_LOG_INFO("    --registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        APP_LOG_ERR("Unrecognized GATT Register OP Code: %d\n", ctxt->op);
    }
}

int ble_svc_uart_spp_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(ble_svc_uart_spp_svr_defs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(ble_svc_uart_spp_svr_defs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

void app_ble_svc_init(void)
{
	/* Register DIS service */
	ble_svc_gap_init();

	/* Register UART SPP service */
	ble_svc_uart_spp_init();

	/* register call-back */
	ble_hs_cfg.gatts_register_cb  = app_ble_svr_register_cb;
	ble_hs_cfg.gatts_register_arg = NULL;
}
