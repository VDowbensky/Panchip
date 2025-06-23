/*
 * Copyright (c) 2024, sakumisu
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PanSeries.h"
#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "app_log.h"

#define USBD_VID            0xFFFF
#define USBD_PID            0x9F0C
#define USBD_MAX_POWER      100
#define USBD_LANGID_STRING  1033

#define CDC_IN_EP           0x81
#define CDC_OUT_EP          0x01
#define CDC_INT_EP          0x82

/*!< config descriptor size */
#define USB_CONFIG_SIZE     (9 + CDC_ACM_DESCRIPTOR_LEN)

#define CDC_MAX_MPS         64

/*!< global descriptor */
static const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02),
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
    'C', 0x00,                  /* wcChar10 */
    'D', 0x00,                  /* wcChar11 */
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

#define BUFFER_SIZE       256

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[BUFFER_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[BUFFER_SIZE];

static volatile bool ep_tx_busy_flag = false;
static volatile bool ep_rx_done_flag = false;
static volatile uint32_t ep_rx_actual_size = 0;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
        case USBD_EVENT_RESET:
            APP_LOG_INFO("USBD_EVENT_RESET\n");
            break;
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
#if CONFIG_USBDEV_SOF_INT_EN
        case USBD_EVENT_SOF:
            APP_LOG_INFO("USBD_EVENT_SOF\n");
            break;
#endif
        case USBD_EVENT_RESUME:
            APP_LOG_INFO("USBD_EVENT_RESUME\n");
            break;
        case USBD_EVENT_SUSPEND:
            APP_LOG_INFO("USBD_EVENT_SUSPEND\n");
            break;
        case USBD_EVENT_CONFIGURED:
            ep_tx_busy_flag = false;
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, BUFFER_SIZE);
            APP_LOG_INFO("USBD_EVENT_CONFIGURED\n");
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            APP_LOG_INFO("USBD_EVENT_SET_REMOTE_WAKEUP\n");
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            APP_LOG_INFO("USBD_EVENT_CLR_REMOTE_WAKEUP\n");
            break;

        default:
            break;
    }
}

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    APP_LOG_DEBUG("actual out len:%d\n", nbytes);
#if 0
    for (int i = 0; i < BUFFER_SIZE; i++) {
        USB_LOG_RAW("%02x ", read_buffer[i]);
        if ((i + 1) % 16 == 0) {
            USB_LOG_RAW("\n");
        }
    }
#endif
    ep_rx_done_flag = true;
    ep_rx_actual_size = nbytes;
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    APP_LOG_DEBUG("actual in len:%d\n", nbytes);

    if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(busid, CDC_IN_EP, NULL, 0);
    } else {
        ep_tx_busy_flag = false;
    }
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

static struct usbd_interface intf0;
static struct usbd_interface intf1;

void cdc_acm_init(void)
{
    usbd_desc_register(0, cdc_descriptor);
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf0));
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf1));
    usbd_add_endpoint(0, &cdc_out_ep);
    usbd_add_endpoint(0, &cdc_in_ep);
    usbd_initialize(0, USB_BASE, usbd_event_handler);
}

static uint32_t baudrate = 2000000; // 2M
static uint8_t databits_num = 8;    // 8-bits
static uint8_t parity_type = 0;     // None
static uint8_t char_format = 0;     // 1 stop bit

extern const char *stop_name[];
extern const char *parity_name[];

/* Re-implement the weak function in usbd_cdc_acm.c */
void usbd_cdc_acm_set_line_coding(uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding)
{
    (void)busid;
    (void)intf;

    APP_LOG_DEBUG("USB CDC-ACM intf:%d, SET linecoding <%d %d %s %s>\r\n",
                  intf,
                  line_coding->dwDTERate,
                  line_coding->bDataBits,
                  parity_name[line_coding->bParityType],
                  stop_name[line_coding->bCharFormat]);

    baudrate     = line_coding->dwDTERate;
    databits_num = line_coding->bDataBits;
    parity_type  = line_coding->bParityType;
    char_format  = line_coding->bCharFormat;
}

/* Re-implement the weak function in usbd_cdc_acm.c */
void usbd_cdc_acm_get_line_coding(uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding)
{
    (void)busid;
    (void)intf;

    APP_LOG_DEBUG("USB CDC-ACM intf:%d, GET linecoding <%d %d %s %s>\r\n",
                  intf,
                  baudrate,
                  databits_num,
                  parity_name[parity_type],
                  stop_name[char_format]);

    line_coding->dwDTERate   = baudrate;
    line_coding->bDataBits   = databits_num;
    line_coding->bParityType = parity_type;
    line_coding->bCharFormat = char_format;
}

void cdc_acm_data_send_receive_test(void)
{
    // Check if there is data received from USB Host VCOM
    if (ep_rx_done_flag) {
        // Clear Rx done flag
        ep_rx_done_flag = false;

        // Set data Tx busy flag
        ep_tx_busy_flag = true;
        // Just loop received data back
        memcpy(write_buffer, read_buffer, ep_rx_actual_size);
        // Start to write data to USB Host
        usbd_ep_start_write(0, CDC_IN_EP, write_buffer, ep_rx_actual_size);
        // Busy wait for data Tx done
        while (ep_tx_busy_flag) {
        }

        /* setup next out ep read transfer */
        usbd_ep_start_read(0, CDC_OUT_EP, read_buffer, BUFFER_SIZE);
    }
}
