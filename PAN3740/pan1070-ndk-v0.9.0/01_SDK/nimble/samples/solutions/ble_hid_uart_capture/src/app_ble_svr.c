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
#include "hid.h"

#include "uart_AT.h"

uint16_t uart_spp_tx_handle;
uint16_t ff00_handle;
uint16_t ff80_handle;

static int gatt_svr_chr_access_uart_notify(uint16_t conn_handle, uint16_t attr_handle,
										   struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_uart_read_write(uint16_t conn_handle, uint16_t attr_handle,
												struct ble_gatt_access_ctxt *ctxt, void *arg);

static int
gatt_svr_chr_access_ff80(uint16_t conn_handle, uint16_t attr_handle,
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
				.uuid = BLE_UUID16_DECLARE(0x00DC),
				.access_cb = gatt_svr_chr_access_uart_read_write,
				.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
			},
			{
				.uuid = BLE_UUID16_DECLARE(0x00C8),
				.access_cb = gatt_svr_chr_access_uart_notify,
				.val_handle = &uart_spp_tx_handle,
				.flags = BLE_GATT_CHR_F_NOTIFY,
			},
			{
				0, /* No more characteristics in this service */
			},
		}
    },

	{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xff80),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(0xff82),
            .access_cb = gatt_svr_chr_access_ff80,
			.val_handle = &ff80_handle,
            .flags = BLE_GATT_CHR_F_WRITE_NO_RSP  | BLE_GATT_CHR_F_WRITE,
        }, {
            .uuid = BLE_UUID16_DECLARE(0xff81),
			.access_cb = gatt_svr_chr_access_uart_notify,
			.val_handle = &ff80_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        }, {
            0, /* No more characteristics in this service */
        }, }
	},

		{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xff00),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(0xff02),
			.access_cb = gatt_svr_chr_access_ff80,
			.val_handle = &ff00_handle,
            .flags = BLE_GATT_CHR_F_WRITE_NO_RSP  | BLE_GATT_CHR_F_WRITE,
        }, {
            .uuid = BLE_UUID16_DECLARE(0xff01),
			.access_cb = gatt_svr_chr_access_uart_notify,
			.val_handle = &ff00_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        },{
            .uuid = BLE_UUID16_DECLARE(0xff03),
			.access_cb = gatt_svr_chr_access_uart_notify,
			.val_handle = &ff00_handle,
            .flags = BLE_GATT_CHR_F_NOTIFY,
        },{
            0, /* No more characteristics in this service */
        }, }
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

    TGT_SendMultiData(rx_data, OS_MBUF_PKTLEN(ctxt->om));

    return 0;
}

static int
gatt_svr_chr_access_ff80(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    /* Sensor location, set to "Chest" */
	uint8_t rx_data[256];
	uint16_t rx_len;

	const uint8_t version_data[] = {0x41, 0x54, 0x2B, 0x47, 0x46, 0x57, 0x56, 0x45, 0x52, 0x3F, 0x0D, 0x0A};
	const uint8_t addr_data[] = {0x41, 0x54, 0x2B, 0x4C, 0x42, 0x44, 0x41, 0x44, 0x44, 0x52, 0x3F, 0x0D};

	rx_len = OS_MBUF_PKTLEN(ctxt->om);
	os_mbuf_copydata(ctxt->om, 0, rx_len, rx_data);

	if(!memcmp(rx_data, version_data, rx_len)) {
		ff81_notify((uint8_t *)"\r\nBR2141e-s(A02)_DH_210104_r5723\r\n", strlen("\r\nBR2141e-s(A02)_DH_210104_r5723\r\n"));

		vTaskDelay(100);

		ff81_notify((uint8_t *)"\r\nOK\r\n", strlen("\r\nOK\r\n"));
	} else if(!memcmp(rx_data, addr_data, rx_len)) {
		char str[50];

		extern uint8_t m_chip_mac[6];
		sprintk(str, "\r\n+LBDADDR:%x%x%x%x%x%x\r\n",
		m_chip_mac[0], m_chip_mac[1], m_chip_mac[2], m_chip_mac[3], m_chip_mac[4], m_chip_mac[5]);
		ff81_notify((uint8_t *)str, strlen(str));

		vTaskDelay(100);
		ff81_notify((uint8_t *)"\r\nOK\r\n", strlen("\r\nOK\r\n"));
	}

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
	ble_svc_gap_init();

	ble_svc_gatt_init();

	/* Register UART SPP service */
	ble_svc_uart_spp_init();

	ble_svc_hid_init();

	/* register call-back */
	ble_hs_cfg.gatts_register_cb  = app_ble_svr_register_cb;
	ble_hs_cfg.gatts_register_arg = NULL;
}
