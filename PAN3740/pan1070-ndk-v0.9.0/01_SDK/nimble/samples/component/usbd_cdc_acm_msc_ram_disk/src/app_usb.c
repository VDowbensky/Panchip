/*
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PanSeries.h"
#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "usbd_msc.h"
#include "app_log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/////////////////////// Configs and Data for CDC-ACM /////////////////////////
/*!< cdc-acm endpoints */
#define CDC_IN_EP               0x81    // EP1
#define CDC_OUT_EP              0x01    // EP1
#define CDC_INT_EP              0x82    // EP2
#define CDC_MAX_MPS             64

#define BUFFER_SIZE             256

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[BUFFER_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[BUFFER_SIZE];

static volatile bool ep_tx_busy_flag = false;
static SemaphoreHandle_t sem_ep_rx_done = NULL;    // ep_rx_done_flag
static volatile uint32_t ep_rx_actual_size = 0;

static volatile uint32_t baudrate = 2000000; // 2M
static volatile uint8_t databits_num = 8;    // 8-bits
static volatile uint8_t parity_type = 0;     // None
static volatile uint8_t char_format = 0;     // 1 stop bit

////////////////////////// Configs and Data for MSC //////////////////////////
/*!< msc endpoints */
#define MSC_IN_EP               0x83    // EP3
#define MSC_OUT_EP              0x03    // EP3
#define MSC_MAX_MPS             64

#define BLOCK_SIZE              512
#define BLOCK_COUNT             10

typedef struct
{
    uint8_t BlockSpace[BLOCK_SIZE];
} BLOCK_TYPE;

BLOCK_TYPE mass_block[BLOCK_COUNT];

///////////////////// Configs for USB Device/Config ////////////////////////
#define USBD_VID                0xFFFF
#define USBD_PID                0x9F11
#define USBD_MAX_POWER          100
#define USBD_LANGID_STRING      1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE         (9 + CDC_ACM_DESCRIPTOR_LEN + MSC_DESCRIPTOR_LEN)

///////////////////// Global USB Descriptor Table /////////////////////////
/*!< global descriptor */
static const uint8_t cdc_msc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x03, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02),
    MSC_DESCRIPTOR_INIT(0x02, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x00),
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
    '-', 0x00,                  /* wcChar11 */
    'M', 0x00,                  /* wcChar12 */
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

////////////////////////// USB Event Handler //////////////////////////

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
            ep_tx_busy_flag = false;
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, BUFFER_SIZE);
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


////////////////////////// Functions for CDC-ACM //////////////////////////

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

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    APP_LOG_DEBUG("actual out len:%d\n", nbytes);

    ep_rx_actual_size = nbytes;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sem_ep_rx_done, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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


////////////////////////// Functions for MSC //////////////////////////

void usbd_msc_get_cap(uint8_t busid, uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    *block_num = 1000; // Pretend having so many buffer, not has actually.
    *block_size = BLOCK_SIZE;
}

int usbd_msc_sector_read(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    APP_LOG_DEBUG("%s(): sector = %d, length = %d\n", __FUNCTION__, sector, length);

    if (sector < BLOCK_COUNT)
        memcpy(buffer, mass_block[sector].BlockSpace, length);

    return 0;
}

int usbd_msc_sector_write(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    APP_LOG_DEBUG("%s(): sector = %d, length = %d\n", __FUNCTION__, sector, length);

    if (sector < BLOCK_COUNT)
        memcpy(mass_block[sector].BlockSpace, buffer, length);

    return 0;
}


////////////////////////////// USB Init //////////////////////////////

static struct usbd_interface intf0;
static struct usbd_interface intf1;
static struct usbd_interface intf2;

void usbd_cdc_acm_msc_init(void)
{
    usbd_desc_register(0, cdc_msc_descriptor);

    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf0));
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf1));
    usbd_add_endpoint(0, &cdc_out_ep);
    usbd_add_endpoint(0, &cdc_in_ep);
    
    usbd_add_interface(0, usbd_msc_init_intf(0, &intf2, MSC_OUT_EP, MSC_IN_EP));

    usbd_initialize(0, USB_BASE, usbd_event_handler);
}


/////////////////////////// CDC-ACM Test /////////////////////////////

void cdc_acm_data_send_receive_test(void)
{
    // Create a semaphore for cdc-acm test
    sem_ep_rx_done = xSemaphoreCreateBinary();
    if (sem_ep_rx_done == NULL) {
        APP_LOG_ERR("sem_ep_rx_done created failed!\n");
    }

    while (1) {
        // Check if there is data received from USB Host VCOM
        if(xSemaphoreTake(sem_ep_rx_done, portMAX_DELAY) == pdTRUE) {
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
}
