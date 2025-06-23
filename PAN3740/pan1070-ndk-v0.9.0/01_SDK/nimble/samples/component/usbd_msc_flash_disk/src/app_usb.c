/*
 * Copyright (c) 2024, sakumisu
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PanSeries.h"
#include "usbd_core.h"
#include "usbd_msc.h"
#include "app_log.h"

#define MSC_IN_EP  0x81
#define MSC_OUT_EP 0x02

#define USBD_VID           0xFFFF
#define USBD_PID           0x9F0B
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#define USB_CONFIG_SIZE (9 + MSC_DESCRIPTOR_LEN)

#define MSC_MAX_MPS 64

const uint8_t msc_storage_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'M', 0x00,                  /* wcChar10 */
    'S', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '5', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
    0x00
};

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
        case USBD_EVENT_INIT:
            APP_LOG_INFO("USBD_EVENT_INIT\n");
            break;
        case USBD_EVENT_DEINIT:
            APP_LOG_INFO("USBD_EVENT_DEINIT\n");
            break;
        case USBD_EVENT_RESET:
            APP_LOG_INFO("USBD_EVENT_RESET\n");
            break;
#if CONFIG_USBDEV_PLUG_INT_EN
        case USBD_EVENT_CONNECTED:
#if CONFIG_USBDEV_SOF_INT_EN
            // Enable SOF interrupt when usb plugged in
            WRITE_REG(USB->INT_USBE, READ_REG(USB->INT_USBE) | M_INTR_SOF_ENABLE);
#endif
            APP_LOG_INFO("USBD_EVENT_CONNECTED\n");
            break;
        case USBD_EVENT_DISCONNECTED:
#if CONFIG_USBDEV_SOF_INT_EN
            // Disable SOF interrupt when usb plugged out
            WRITE_REG(USB->INT_USBE, READ_REG(USB->INT_USBE) & ~M_INTR_SOF_ENABLE);
#endif
            APP_LOG_INFO("USBD_EVENT_DISCONNECTED\n");
            break;
#endif // CONFIG_USBDEV_PLUG_INT_EN
#if CONFIG_USBDEV_SOF_INT_EN
        case USBD_EVENT_SOF:
            APP_LOG_INFO("USBD_EVENT_SOF\n");
            break;
#endif // CONFIG_USBDEV_SOF_INT_EN
        case USBD_EVENT_RESUME:
            APP_LOG_INFO("USBD_EVENT_RESUME\n");
            break;
        case USBD_EVENT_SUSPEND:
            APP_LOG_INFO("USBD_EVENT_SUSPEND\n");
            break;
        case USBD_EVENT_CONFIGURED:
            APP_LOG_INFO("USBD_EVENT_CONFIGURED\n");
            break;
        case USBD_EVENT_SET_INTERFACE:
            APP_LOG_INFO("USBD_EVENT_SET_INTERFACE\n");
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            APP_LOG_INFO("USBD_EVENT_SET_REMOTE_WAKEUP\n");
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            APP_LOG_INFO("USBD_EVENT_CLR_REMOTE_WAKEUP\n");
            break;
        default:
            APP_LOG_WRN("Unhandled USBD Event: %d\n", event);
            break;
    }
}

#if DISK_USE_EXTERNAL_SPI_FLASH
#define MSC_STORAGE_ADDR    0x0
#define MSC_STORAGE_SIZE    0x100000    // 1MB
extern int spi_flash_memory_read(const uint8_t *buf, uint32_t flash_addr, size_t nbytes);
extern int spi_flash_memory_write(const uint8_t *buf, uint32_t flash_addr, size_t nbytes);
extern void spi_flash_memory_write_sync(void);
#else
#define MSC_STORAGE_ADDR    CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR
#define MSC_STORAGE_SIZE    CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE
extern int soc_flash_memory_read(const uint8_t *buf, uint32_t flash_addr, size_t nbytes);
extern int soc_flash_memory_write(const uint8_t *buf, uint32_t flash_addr, size_t nbytes);
extern void soc_flash_memory_write_sync(void);
#endif

#define BLOCK_SIZE          512 // Should be exact divisor of CONFIG_USBDEV_MSC_MAX_BUFSIZE
#define BLOCK_COUNT         (MSC_STORAGE_SIZE / BLOCK_SIZE)

void usbd_msc_get_cap(uint8_t busid, uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    *block_num = BLOCK_COUNT;
    *block_size = BLOCK_SIZE;
}

int usbd_msc_sector_read(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    APP_LOG_DEBUG("%s(): sector = %d, length = %d\n", __FUNCTION__, sector, length);

    // Read data from msc storage area
#if DISK_USE_EXTERNAL_SPI_FLASH
    spi_flash_memory_read(buffer, MSC_STORAGE_ADDR + sector * BLOCK_SIZE, length);
#else
    soc_flash_memory_read(buffer, MSC_STORAGE_ADDR + sector * BLOCK_SIZE, length);
#endif
    return 0;
}

int usbd_msc_sector_write(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    APP_LOG_DEBUG("%s(): sector = %d, length = %d\n", __FUNCTION__, sector, length);

    // Write data to msc storage area (with erase when needed)
#if DISK_USE_EXTERNAL_SPI_FLASH
    spi_flash_memory_write(buffer, MSC_STORAGE_ADDR + sector * BLOCK_SIZE, length);
    spi_flash_memory_write_sync();
#else
    soc_flash_memory_write(buffer, MSC_STORAGE_ADDR + sector * BLOCK_SIZE, length);
    soc_flash_memory_write_sync();
#endif

    return 0;
}

static struct usbd_interface intf0;

void usbd_msc_init(void)
{
    usbd_desc_register(0, msc_storage_descriptor);
    usbd_add_interface(0, usbd_msc_init_intf(0, &intf0, MSC_OUT_EP, MSC_IN_EP));

    usbd_initialize(0, USB_BASE, usbd_event_handler);
}

#if defined(CONFIG_USBDEV_MSC_POLLING)
void msc_storage_polling(void)
{
    usbd_msc_polling(busid);
}
#endif
