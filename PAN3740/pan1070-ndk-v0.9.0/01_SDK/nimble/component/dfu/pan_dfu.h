/**
 *******************************************************************************
 * @file     pan_dfu.h
 * @create   2025-04-07
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

#ifndef __PAN_DFU_H
#define __PAN_DFU_H

#include "pan_smp.h"

#include "pan_smp.c"
#include "pan_smp_ble.c"
#include "pan_flash.c"

#include "tinycbor/src/cbor_buf_reader.c"
#include "tinycbor/src/cbor_mbuf_reader.c"
#include "tinycbor/src/cbor_mbuf_writer.c"
#include "tinycbor/src/cborencoder.c"
#include "tinycbor/src/cborparser.c"
#include "mcumgr/cborattr/src/cborattr.c"
#include "mcumgr/cmd/img_mgmt/src/img_mgmt.c"
#include "mcumgr/cmd/img_mgmt/src/img_mgmt_state.c"
#include "mcumgr/cmd/img_mgmt/src/img_mgmt_util.c"
#include "mcumgr/cmd/img_mgmt/port/mynewt/src/mynewt_img_mgmt.c"
#include "mcumgr/cmd/img_mgmt/port/mynewt/src/mynewt_img_mgmt_log.c"
#include "mcumgr/cmd/os_mgmt/port/mynewt/src/mynewt_os_mgmt.c"
#include "mcumgr/cmd/os_mgmt/src/os_mgmt.c"
#include "mcumgr/mgmt/src/mgmt.c"
#include "mcumgr/smp/src/smp.c"
#include "mcumgr/util/src/mcumgr_util.c"

#endif /*__PAN_DFU_H */
