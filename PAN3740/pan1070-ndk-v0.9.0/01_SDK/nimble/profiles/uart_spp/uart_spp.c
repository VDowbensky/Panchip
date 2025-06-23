/**
 *******************************************************************************
 * @file     uart_spp.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "uart_spp.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"

uint16_t uart_tx_handle;

int ble_svc_spp_uart_write(uint8_t *p, uint32_t len);
static int ble_svc_chr_access_spp(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_uart_spp_defs[] = 
{
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(BLE_SVC_SPP_UUID128),
        .characteristics = (struct ble_gatt_chr_def[]) 
        { 
            {
                .uuid = BLE_UUID128_DECLARE(BLE_SVC_SPP_CHR_UUID128_RX),
                .access_cb = ble_svc_chr_access_spp,
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
            }, 
            {
                .uuid = BLE_UUID128_DECLARE(BLE_SVC_SPP_CHR_UUID128_TX),
                .access_cb = ble_svc_chr_access_spp,
                .val_handle = &uart_tx_handle,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            }, {
                0, /* No more characteristics in this service */
            }, 
        }
    },{
		0, /* No more services */
	},
};

static int ble_svc_chr_access_spp(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	uint8_t data[256] = {0};
    volatile uint16_t uuid;
	uint16_t om_len;
    int rc;

	if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
		uuid = ble_uuid_u16(ctxt->chr->uuid);
		om_len = OS_MBUF_PKTLEN(ctxt->om);
		rc = ble_hs_mbuf_to_flat(ctxt->om, data, sizeof(data), &om_len);
		if (rc != 0) {
			assert(0);
			return BLE_ATT_ERR_UNLIKELY;
		} 

        /* UART send data */
		ble_svc_spp_uart_write(data, om_len);
	} 
    else if(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
		uuid = ble_uuid_u16(ctxt->chr->uuid);
	}

	return 0;
}

__WEAK int ble_svc_spp_uart_write(uint8_t *p, uint32_t len)
{
    return 0;
}

int ble_svc_spp_send_notify(uint16_t conn_handle, uint8_t *pdata, uint32_t len)
{
    int rc = 0;

    struct os_mbuf *om = ble_hs_mbuf_from_flat(pdata, len);
    if(om == NULL){
        return BLE_HS_ENOMEM;
    }

    rc = ble_gatts_notify_custom(conn_handle, uart_tx_handle, om);
    return rc;
}

int ble_svc_spp_init(void)
{
    int rc = 0;

    rc = ble_gatts_count_cfg(ble_svc_uart_spp_defs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(ble_svc_uart_spp_defs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
