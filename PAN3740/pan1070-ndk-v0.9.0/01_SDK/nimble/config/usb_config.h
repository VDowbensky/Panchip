/*
 * Copyright (c) 2022, sakumisu
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CHERRYUSB_CONFIG_H
#define CHERRYUSB_CONFIG_H

/* ================ USB common Configuration ================ */

#include "pan_sys_log.h"
#define CONFIG_USB_PRINTF(...) SYS_PRINT(__VA_ARGS__)

#ifndef CONFIG_USB_DBG_LEVEL
#define CONFIG_USB_DBG_LEVEL USB_DBG_INFO
#endif

/* Enable print with color */
//#define CONFIG_USB_PRINTF_COLOR_ENABLE

/* data align size when use dma */
#ifndef CONFIG_USB_ALIGN_SIZE
#define CONFIG_USB_ALIGN_SIZE 4
#endif

/* attribute data into no cache ram */
#define USB_NOCACHE_RAM_SECTION __attribute__((section(".noncacheable")))

/* ================= USB Device Stack Configuration ================ */

/* Ep0 in and out transfer buffer */
#ifndef CONFIG_USBDEV_REQUEST_BUFFER_LEN
#define CONFIG_USBDEV_REQUEST_BUFFER_LEN 256
#endif

/* Setup packet log for debug */
// #define CONFIG_USBDEV_SETUP_LOG_PRINT

/* Send ep0 in data from user buffer instead of copying into ep0 reqdata
 * Please note that user buffer must be aligned with CONFIG_USB_ALIGN_SIZE
*/
// #define CONFIG_USBDEV_EP0_INDATA_NO_COPY

/* Check if the input descriptor is correct */
// #define CONFIG_USBDEV_DESC_CHECK

/* Enable test mode */
// #define CONFIG_USBDEV_TEST_MODE

#ifndef CONFIG_USBDEV_MSC_MAX_LUN
#define CONFIG_USBDEV_MSC_MAX_LUN 1
#endif

#ifndef CONFIG_USBDEV_MSC_MAX_BUFSIZE
#define CONFIG_USBDEV_MSC_MAX_BUFSIZE 512
#endif

#ifndef CONFIG_USBDEV_MSC_MANUFACTURER_STRING
#define CONFIG_USBDEV_MSC_MANUFACTURER_STRING ""
#endif

#ifndef CONFIG_USBDEV_MSC_PRODUCT_STRING
#define CONFIG_USBDEV_MSC_PRODUCT_STRING ""
#endif

#ifndef CONFIG_USBDEV_MSC_VERSION_STRING
#define CONFIG_USBDEV_MSC_VERSION_STRING "0.01"
#endif

/* move msc read & write from isr to while(1), you should call usbd_msc_polling in while(1) */
// #define CONFIG_USBDEV_MSC_POLLING

/* move msc read & write from isr to thread */
#define CONFIG_USBDEV_MSC_THREAD

#ifndef CONFIG_USBDEV_MSC_PRIO
#define CONFIG_USBDEV_MSC_PRIO 4
#endif

#ifndef CONFIG_USBDEV_MSC_STACKSIZE
#define CONFIG_USBDEV_MSC_STACKSIZE 1024
#endif

#ifndef CONFIG_USBDEV_RNDIS_RESP_BUFFER_SIZE
#define CONFIG_USBDEV_RNDIS_RESP_BUFFER_SIZE 156
#endif

/* rndis transfer buffer size, must be a multiple of (1536 + 44)*/
#ifndef CONFIG_USBDEV_RNDIS_ETH_MAX_FRAME_SIZE
#define CONFIG_USBDEV_RNDIS_ETH_MAX_FRAME_SIZE 1580
#endif

#ifndef CONFIG_USBDEV_RNDIS_VENDOR_ID
#define CONFIG_USBDEV_RNDIS_VENDOR_ID 0x0000ffff
#endif

#ifndef CONFIG_USBDEV_RNDIS_VENDOR_DESC
#define CONFIG_USBDEV_RNDIS_VENDOR_DESC "CherryUSB"
#endif

#define CONFIG_USBDEV_RNDIS_USING_LWIP

/* ================ USB Device Port Configuration ================*/

#ifndef CONFIG_USBDEV_MAX_BUS
#define CONFIG_USBDEV_MAX_BUS 1 // for now, bus num must be 1 except hpm ip
#endif

#ifndef CONFIG_USBDEV_EP_NUM
#define CONFIG_USBDEV_EP_NUM 4
#endif

#ifndef CONFIG_USBDEV_PLUG_INT_EN
#define CONFIG_USBDEV_PLUG_INT_EN   1
#endif

#ifndef CONFIG_USBDEV_SOF_INT_EN
#define CONFIG_USBDEV_SOF_INT_EN    0
#endif

/* When your chip hardware supports high-speed and wants to initialize it in high-speed mode, the relevant IP will configure the internal or external high-speed PHY according to CONFIG_USB_HS. */
// #define CONFIG_USB_HS

/* ================ USB Device Interrupt Enable Ctrl Flags ================*/
#define M_INTR_SUSPEND_ENABLE                   0x01
#define M_INTR_RESUME_ENABLE                    0x02
#define M_INTR_RESET_ENABLE                     0x04
#define M_INTR_SOF_ENABLE                       0x08
#define M_INTR_PLUG_ENABLE                      0x10

#endif // CHERRYUSB_CONFIG_H
