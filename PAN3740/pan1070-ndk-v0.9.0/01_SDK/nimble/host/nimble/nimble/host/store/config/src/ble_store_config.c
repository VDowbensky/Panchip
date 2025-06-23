/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <inttypes.h>
#include <string.h>

#include "sysinit/sysinit.h"
#include "nimble_syscfg.h"
#include "host/ble_hs.h"
#include "ble_store_config_priv.h"
#include "os/panchip_mempool.h"
#if MYNEWT_VAL_BLE_STORE_CONFIG_PERSIST
#include "kv_store_api.h"
#endif

#if SMP_MULTI_ADDR_MODE_EN
#include "host/ble_hs_id.h"
#endif

#define BLE_STORE_LOG_EN 	0

#ifdef IP_101x

struct ble_store_value_sec *ble_store_config_our_secs;
int ble_store_config_num_our_secs;

struct ble_store_value_sec *ble_store_config_peer_secs;
int ble_store_config_num_peer_secs;

struct ble_store_value_cccd *ble_store_config_cccds;
int ble_store_config_num_cccds;

#else
struct ble_store_value_sec
    ble_store_config_our_secs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
int ble_store_config_num_our_secs;

struct ble_store_value_sec
    ble_store_config_peer_secs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
int ble_store_config_num_peer_secs;

struct ble_store_value_cccd
    ble_store_config_cccds[MYNEWT_VAL(BLE_STORE_MAX_CCCDS)];
int ble_store_config_num_cccds;
#endif

#if SMP_MULTI_ADDR_MODE_EN
struct ble_store_value_sec *ble_store_config_get_our_secs(uint8_t id)
{
    return &ble_store_config_our_secs[id];
}

struct ble_store_value_sec *ble_store_config_get_peer_secs(uint8_t id)
{
    return &ble_store_config_peer_secs[id];
}

struct ble_store_value_cccd *ble_store_config_get_cccds(uint8_t id)
{
    return &ble_store_config_cccds[id];
}

int  ble_store_config_get_our_secs_num(void)
{
    return ble_store_config_num_our_secs;
}

int ble_store_config_get_peer_secs_num(void)
{
    return ble_store_config_num_peer_secs;
}

int ble_store_config_get_cccds_num(void)
{
    return ble_store_config_num_cccds;
}
#endif

/*****************************************************************************
 * $sec                                                                      *
 *****************************************************************************/
#if BLE_STORE_LOG_EN
void
ble_hs_log_flat_buf_priv(const void *data, int len)
{
    const uint8_t *u8ptr;
    int i;

    u8ptr = data;
    for (i = 0; i < len; i++) {
        SYS_PRINT( "%02x", u8ptr[i]);
    }
}
#endif

static void
ble_store_config_print_value_sec(const struct ble_store_value_sec *sec)
{
#if BLE_STORE_LOG_EN
    if (sec->ltk_present) {
        SYS_PRINT( "ediv=%u rand=%llu authenticated=%d ltk= ",
                       sec->ediv, sec->rand_num, sec->authenticated);
        ble_hs_log_flat_buf_priv(sec->ltk, 16);
        SYS_PRINT(" ");
    }
    if (sec->irk_present) {
        SYS_PRINT("irk= ");
        ble_hs_log_flat_buf_priv(sec->irk, 16);
        SYS_PRINT(" ");
    }
    if (sec->csrk_present) {
        SYS_PRINT( "csrk= ");
        ble_hs_log_flat_buf_priv(sec->csrk, 16);
        SYS_PRINT(" ");
    }
    SYS_PRINT( "\n");
#endif
}

static void
ble_store_config_print_key_sec(const struct ble_store_key_sec *key_sec)
{
    if (ble_addr_cmp(&key_sec->peer_addr, BLE_ADDR_ANY)) {
        BLE_HS_LOG(DEBUG, "peer_addr_type=%d peer_addr=",
                       key_sec->peer_addr.type);
        ble_hs_log_flat_buf(key_sec->peer_addr.val, 6);
        BLE_HS_LOG(DEBUG, " ");
    }
    if (key_sec->ediv_rand_present) {
        BLE_HS_LOG(DEBUG, "ediv=0x%02x rand=0x%llx ",
                       key_sec->ediv, key_sec->rand_num);
    }
}

static int
ble_store_config_find_sec(const struct ble_store_key_sec *key_sec,
                          const struct ble_store_value_sec *value_secs,
                          int num_value_secs)
{
    const struct ble_store_value_sec *cur;
    int skipped;
    int i;

    skipped = 0;

    for (i = 0; i < num_value_secs; i++) {
        cur = value_secs + i;

        if (ble_addr_cmp(&key_sec->peer_addr, BLE_ADDR_ANY)) {
		#if SMP_MULTI_ADDR_MODE_EN
        	ble_addr_t own_addr;
            if(key_sec->is_own_addr_valid == 0xAA){
                own_addr = key_sec->own_addr;
            }
            else{
                own_addr.type = ble_hs_id_get_used_addr_type();
                int rc = ble_hs_id_copy_addr(own_addr.type, own_addr.val, NULL);
                if(rc != 0){
                    return -1;
                }
            }

			if (ble_addr_cmp(&cur->peer_addr, &key_sec->peer_addr) ||
				ble_addr_cmp(&cur->own_addr, &own_addr)) {
				continue;
			}
		#else
            if (ble_addr_cmp(&cur->peer_addr, &key_sec->peer_addr)) {
                continue;
            }
		#endif
        }

        if (key_sec->ediv_rand_present) {
            if (cur->ediv != key_sec->ediv) {
                continue;
            }

            if (cur->rand_num != key_sec->rand_num) {
                continue;
            }
        }

        if (key_sec->idx > skipped) {
            skipped++;
            continue;
        }

        return i;
    }

    return -1;
}

static int
ble_store_config_read_our_sec(const struct ble_store_key_sec *key_sec,
                              struct ble_store_value_sec *value_sec)
{
    int idx;

    idx = ble_store_config_find_sec(key_sec, ble_store_config_our_secs,
                                    ble_store_config_num_our_secs);
    if (idx == -1) {
        return BLE_HS_ENOENT;
    }

    *value_sec = ble_store_config_our_secs[idx];
    return 0;
}


static int
ble_store_config_write_our_sec(const struct ble_store_value_sec *value_sec)
{
    struct ble_store_key_sec key_sec;
    int idx;
    int rc;

    BLE_HS_LOG(DEBUG, "persisting our sec; ");
    ble_store_config_print_value_sec(value_sec);

    ble_store_key_from_value_sec(&key_sec, value_sec);
    idx = ble_store_config_find_sec(&key_sec, ble_store_config_our_secs,
                                    ble_store_config_num_our_secs);
    if (idx == -1) {
        if (ble_store_config_num_our_secs >= MYNEWT_VAL(BLE_STORE_MAX_BONDS)) {
            BLE_HS_LOG(DEBUG, "error persisting our sec; too many entries "
                              "(%d)\n", ble_store_config_num_our_secs);
            return BLE_HS_ESTORE_CAP;
        }

        idx = ble_store_config_num_our_secs;
        ble_store_config_num_our_secs++;
    }

    ble_store_config_our_secs[idx] = *value_sec;

    rc = ble_store_config_persist_our_secs();
    if (rc != 0) {
        return rc;
    }

    return 0;
}

static int
ble_store_config_delete_obj(void *values, int value_size, int idx,
                            int *num_values)
{
    uint8_t *dst;
    uint8_t *src;
    int move_count;

    (*num_values)--;
    if (idx < *num_values) {
        dst = values;
        dst += idx * value_size;
        src = dst + value_size;

        move_count = *num_values - idx;
        memmove(dst, src, move_count * value_size);
    }

    return 0;
}

static int
ble_store_config_delete_sec(const struct ble_store_key_sec *key_sec,
                            struct ble_store_value_sec *value_secs,
                            int *num_value_secs)
{
    int idx;
    int rc;

    idx = ble_store_config_find_sec(key_sec, value_secs, *num_value_secs);
    if (idx == -1) {
        return BLE_HS_ENOENT;
    }

    rc = ble_store_config_delete_obj(value_secs, sizeof *value_secs, idx,
                                  num_value_secs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

static int
ble_store_config_delete_our_sec(const struct ble_store_key_sec *key_sec)
{
    int rc;

    rc = ble_store_config_delete_sec(key_sec, ble_store_config_our_secs,
                                     &ble_store_config_num_our_secs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_store_config_persist_our_secs();
    if (rc != 0) {
        return rc;
    }

    return 0;
}

static int
ble_store_config_delete_peer_sec(const struct ble_store_key_sec *key_sec)
{
    int rc;

    rc = ble_store_config_delete_sec(key_sec, ble_store_config_peer_secs,
                                  &ble_store_config_num_peer_secs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_store_config_persist_peer_secs();
    if (rc != 0) {
        return rc;
    }

    return 0;
}

static int
ble_store_config_read_peer_sec(const struct ble_store_key_sec *key_sec,
                               struct ble_store_value_sec *value_sec)
{
    int idx;

    idx = ble_store_config_find_sec(key_sec, ble_store_config_peer_secs,
                             ble_store_config_num_peer_secs);
    if (idx == -1) {
        return BLE_HS_ENOENT;
    }

    *value_sec = ble_store_config_peer_secs[idx];
    return 0;
}

static int
ble_store_config_write_peer_sec(const struct ble_store_value_sec *value_sec)
{
    struct ble_store_key_sec key_sec;
    int idx;
    int rc;

    BLE_HS_LOG(DEBUG, "persisting peer sec; ");
    ble_store_config_print_value_sec(value_sec);

    ble_store_key_from_value_sec(&key_sec, value_sec);
    idx = ble_store_config_find_sec(&key_sec, ble_store_config_peer_secs,
                                 ble_store_config_num_peer_secs);
    if (idx == -1) {
        if (ble_store_config_num_peer_secs >= MYNEWT_VAL(BLE_STORE_MAX_BONDS)) {
            BLE_HS_LOG(DEBUG, "error persisting peer sec; too many entries "
                             "(%d)\n", ble_store_config_num_peer_secs);
            return BLE_HS_ESTORE_CAP;
        }

        idx = ble_store_config_num_peer_secs;
        ble_store_config_num_peer_secs++;
    }

    ble_store_config_peer_secs[idx] = *value_sec;

    rc = ble_store_config_persist_peer_secs();
    if (rc != 0) {
        return rc;
    }

    return 0;
}

/*****************************************************************************
 * $cccd                                                                     *
 *****************************************************************************/

static int
ble_store_config_find_cccd(const struct ble_store_key_cccd *key)
{
    struct ble_store_value_cccd *cccd;
    int skipped;
    int i;

    skipped = 0;
    for (i = 0; i < ble_store_config_num_cccds; i++) {
        cccd = ble_store_config_cccds + i;

        if (ble_addr_cmp(&key->peer_addr, BLE_ADDR_ANY)) {
		#if SMP_MULTI_ADDR_MODE_EN
        	ble_addr_t own_addr;
            if(key->is_own_addr_valid == 0xAA){
                own_addr = key->own_addr;
            }
            else{
                own_addr.type = ble_hs_id_get_used_addr_type();
                int rc = ble_hs_id_copy_addr(own_addr.type, own_addr.val, NULL);
                if(rc != 0){
                    return -1;
                }
            }

			if (ble_addr_cmp(&cccd->peer_addr, &key->peer_addr) ||
				ble_addr_cmp(&cccd->own_addr, &own_addr)) {
				continue;
			}
		#else
            if (ble_addr_cmp(&cccd->peer_addr, &key->peer_addr)) {
                continue;
            }
		#endif
        }

        if (key->chr_val_handle != 0) {
            if (cccd->chr_val_handle != key->chr_val_handle) {
                continue;
            }
        }

        if (key->idx > skipped) {
            skipped++;
            continue;
        }

        return i;
    }

    return -1;
}

static int
ble_store_config_delete_cccd(const struct ble_store_key_cccd *key_cccd)
{
    int idx;
    int rc;

    idx = ble_store_config_find_cccd(key_cccd);
    if (idx == -1) {
        return BLE_HS_ENOENT;
    }

    rc = ble_store_config_delete_obj(ble_store_config_cccds,
                                     sizeof *ble_store_config_cccds,
                                     idx,
                                     &ble_store_config_num_cccds);
    if (rc != 0) {
        return rc;
    }

    rc = ble_store_config_persist_cccds();
    if (rc != 0) {
        return rc;
    }

    return 0;
}

static int
ble_store_config_read_cccd(const struct ble_store_key_cccd *key_cccd,
                           struct ble_store_value_cccd *value_cccd)
{
    int idx;

    idx = ble_store_config_find_cccd(key_cccd);
    if (idx == -1) {
        return BLE_HS_ENOENT;
    }

    *value_cccd = ble_store_config_cccds[idx];
    return 0;
}

static int
ble_store_config_write_cccd(const struct ble_store_value_cccd *value_cccd)
{
    struct ble_store_key_cccd key_cccd;
    int idx;
    int rc;

    ble_store_key_from_value_cccd(&key_cccd, value_cccd);
    idx = ble_store_config_find_cccd(&key_cccd);
    if (idx == -1) {
        if (ble_store_config_num_cccds >= MYNEWT_VAL(BLE_STORE_MAX_CCCDS)) {
            BLE_HS_LOG(ERROR, "error persisting cccd, too many entries (%d)", ble_store_config_num_cccds);
            return BLE_HS_ESTORE_CAP;
        }

        idx = ble_store_config_num_cccds;
        ble_store_config_num_cccds++;
    }

    ble_store_config_cccds[idx] = *value_cccd;

    rc = ble_store_config_persist_cccds();
    if (rc != 0) {
        return rc;
    }

    return 0;
}

/*****************************************************************************
 * $api                                                                      *
 *****************************************************************************/

/**
 * Searches the database for an object matching the specified criteria.
 *
 * @return                      0 if a key was found; else BLE_HS_ENOENT.
 */
int
ble_store_config_read(int obj_type, const union ble_store_key *key,
                      union ble_store_value *value)
{
    int rc;

    switch (obj_type) {
    case BLE_STORE_OBJ_TYPE_PEER_SEC:
        /* An encryption procedure (bonding) is being attempted.  The nimble
         * stack is asking us to look in our key database for a long-term key
         * corresponding to the specified ediv and random number.
         *
         * Perform a key lookup and populate the context object with the
         * result.  The nimble stack will use this key if this function returns
         * success.
         */
        BLE_HS_LOG(DEBUG, "looking up peer sec; ");
        ble_store_config_print_key_sec(&key->sec);
        BLE_HS_LOG(DEBUG, "\n");
        rc = ble_store_config_read_peer_sec(&key->sec, &value->sec);
        return rc;

    case BLE_STORE_OBJ_TYPE_OUR_SEC:
        BLE_HS_LOG(DEBUG, "looking up our sec; ");
        ble_store_config_print_key_sec(&key->sec);
        BLE_HS_LOG(DEBUG, "\n");
        rc = ble_store_config_read_our_sec(&key->sec, &value->sec);
        return rc;

    case BLE_STORE_OBJ_TYPE_CCCD:
        rc = ble_store_config_read_cccd(&key->cccd, &value->cccd);
        return rc;

    default:
        return BLE_HS_ENOTSUP;
    }
}

/**
 * Adds the specified object to the database.
 *
 * @return                      0 on success;
 *                              BLE_HS_ESTORE_CAP if the database is full.
 */
int
ble_store_config_write(int obj_type, const union ble_store_value *val)
{
    int rc;

    switch (obj_type) {
    case BLE_STORE_OBJ_TYPE_PEER_SEC:
        rc = ble_store_config_write_peer_sec(&val->sec);
        return rc;

    case BLE_STORE_OBJ_TYPE_OUR_SEC:
        rc = ble_store_config_write_our_sec(&val->sec);
        return rc;

    case BLE_STORE_OBJ_TYPE_CCCD:
        rc = ble_store_config_write_cccd(&val->cccd);
        return rc;

    default:
        return BLE_HS_ENOTSUP;
    }
}

int
ble_store_config_delete(int obj_type, const union ble_store_key *key)
{
    int rc;

    switch (obj_type) {
    case BLE_STORE_OBJ_TYPE_PEER_SEC:
        rc = ble_store_config_delete_peer_sec(&key->sec);
        return rc;

    case BLE_STORE_OBJ_TYPE_OUR_SEC:
        rc = ble_store_config_delete_our_sec(&key->sec);
        return rc;

    case BLE_STORE_OBJ_TYPE_CCCD:
        rc = ble_store_config_delete_cccd(&key->cccd);
        return rc;

    default:
        return BLE_HS_ENOTSUP;
    }
}

void
ble_store_config_init(void)
{
    /* Ensure this function only gets called by sysinit. */
#ifdef IP_101x
	ble_store_config_our_secs = panchip_mem_get((MYNEWT_VAL(BLE_STORE_MAX_BONDS)) * sizeof(struct ble_store_value_sec), 8);
	ble_store_config_peer_secs = panchip_mem_get((MYNEWT_VAL(BLE_STORE_MAX_BONDS)) * sizeof(struct ble_store_value_sec), 8);
	ble_store_config_cccds = panchip_mem_get((MYNEWT_VAL(BLE_STORE_MAX_CCCDS)) * sizeof(struct ble_store_value_cccd), 4);
#endif

    SYSINIT_ASSERT_ACTIVE();

#if MYNEWT_VAL_BLE_STORE_CONFIG_PERSIST
	/* kv initialization */
	extern void kv_init(void);
    kv_init();
#endif

    ble_hs_cfg.store_read_cb = ble_store_config_read;
    ble_hs_cfg.store_write_cb = ble_store_config_write;
    ble_hs_cfg.store_delete_cb = ble_store_config_delete;

    /* Re-initialize BSS values in case of unit tests. */
    ble_store_config_num_our_secs = 0;
    ble_store_config_num_peer_secs = 0;
    ble_store_config_num_cccds = 0;

    ble_store_config_conf_init();
}


struct find_replace_addr {
    bool found_addr;
    ble_addr_t* src_addr;
    ble_addr_t* target_addr;
};

extern uint8_t ble_hs_misc_peer_addr_type_to_id(uint8_t peer_addr_type);

static int
ble_store_util_iter_replace_mac(int obj_type,
                          union ble_store_value *val,
                          void *arg)
{
    int rc;

    struct find_replace_addr* addrs = arg;

    if(val->cccd.peer_addr.type == 1) /*random address, maybe rpa addr*/
    {
        if (ble_addr_cmp(addrs->src_addr, &val->cccd.peer_addr) == 0) {
            memcpy(&val->cccd.peer_addr, addrs->target_addr, sizeof(ble_addr_t));
            val->cccd.peer_addr.type = ble_hs_misc_peer_addr_type_to_id(val->cccd.peer_addr.type);
            rc = ble_store_config_write(BLE_STORE_OBJ_TYPE_CCCD, val);
            addrs->found_addr = true;
            if (rc != 0) {
                return rc;
            }
        }
    }

    return 0;
}

int ble_store_iterate_replace_cccd(ble_addr_t* src_addr, ble_addr_t* target_addr)
{
    int rc;
    struct find_replace_addr addrs;

    addrs.found_addr = false;
    addrs.src_addr = src_addr;
    addrs.target_addr = target_addr;

    rc = ble_store_iterate(BLE_STORE_OBJ_TYPE_CCCD,
                           ble_store_util_iter_replace_mac,
                           &addrs);

    if(addrs.found_addr)
    {
        union ble_store_key key;

        memset(&key, 0, sizeof key);
        key.sec.peer_addr = *src_addr;

        rc = ble_store_util_delete_all(BLE_STORE_OBJ_TYPE_CCCD, &key);
    }

    if (rc != 0) {
        return rc;
    }

    return 0;
}
