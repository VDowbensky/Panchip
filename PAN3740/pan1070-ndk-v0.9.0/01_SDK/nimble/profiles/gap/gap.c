#include <string.h>
#include "gap.h"
#include "host/ble_hs.h"
#include "pan_sys_log.h"

static ble_svc_gap_chr_changed_fn *ble_svc_gap_chr_changed_cb_fn;

static char ble_svc_gap_name[BLE_SVC_GAP_NAME_MAX_LEN + 1] = "PN_BLE";
static uint16_t ble_svc_gap_appearance = BLE_SVC_GAP_APPEARANCE_GEN_HID;

static int ble_svc_gap_access(uint16_t conn_handle, uint16_t attr_handle,
                                struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_gap_defs[] = {
    {
        /*** Service: GAP. */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_GAP_UUID16),
        .characteristics = (struct ble_gatt_chr_def[])
		{
        	{
				/*** Characteristic: Device Name. */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_GAP_CHR_UUID16_DEVICE_NAME),
				.access_cb = ble_svc_gap_access,
				.flags = BLE_GATT_CHR_F_READ|BLE_GATT_CHR_F_WRITE,
                .val_handle = NULL,
        	},
			{
				/*** Characteristic: Appearance. */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_GAP_CHR_UUID16_APPEARANCE),
				.access_cb = ble_svc_gap_access,
				.flags = BLE_GATT_CHR_F_READ,
				.val_handle = NULL,
			},
		#if BLE_SVC_GAP_PERIPH_PREF_CONN_PARAM_EN
			{
				/*** Characteristic: Peripheral Preferred Connection Parameters. */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_GAP_CHR_UUID16_PERIPH_PREF_CONN_PARAMS),
				.access_cb = ble_svc_gap_access,
				.flags = BLE_GATT_CHR_F_READ,
			},
		#endif
		#if BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION
			{
				/*** Characteristic: Central Address Resolution. */
				.uuid = BLE_UUID16_DECLARE(BLE_SVC_GAP_CHR_UUID16_CENTRAL_ADDRESS_RESOLUTION),
				.access_cb = ble_svc_gap_access,
				.flags = BLE_GATT_CHR_F_READ,
			},
		#endif
			{
				0, /* No more characteristics in this service. */
			}
		},
    },
    {
        0, /* No more services. */
    },
};

static int ble_svc_gap_device_name_read_access(struct ble_gatt_access_ctxt *ctxt)
{
    int rc;

    rc = os_mbuf_append(ctxt->om, ble_svc_gap_name, strlen(ble_svc_gap_name));

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int ble_svc_gap_device_name_write_access(struct ble_gatt_access_ctxt *ctxt)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(ctxt->om);
    if (om_len > BLE_SVC_GAP_NAME_MAX_LEN) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(ctxt->om, ble_svc_gap_name, om_len, NULL);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    ble_svc_gap_name[om_len] = '\0';

    if (ble_svc_gap_chr_changed_cb_fn) {
        ble_svc_gap_chr_changed_cb_fn(BLE_SVC_GAP_CHR_UUID16_DEVICE_NAME);
    }

    return rc;
}

static int ble_svc_gap_appearance_read_access(struct ble_gatt_access_ctxt *ctxt)
{
    uint16_t appearance = htole16(ble_svc_gap_appearance);
    int rc;

    rc = os_mbuf_append(ctxt->om, &appearance, sizeof(appearance));

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int ble_svc_gap_appearance_write_access(struct ble_gatt_access_ctxt *ctxt)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(ctxt->om);
    if (om_len != sizeof(ble_svc_gap_appearance)) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(ctxt->om, &ble_svc_gap_appearance, om_len, NULL);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    ble_svc_gap_appearance = le16toh(ble_svc_gap_appearance);

    if (ble_svc_gap_chr_changed_cb_fn) {
        ble_svc_gap_chr_changed_cb_fn(BLE_SVC_GAP_CHR_UUID16_APPEARANCE);
    }

    return rc;
}

static int ble_svc_gap_access(uint16_t conn_handle, uint16_t attr_handle,
                   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t uuid16;

#if BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION
    uint8_t central_ar = BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION;
#endif

#if BLE_SVC_GAP_PERIPH_PREF_CONN_PARAM_EN
    uint16_t ppcp[4] = {
        htole16(BLE_SVC_GAP_PPCP_MIN_CONN_INTERVAL),
        htole16(BLE_SVC_GAP_PPCP_MAX_CONN_INTERVAL),
        htole16(BLE_SVC_GAP_PPCP_SLAVE_LATENCY),
        htole16(BLE_SVC_GAP_PPCP_SUPERVISION_TMO)
    };
#endif

    int rc;

    uuid16 = ble_uuid_u16(ctxt->chr->uuid);
    SYS_ASSERT(uuid16 != 0);

    switch (uuid16)
    {
    case BLE_SVC_GAP_CHR_UUID16_DEVICE_NAME:
        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            rc = ble_svc_gap_device_name_read_access(ctxt);
        } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            rc = ble_svc_gap_device_name_write_access(ctxt);
        } else {
            SYS_ERR("Unrecognized GATT Access OP Code: %d\n", ctxt->op);
            rc = BLE_ATT_ERR_UNLIKELY;
        }
        return rc;

    case BLE_SVC_GAP_CHR_UUID16_APPEARANCE:
        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            rc = ble_svc_gap_appearance_read_access(ctxt);
        } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            rc = ble_svc_gap_appearance_write_access(ctxt);
        } else {
            SYS_ERR("Unrecognized GATT Access OP Code: %d\n", ctxt->op);
            rc = BLE_ATT_ERR_UNLIKELY;
        }
        return rc;

#if BLE_SVC_GAP_PERIPH_PREF_CONN_PARAM_EN
    case BLE_SVC_GAP_CHR_UUID16_PERIPH_PREF_CONN_PARAMS:
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);
        rc = os_mbuf_append(ctxt->om, &ppcp, sizeof(ppcp));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
#endif

#if BLE_SVC_GAP_CENTRAL_ADDRESS_RESOLUTION
    case BLE_SVC_GAP_CHR_UUID16_CENTRAL_ADDRESS_RESOLUTION:
        assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);
        rc = os_mbuf_append(ctxt->om, &central_ar, sizeof(central_ar));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
#endif

    default:
        assert(0);
        return BLE_ATT_ERR_UNLIKELY;
    }
}

const char *ble_svc_gap_device_name(void)
{
    return ble_svc_gap_name;
}

int ble_svc_gap_device_name_set(const char *name)
{
    int len;

    len = strlen(name);
    if (len > BLE_SVC_GAP_NAME_MAX_LEN) {
        return BLE_HS_EINVAL;
    }

    memcpy(ble_svc_gap_name, name, len);
    ble_svc_gap_name[len] = '\0';

    return 0;
}

uint16_t ble_svc_gap_device_appearance(void)
{
  return ble_svc_gap_appearance;
}

int ble_svc_gap_device_appearance_set(uint16_t appearance)
{
    ble_svc_gap_appearance = appearance;

    return 0;
}

void ble_svc_gap_set_chr_changed_cb(ble_svc_gap_chr_changed_fn *cb)
{
    ble_svc_gap_chr_changed_cb_fn = cb;
}

void ble_svc_gap_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(ble_svc_gap_defs);
    SYS_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_gap_defs);
    SYS_ASSERT(rc == 0);
}
