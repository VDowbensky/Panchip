/*
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PanSeries.h"
#include "usbd_core.h"
#include "usbd_hid.h"
#include "usbd_msc.h"
#include "app_log.h"

////////////////////////// Configs and Data for HID //////////////////////////
/*!< hidraw in endpoint */
#define HIDRAW_IN_EP            0x81
#define HIDRAW_IN_EP_SIZE       64
#define HIDRAW_IN_EP_INTERVAL   10

/*!< hidraw out endpoint */
#define HIDRAW_OUT_EP           0x01
#define HIDRAW_OUT_EP_SIZE      64
#define HIDRAW_OUT_EP_INTERVAL  10

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[HIDRAW_OUT_EP_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t send_buffer[HIDRAW_IN_EP_SIZE];

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

/*!< hid state ! Data can be sent only when state is idle  */
static volatile uint8_t hid_custom_state;


////////////////////////// Configs and Data for MSC //////////////////////////
/*!< msc endpoints */
#define MSC_IN_EP               0x82
#define MSC_OUT_EP              0x02
#define MSC_MAX_MPS             64

/*!< custom hid report descriptor size */
#if CONFIG_HID_ASSIGN_REPORT_ID
#define HID_CUSTOM_REPORT_DESC_SIZE 38
#else
#define HID_CUSTOM_REPORT_DESC_SIZE 24
#endif

#define BLOCK_SIZE  512
#define BLOCK_COUNT 10

typedef struct
{
    uint8_t BlockSpace[BLOCK_SIZE];
} BLOCK_TYPE;

BLOCK_TYPE mass_block[BLOCK_COUNT];

///////////////////// Configs for USB Device/Config ////////////////////////
#define USBD_VID                0xFFFF
#define USBD_PID                0x9F10
#define USBD_MAX_POWER          100
#define USBD_LANGID_STRING      1033

/*!< config descriptor size */
#define USB_CONFIG_DESC_SIZ     (9 + MSC_DESCRIPTOR_LEN + (9 + 9 + 7 + 7))

///////////////////// Global USB Descriptor Table /////////////////////////
/*!< global descriptor */
static const uint8_t hid_msc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_DESC_SIZ, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02),
    /************** Descriptor of Custom interface *****************/
    0x09,                          /* bLength: Interface Descriptor size */
    USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x01,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x02,                          /* bNumEndpoints */
    0x03,                          /* bInterfaceClass: HID */
    0x01,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    0x00,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0x00,                          /* iInterface: Index of string descriptor */
    /******************** Descriptor of Custom HID ********************/
    0x09,                        /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE_HID,     /* bDescriptorType: HID */
    0x11,                        /* bcdHID: HID Class Spec release number */
    0x01,
    0x00,                        /* bCountryCode: Hardware target country */
    0x01,                        /* bNumDescriptors: Number of HID class descriptors to follow */
    0x22,                        /* bDescriptorType */
    HID_CUSTOM_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
    0x00,
    /******************** Descriptor of Custom in endpoint ********************/
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    HIDRAW_IN_EP,                 /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    WBVAL(HIDRAW_IN_EP_SIZE),     /* wMaxPacketSize: */
    HIDRAW_IN_EP_INTERVAL,        /* bInterval: Polling Interval */
    /******************** Descriptor of Custom out endpoint ********************/
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    HIDRAW_OUT_EP,                /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    WBVAL(HIDRAW_OUT_EP_SIZE),    /* wMaxPacketSize: */
    HIDRAW_OUT_EP_INTERVAL,       /* bInterval: Polling Interval */
    /* 73 */
    /*
     * string0 descriptor
     */
    USB_LANGID_INIT(USBD_LANGID_STRING),
    /*
     * string1 descriptor
     */
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
    /*
     * string2 descriptor
     */
    0x2E,                       /* bLength */
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
    'H', 0x00,                  /* wcChar10 */
    'I', 0x00,                  /* wcChar11 */
    'D', 0x00,                  /* wcChar12 */
    '_', 0x00,                  /* wcChar13 */
    'M', 0x00,                  /* wcChar14 */
    'S', 0x00,                  /* wcChar15 */
    'C', 0x00,                  /* wcChar16 */
    ' ', 0x00,                  /* wcChar17 */
    'D', 0x00,                  /* wcChar18 */
    'E', 0x00,                  /* wcChar19 */
    'M', 0x00,                  /* wcChar20 */
    'O', 0x00,                  /* wcChar21 */
    /*
     * string3 descriptor
     */
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

/*!< custom hid report descriptor */
static const uint8_t hid_custom_report_desc[HID_CUSTOM_REPORT_DESC_SIZE] = {
#if CONFIG_HID_ASSIGN_REPORT_ID
    /* USER CODE BEGIN 0 */
    0x06, 0x00, 0xff, /* USAGE_PAGE (Vendor Defined Page 1) */
    0x09, 0x01,       /* USAGE (Vendor Usage 1) */
    0xa1, 0x01,       /* COLLECTION (Application) */
    0x85, 0x02,       /*   REPORT ID (0x02) */
    0x09, 0x01,       /*   USAGE (Vendor Usage 1) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x26, 0xff, 0x00, /*   LOGICAL_MAXIMUM (255) */
    0x95, 0x40 - 1,   /*   REPORT_COUNT (63) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x81, 0x02,       /*   INPUT (Data,Var,Abs) */
    /* <___________________________________________________> */
    0x85, 0x01,       /*   REPORT ID (0x01) */
    0x09, 0x01,       /*   USAGE (Vendor Usage 1) */
    0x15, 0x00,       /*   LOGICAL_MINIMUM (0) */
    0x26, 0xff, 0x00, /*   LOGICAL_MAXIMUM (255) */
    0x95, 0x40 - 1,   /*   REPORT_COUNT (63) */
    0x75, 0x08,       /*   REPORT_SIZE (8) */
    0x91, 0x02,       /*   OUTPUT (Data,Var,Abs) */
    /* USER CODE END 0 */
    0xC0 /*     END_COLLECTION               */
#else
    0x06, 0x00, 0xff, // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,       // USAGE (Vendor Usage 1)
    0xa1, 0x00,       // COLLECTION (Physical)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x25, 0xff,       //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,       //   REPORT_SIZE (8)
    0x95, 0x40,       //   REPORT_COUNT (64)
    0x09, 0x01,       //   USAGE (Vendor Usage 1)
    0x81, 0x02,       //   INPUT (Data,Var,Abs)
    0x09, 0x01,       //   USAGE (Vendor Usage 1)
    0x91, 0x02,       //   OUTPUT (Data,Var,Abs)
    0xc0              // END_COLLECTION
#endif // CONFIG_HID_ASSIGN_REPORT_ID
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
            hid_custom_state = HID_STATE_IDLE;
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, HIDRAW_OUT_EP, read_buffer, HIDRAW_OUT_EP_SIZE);
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

////////////////////////// Functions for HID //////////////////////////

static void usbd_hid_custom_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    (void)ep;
    USB_LOG_RAW("actual in len:%d\n", nbytes);
    hid_custom_state = HID_STATE_IDLE;
}

static void usbd_hid_custom_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual out len:%d\n", nbytes);
    usbd_ep_start_read(busid, ep, read_buffer, HIDRAW_IN_EP_SIZE);
    read_buffer[0] = 0x02; /* IN report id (if CONFIG_HID_ASSIGN_REPORT_ID enabled) */
    usbd_ep_start_write(busid, HIDRAW_IN_EP, read_buffer, nbytes);
}

static struct usbd_endpoint custom_in_ep = {
    .ep_cb = usbd_hid_custom_in_callback,
    .ep_addr = HIDRAW_IN_EP
};

static struct usbd_endpoint custom_out_ep = {
    .ep_cb = usbd_hid_custom_out_callback,
    .ep_addr = HIDRAW_OUT_EP
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

void usbd_hid_custom_msc_init(void)
{
    usbd_desc_register(0, hid_msc_descriptor);

    usbd_add_interface(0, usbd_msc_init_intf(0, &intf0, MSC_OUT_EP, MSC_IN_EP));

    usbd_add_interface(0, usbd_hid_init_intf(0, &intf1, hid_custom_report_desc, HID_CUSTOM_REPORT_DESC_SIZE));
    usbd_add_endpoint(0, &custom_in_ep);
    usbd_add_endpoint(0, &custom_out_ep);

    usbd_initialize(0, USB_BASE, usbd_event_handler);
}
