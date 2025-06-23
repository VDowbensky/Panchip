/*
 * Copyright (c) 2022, sakumisu
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usb_dc_panchip.h"
#include "soc_api.h"

extern volatile bool g_llp_flag;

// clang-format off
const struct musb_fifo_cfg musb_device_table[4] = {
{ .ep_num =  0, .style = FIFO_TXRX,   .maxpacket = 64, },
{ .ep_num =  1, .style = FIFO_TXRX,   .maxpacket = 128, },
{ .ep_num =  2, .style = FIFO_TXRX,   .maxpacket = 128, },
{ .ep_num =  3, .style = FIFO_TXRX,   .maxpacket = 128, },
};
// clang-format on

#if 0
uint8_t usbd_get_musb_fifo_cfg(struct musb_fifo_cfg **cfg)
{
    *cfg = musb_device_table;
    return sizeof(musb_device_table) / sizeof(musb_device_table[0]);
}

uint32_t usb_get_musb_ram_size(void)
{
    return 1024;
}
#endif

void usbd_musb_delay_ms(uint8_t ms)
{
    soc_busy_wait(1000 * ms);
}

#define HWREG(x) \
    (*((volatile uint32_t *)(x)))
#define HWREGH(x) \
    (*((volatile uint16_t *)(x)))
#define HWREGB(x) \
    (*((volatile uint8_t *)(x)))

#ifndef USB_BASE
#define USB_BASE (g_usbdev_bus[0].reg_base)
#endif

#define MUSB_FADDR_OFFSET       0x00    // [R/W] FAddr
#define MUSB_POWER_OFFSET       0x01    // [R/W] Power
#define MUSB_TXIS_OFFSET        0x02    // [ R ] IntrIn1
#define MUSB_RXIS_OFFSET        0x04    // [ R ] IntrOut1
#define MUSB_IS_OFFSET          0x06    // [ R ] IntrUSB
#define MUSB_TXIE_OFFSET        0x07    // [R/W] IntrIn1E
#define MUSB_RXIE_OFFSET        0x09    // [R/W] IntrOut1E
#define MUSB_IE_OFFSET          0x0B    // [R/W] IntrUSBE

#define MUSB_FRAME1_OFFSET      0x0C    // [ R ] Frame1
#define MUSB_FRAME2_OFFSET      0x0D    // [ R ] Frame2

#define MUSB_EPIDX_OFFSET       0x0E    // [R/W] Index

#define MUSB_IND_TXMAP_OFFSET   0x10    // [R/W] InMaxP
#define MUSB_IND_TXCSRL_OFFSET  0x11    // [R/W] CSR0/InCSR1
#define MUSB_IND_TXCSRH_OFFSET  0x12    // [R/W] InCSR2
#define MUSB_IND_RXMAP_OFFSET   0x13    // [R/W] OutMaxP
#define MUSB_IND_RXCSRL_OFFSET  0x14    // [R/W] OutCSR1
#define MUSB_IND_RXCSRH_OFFSET  0x15    // [R/W] OutCSR2
#define MUSB_IND_RXCOUNT_OFFSET 0x16    // [ R ] Count0/OutCount1 & OutCount2 (2B)

#define MUSB_FIFO_OFFSET        0x20    // [R/W] FIFOx

#define USB_FIFO_BASE(ep_idx) (USB_BASE + MUSB_FIFO_OFFSET + (ep_idx << 2))

typedef enum {
    USB_EP0_STATE_SETUP = 0x0,      /**< SETUP DATA */
    USB_EP0_STATE_IN_DATA = 0x1,    /**< IN DATA */
    USB_EP0_STATE_OUT_DATA = 0x3,   /**< OUT DATA */
    USB_EP0_STATE_IN_STATUS = 0x4,  /**< IN status */
    USB_EP0_STATE_OUT_STATUS = 0x5, /**< OUT status */
    USB_EP0_STATE_IN_ZLP = 0x6,     /**< OUT status */
    USB_EP0_STATE_STALL = 0x7,      /**< STALL status */
} ep0_state_t;

/* Endpoint state */
struct musb_ep_state {
    uint16_t ep_mps;    /* Endpoint max packet size */
    uint8_t ep_type;    /* Endpoint type */
    uint8_t ep_stalled; /* Endpoint stall flag */
    uint8_t ep_enable;  /* Endpoint enable */
    uint8_t *xfer_buf;
    uint32_t xfer_len;
    uint32_t actual_xfer_len;
};

/* Driver state */
struct musb_udc {
    volatile uint8_t dev_addr;
    __attribute__((aligned(32))) struct usb_setup_packet setup;
    struct musb_ep_state in_ep[CONFIG_USBDEV_EP_NUM];  /*!< IN endpoint parameters*/
    struct musb_ep_state out_ep[CONFIG_USBDEV_EP_NUM]; /*!< OUT endpoint parameters */
} g_musb_udc;

static volatile uint8_t usb_ep0_state = USB_EP0_STATE_SETUP;

/* get current active ep */
static uint8_t musb_get_active_ep(void)
{
    return HWREGB(USB_BASE + MUSB_EPIDX_OFFSET);
}

/* set the active ep */
static void musb_set_active_ep(uint8_t ep_index)
{
    HWREGB(USB_BASE + MUSB_EPIDX_OFFSET) = ep_index;
}

static void musb_write_packet(uint8_t ep_idx, uint8_t *buffer, uint16_t len)
{
    uint8_t *buf8;
    int i;

    buf8 = buffer;
    for (i = 0; i < len; i++) {
        HWREGB(USB_FIFO_BASE(ep_idx)) = *buf8++;
    }
}

static void musb_read_packet(uint8_t ep_idx, uint8_t *buffer, uint16_t len)
{
    uint8_t *buf8;
    int i;

    buf8 = buffer;
    for (i = 0; i < len; i++) {
        *buf8++ = HWREGB(USB_FIFO_BASE(ep_idx));
    }
}

__WEAK void usb_dc_low_level_init(void)
{
    // Enable USB clock
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_USB_AHB | CLK_AHBPeriph_USB_48M, ENABLE);

    // Enable USB Interrupts
    HWREGB(USB_BASE + MUSB_IE_OFFSET) = USB_IE_RESET | USB_IE_SUSPND | USB_IE_RESUME;
#if CONFIG_USBDEV_PLUG_INT_EN
    HWREGB(USB_BASE + MUSB_IE_OFFSET) |= USB_IE_PLUG;
#endif
#if CONFIG_USBDEV_SOF_INT_EN
    HWREGB(USB_BASE + MUSB_IE_OFFSET) |= USB_IE_SOF;
#endif

    // Configure NVIC USB Interrupt
    NVIC_SetPriority(USB_IRQn, 0);
    NVIC_EnableIRQ(USB_IRQn);
    
    /* Set USB plug in/out debounce time */
    SYS->CTRL1 = ((SYS->CTRL1 & ~0xffff) | 100);

    // Enable USB internal pullup path
    ANA->ANA_MISC_3V |= ((0x1 << 27) | (0x1 << 28));

    // Enable USB module
    ANA->ANA_MISC_3V |= ((0X1 << 26));
}

__WEAK void usb_dc_low_level_deinit(void)
{
    // Disable USB clock
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_USB_AHB | CLK_AHBPeriph_USB_48M, DISABLE);

    // Disable USB inernal pullup path and usb module
    ANA->ANA_MISC_3V &= ~((0X1 << 26) | (0x1 << 27) | (0x1 << 28));

    /* Then delay 200ms to trigger usb plug-out event */
}

int usb_dc_init(uint8_t busid)
{
    usb_dc_low_level_init();

    musb_set_active_ep(0);
    HWREGB(USB_BASE + MUSB_FADDR_OFFSET) = 0;

    /* Enable USB interrupts */
//    HWREGB(USB_BASE + MUSB_IE_OFFSET) = USB_IE_PLUG | USB_IE_SOF | USB_IE_RESET | USB_IE_SUSPND | USB_IE_RESUME;
    HWREGB(USB_BASE + MUSB_TXIE_OFFSET) = USB_TXIE_EP0;
    HWREGB(USB_BASE + MUSB_RXIE_OFFSET) = 0;

    return 0;
}

int usb_dc_deinit(uint8_t busid)
{
    usb_dc_low_level_deinit();

    return 0;
}

int usbd_set_address(uint8_t busid, const uint8_t addr)
{
    if (addr == 0) {
        HWREGB(USB_BASE + MUSB_FADDR_OFFSET) = 0;
    }

    g_musb_udc.dev_addr = addr;
    return 0;
}

int usbd_set_remote_wakeup(uint8_t busid)
{
    HWREGB(USB_BASE + MUSB_POWER_OFFSET) |= USB_POWER_RESUME;
    usbd_musb_delay_ms(10);
    HWREGB(USB_BASE + MUSB_POWER_OFFSET) &= ~USB_POWER_RESUME;
    return 0;
}

#if 0
uint8_t usbd_get_port_speed(uint8_t busid)
{
    uint8_t speed = USB_SPEED_UNKNOWN;

    if (HWREGB(USB_BASE + MUSB_POWER_OFFSET) & USB_POWER_HSMODE)
        speed = USB_SPEED_HIGH;
    else if (HWREGB(USB_BASE + MUSB_DEVCTL_OFFSET) & USB_DEVCTL_FSDEV)
        speed = USB_SPEED_FULL;
    else if (HWREGB(USB_BASE + MUSB_DEVCTL_OFFSET) & USB_DEVCTL_LSDEV)
        speed = USB_SPEED_LOW;

    return speed;
}
#endif

int usbd_ep_open(uint8_t busid, const struct usb_endpoint_descriptor *ep)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep->bEndpointAddress);
    uint8_t old_ep_idx;
    uint32_t ui32Flags = 0;
    uint16_t ui32Register = 0;

    if (ep_idx == 0) {
        g_musb_udc.out_ep[0].ep_mps = USB_CTRL_EP_MPS;
        g_musb_udc.out_ep[0].ep_type = 0x00;
        g_musb_udc.out_ep[0].ep_enable = true;
        g_musb_udc.in_ep[0].ep_mps = USB_CTRL_EP_MPS;
        g_musb_udc.in_ep[0].ep_type = 0x00;
        g_musb_udc.in_ep[0].ep_enable = true;
        return 0;
    }

    if (ep_idx > (CONFIG_USBDEV_EP_NUM - 1)) {
        USB_LOG_ERR("Ep addr %02x overflow\r\n", ep->bEndpointAddress);
        return -1;
    }

    old_ep_idx = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep->bEndpointAddress)) {
        g_musb_udc.out_ep[ep_idx].ep_mps = USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize);
        g_musb_udc.out_ep[ep_idx].ep_type = USB_GET_ENDPOINT_TYPE(ep->bmAttributes);
        g_musb_udc.out_ep[ep_idx].ep_enable = true;

        if (musb_device_table[ep_idx].maxpacket < g_musb_udc.out_ep[ep_idx].ep_mps) {
            USB_LOG_ERR("Ep %02x fifo is overflow\r\n", ep->bEndpointAddress);
            return -2;
        }

        /* Round up max packet size to a multiple of 8 for writing to MaxP registers */
        HWREGB(USB_BASE + MUSB_IND_RXMAP_OFFSET) = (uint8_t)((ep->wMaxPacketSize + 7) >> 3);

        //
        // Allow auto clearing of RxPktRdy when packet of size max packet
        // has been unloaded from the FIFO.
        //
        if (ui32Flags & USB_EP_AUTO_CLEAR) {
            ui32Register = USB_RXCSRH1_AUTOCL;
        }
#if 0
        //
        // Configure the DMA mode.
        //
        if (ui32Flags & USB_EP_DMA_MODE_1) {
            ui32Register |= USB_RXCSRH1_DMAEN | USB_RXCSRH1_DMAMOD;
        } else if (ui32Flags & USB_EP_DMA_MODE_0) {
            ui32Register |= USB_RXCSRH1_DMAEN;
        }
#endif
        //
        // Enable isochronous mode if requested.
        //
        if (USB_GET_ENDPOINT_TYPE(ep->bmAttributes) == 0x01) {
            ui32Register |= USB_RXCSRH1_ISO;
        }

        HWREGB(USB_BASE + MUSB_IND_RXCSRH_OFFSET) = ui32Register;

        // Reset the Data toggle to zero.
        if (HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) & USB_RXCSRL1_RXRDY)
            HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) = (USB_RXCSRL1_CLRDT | USB_RXCSRL1_FLUSH);
        else
            HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) = USB_RXCSRL1_CLRDT;
    } else {
        g_musb_udc.in_ep[ep_idx].ep_mps = USB_GET_MAXPACKETSIZE(ep->wMaxPacketSize);
        g_musb_udc.in_ep[ep_idx].ep_type = USB_GET_ENDPOINT_TYPE(ep->bmAttributes);
        g_musb_udc.in_ep[ep_idx].ep_enable = true;

        if (musb_device_table[ep_idx].maxpacket < g_musb_udc.in_ep[ep_idx].ep_mps) {
            USB_LOG_ERR("Ep %02x fifo is overflow\r\n", ep->bEndpointAddress);
            return -2;
        }

        /* Round up max packet size to a multiple of 8 for writing to MaxP registers */
        HWREGB(USB_BASE + MUSB_IND_TXMAP_OFFSET) = (uint8_t)((ep->wMaxPacketSize + 7) >> 3);

        //
        // Allow auto setting of TxPktRdy when max packet size has been loaded
        // into the FIFO.
        //
        if (ui32Flags & USB_EP_AUTO_SET) {
            ui32Register |= USB_TXCSRH1_AUTOSET;
        }
#if 0
        //
        // Configure the DMA mode.
        //
        if (ui32Flags & USB_EP_DMA_MODE_1) {
            ui32Register |= USB_TXCSRH1_DMAEN | USB_TXCSRH1_DMAMOD;
        } else if (ui32Flags & USB_EP_DMA_MODE_0) {
            ui32Register |= USB_TXCSRH1_DMAEN;
        }
#endif
        //
        // Enable isochronous mode if requested.
        //
        if (USB_GET_ENDPOINT_TYPE(ep->bmAttributes) == 0x01) {
            ui32Register |= USB_TXCSRH1_ISO;
        }

        HWREGB(USB_BASE + MUSB_IND_TXCSRH_OFFSET) = ui32Register;

        // Reset the Data toggle to zero.
        if (HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) & USB_TXCSRL1_TXRDY)
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = (USB_TXCSRL1_CLRDT | USB_TXCSRL1_FLUSH);
        else
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_CLRDT;
    }

    musb_set_active_ep(old_ep_idx);

    return 0;
}

int usbd_ep_close(uint8_t busid, const uint8_t ep)
{
    return 0;
}

int usbd_ep_set_stall(uint8_t busid, const uint8_t ep)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    old_ep_idx = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep)) {
        if (ep_idx == 0x00) {
            usb_ep0_state = USB_EP0_STATE_STALL;
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) |= (USB_CSRL0_STALL | USB_CSRL0_RXRDYC);
        } else {
            HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) |= USB_RXCSRL1_STALL;
        }
    } else {
        if (ep_idx == 0x00) {
            usb_ep0_state = USB_EP0_STATE_STALL;
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) |= (USB_CSRL0_STALL | USB_CSRL0_RXRDYC);
        } else {
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) |= USB_TXCSRL1_STALL;
        }
    }

    musb_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_clear_stall(uint8_t busid, const uint8_t ep)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    old_ep_idx = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep)) {
        if (ep_idx == 0x00) {
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_STALLED;
        } else {
            // Clear the stall on an OUT endpoint.
            HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~(USB_RXCSRL1_STALL | USB_RXCSRL1_STALLED);
            // Reset the data toggle.
            HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) |= USB_RXCSRL1_CLRDT;
        }
    } else {
        if (ep_idx == 0x00) {
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_STALLED;
        } else {
            // Clear the stall on an IN endpoint.
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~(USB_TXCSRL1_STALL | USB_TXCSRL1_STALLED);
            // Reset the data toggle.
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) |= USB_TXCSRL1_CLRDT;
        }
    }

    musb_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_is_stalled(uint8_t busid, const uint8_t ep, uint8_t *stalled)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    old_ep_idx = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (USB_EP_DIR_IS_OUT(ep)) {
        if(HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) & USB_RXCSRL1_STALL) {
            *stalled = 1;
        } else {
            *stalled = 0;
        }
    } else {
        if(HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) & USB_TXCSRL1_STALL) {
            *stalled = 1;
        } else {
            *stalled = 0;
        }
    }
    musb_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_start_write(uint8_t busid, const uint8_t ep, const uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;
    uint8_t data_len_trim;

    if (!data && data_len) {
        return -1;
    }
    if (!g_musb_udc.in_ep[ep_idx].ep_enable) {
        return -2;
    }

    old_ep_idx = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    if (ep_idx != 0x00) {
        if (HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) & USB_TXCSRL1_TXRDY) {
            musb_set_active_ep(old_ep_idx);
            return -3;
        }
    }

    g_musb_udc.in_ep[ep_idx].xfer_buf = (uint8_t *)data;
    g_musb_udc.in_ep[ep_idx].xfer_len = data_len;
    g_musb_udc.in_ep[ep_idx].actual_xfer_len = 0;

    if (data_len == 0) {
        if (ep_idx == 0x00) {
            if (g_musb_udc.setup.wLength == 0) {
                usb_ep0_state = USB_EP0_STATE_IN_STATUS;
            } else {
                usb_ep0_state = USB_EP0_STATE_IN_ZLP;
            }
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_RXRDYC;
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) |= (USB_CSRL0_TXRDY | USB_CSRL0_DATAEND);
        } else {
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_TXRDY;
            HWREGB(USB_BASE + MUSB_TXIE_OFFSET) |= (1 << ep_idx);
        }
        musb_set_active_ep(old_ep_idx);
        return 0;
    }
    data_len_trim = MIN(data_len, g_musb_udc.in_ep[ep_idx].ep_mps);

    musb_write_packet(ep_idx, (uint8_t *)data, data_len_trim);
    HWREGB(USB_BASE + MUSB_TXIE_OFFSET) |= (1 << ep_idx);

    if (ep_idx == 0x00) {
        usb_ep0_state = USB_EP0_STATE_IN_DATA;
        if ((data_len < g_musb_udc.in_ep[ep_idx].ep_mps) ||
            ((data_len == g_musb_udc.in_ep[ep_idx].ep_mps) && (g_llp_flag == false))) {
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = (USB_CSRL0_TXRDY | USB_CSRL0_DATAEND);
        } else {
            HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_TXRDY;
        }
    } else {
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_TXRDY;
    }

    musb_set_active_ep(old_ep_idx);
    return 0;
}

int usbd_ep_start_read(uint8_t busid, const uint8_t ep, uint8_t *data, uint32_t data_len)
{
    uint8_t ep_idx = USB_EP_GET_IDX(ep);
    uint8_t old_ep_idx;

    if (!data && data_len) {
        return -1;
    }
    if (!g_musb_udc.out_ep[ep_idx].ep_enable) {
        return -2;
    }

    old_ep_idx = musb_get_active_ep();
    musb_set_active_ep(ep_idx);

    g_musb_udc.out_ep[ep_idx].xfer_buf = data;
    g_musb_udc.out_ep[ep_idx].xfer_len = data_len;
    g_musb_udc.out_ep[ep_idx].actual_xfer_len = 0;

    if (data_len == 0) {
        if (ep_idx == 0) {
            usb_ep0_state = USB_EP0_STATE_SETUP;
        }
        musb_set_active_ep(old_ep_idx);
        return 0;
    }
    if (ep_idx == 0) {
        usb_ep0_state = USB_EP0_STATE_OUT_DATA;
    } else {
        // Disable usb Rx interrupt after receiving expected number of data
        HWREGB(USB_BASE + MUSB_RXIE_OFFSET) |= (1 << ep_idx);
        // Clear OutPktRdy flag to indicate bus to receive new usb OUT packet from host to device Rx FIFO
        HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~(USB_RXCSRL1_RXRDY);
    }
    musb_set_active_ep(old_ep_idx);
    return 0;
}

static void handle_ep0(void)
{
    uint8_t ep0_status = HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET);
    uint16_t read_count;

    if (ep0_status & USB_CSRL0_STALLED) {
        // Clear SentStall bit, State -> IDLE
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_CSRL0_STALLED;
        usb_ep0_state = USB_EP0_STATE_SETUP;
        return;
    }

    if (ep0_status & USB_CSRL0_SETEND) {
        // Set ServicedSetupEnd bit, State -> IDLE
        HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_SETENDC;
        usb_ep0_state = USB_EP0_STATE_SETUP;
    }

    if (g_musb_udc.dev_addr > 0) {
        HWREGB(USB_BASE + MUSB_FADDR_OFFSET) = g_musb_udc.dev_addr;
        g_musb_udc.dev_addr = 0;
    }

    switch (usb_ep0_state) {
        case USB_EP0_STATE_SETUP:
            if (ep0_status & USB_CSRL0_RXRDY) {
                read_count = HWREGH(USB_BASE + MUSB_IND_RXCOUNT_OFFSET);

                if (read_count != 8) {
                    return;
                }

                musb_read_packet(0, (uint8_t *)&g_musb_udc.setup, 8);
                if (g_musb_udc.setup.wLength) {
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_RXRDYC;
                } else {
                    // Zero Data Request detected, but here we do not set RXRDYC and DATAEND bit
                    // as we'll handle this case in function usbd_ep_start_write() later.
                }
                usbd_event_ep0_setup_complete_handler(0, (uint8_t *)&g_musb_udc.setup);
            }
            break;

        case USB_EP0_STATE_IN_DATA:
            if (g_musb_udc.in_ep[0].xfer_len > g_musb_udc.in_ep[0].ep_mps) {
                g_musb_udc.in_ep[0].actual_xfer_len += g_musb_udc.in_ep[0].ep_mps;
                g_musb_udc.in_ep[0].xfer_len -= g_musb_udc.in_ep[0].ep_mps;
            } else {
                g_musb_udc.in_ep[0].actual_xfer_len += g_musb_udc.in_ep[0].xfer_len;
                g_musb_udc.in_ep[0].xfer_len = 0;
            }
            usbd_event_ep_in_complete_handler(0, 0x80, g_musb_udc.in_ep[0].actual_xfer_len);
            break;
        case USB_EP0_STATE_OUT_DATA:
            if (ep0_status & USB_CSRL0_RXRDY) {
                read_count = HWREGH(USB_BASE + MUSB_IND_RXCOUNT_OFFSET);

                musb_read_packet(0, g_musb_udc.out_ep[0].xfer_buf, read_count);
                g_musb_udc.out_ep[0].xfer_buf += read_count;
                g_musb_udc.out_ep[0].actual_xfer_len += read_count;

                if (read_count < g_musb_udc.out_ep[0].ep_mps) {
                    usbd_event_ep_out_complete_handler(0, 0x00, g_musb_udc.out_ep[0].actual_xfer_len);
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = (USB_CSRL0_RXRDYC | USB_CSRL0_DATAEND);
                    usb_ep0_state = USB_EP0_STATE_IN_STATUS;
                } else {
                    HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_CSRL0_RXRDYC;
                }
            }
            break;
        case USB_EP0_STATE_IN_STATUS:
        case USB_EP0_STATE_IN_ZLP:
            usb_ep0_state = USB_EP0_STATE_SETUP;
            usbd_event_ep_in_complete_handler(0, 0x80, 0);
            break;
    }
}

void USBD_IRQHandler(uint8_t busid)
{
    uint32_t is;
    uint32_t txis;
    uint32_t rxis;
    uint8_t old_ep_idx;
    uint8_t ep_idx;
    uint16_t write_count, read_count;

    is = HWREGB(USB_BASE + MUSB_IS_OFFSET);
    txis = HWREGB(USB_BASE + MUSB_TXIS_OFFSET);
    rxis = HWREGB(USB_BASE + MUSB_RXIS_OFFSET);

//    HWREGB(USB_BASE + MUSB_IS_OFFSET) = is;

    old_ep_idx = musb_get_active_ep();

    /* Receive a reset signal from the USB bus */
    if (is & USB_IS_RESET) {
        memset(&g_musb_udc, 0, sizeof(struct musb_udc));
        usbd_event_reset_handler(0);
        HWREGB(USB_BASE + MUSB_TXIE_OFFSET) = USB_TXIE_EP0;
        HWREGB(USB_BASE + MUSB_RXIE_OFFSET) = 0;

        usb_ep0_state = USB_EP0_STATE_SETUP;
    }

    if (is & USB_IS_PLUG) {
        if ((is & (USB_IS_DP | USB_IS_DM)) == (USB_IS_DP | USB_IS_DM)) {
            // Handle USB Plug-Out Event
            usbd_event_disconnect_handler(0);
        } else if (is & (USB_IS_DP | USB_IS_DM)) {
            // Handle USB Plug-In Event
            usbd_event_connect_handler(0);
        }
    }

    if (is & USB_IS_SOF) {
        usbd_event_sof_handler(0);
    }

    if (is & USB_IS_RESUME) {
        usbd_event_resume_handler(0);
    }

    if (is & USB_IS_SUSPEND) {
        usbd_event_suspend_handler(0);
    }

    txis &= HWREGB(USB_BASE + MUSB_TXIE_OFFSET);
    /* Handle EP0 interrupt */
    if (txis & USB_TXIE_EP0) {
//        HWREGB(USB_BASE + MUSB_TXIS_OFFSET) = USB_TXIE_EP0;
        musb_set_active_ep(0);
        handle_ep0();
        txis &= ~USB_TXIE_EP0;
    }

    ep_idx = 1;
    while (txis) {
        if (txis & (1 << ep_idx)) {
            musb_set_active_ep(ep_idx);
//            HWREGB(USB_BASE + MUSB_TXIS_OFFSET) = (1 << ep_idx);
            if (HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) & USB_TXCSRL1_UNDRN) {
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) &= ~USB_TXCSRL1_UNDRN;
            }

            if (g_musb_udc.in_ep[ep_idx].xfer_len > g_musb_udc.in_ep[ep_idx].ep_mps) {
                g_musb_udc.in_ep[ep_idx].xfer_buf += g_musb_udc.in_ep[ep_idx].ep_mps;
                g_musb_udc.in_ep[ep_idx].actual_xfer_len += g_musb_udc.in_ep[ep_idx].ep_mps;
                g_musb_udc.in_ep[ep_idx].xfer_len -= g_musb_udc.in_ep[ep_idx].ep_mps;
            } else {
                g_musb_udc.in_ep[ep_idx].xfer_buf += g_musb_udc.in_ep[ep_idx].xfer_len;
                g_musb_udc.in_ep[ep_idx].actual_xfer_len += g_musb_udc.in_ep[ep_idx].xfer_len;
                g_musb_udc.in_ep[ep_idx].xfer_len = 0;
            }

            if (g_musb_udc.in_ep[ep_idx].xfer_len == 0) {
                HWREGB(USB_BASE + MUSB_TXIE_OFFSET) &= ~(1 << ep_idx);
                usbd_event_ep_in_complete_handler(0, ep_idx | 0x80, g_musb_udc.in_ep[ep_idx].actual_xfer_len);
            } else {
                write_count = MIN(g_musb_udc.in_ep[ep_idx].xfer_len, g_musb_udc.in_ep[ep_idx].ep_mps);

                musb_write_packet(ep_idx, g_musb_udc.in_ep[ep_idx].xfer_buf, write_count);
                HWREGB(USB_BASE + MUSB_IND_TXCSRL_OFFSET) = USB_TXCSRL1_TXRDY;
            }

            txis &= ~(1 << ep_idx);
        }
        ep_idx++;
    }

    rxis &= HWREGB(USB_BASE + MUSB_RXIE_OFFSET);
    ep_idx = 1;
    while (rxis) {
        if (rxis & (1 << ep_idx)) {
            musb_set_active_ep(ep_idx);
//            HWREGB(USB_BASE + MUSB_RXIS_OFFSET) = (1 << ep_idx);
            if (HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) & USB_RXCSRL1_RXRDY) {
#if 1
                read_count = HWREGH(USB_BASE + MUSB_IND_RXCOUNT_OFFSET);
#else
                read_count = HWREGB(USB_BASE + MUSB_IND_RXCOUNT_OFFSET + 1);
                read_count <<= 8;
                read_count |= HWREGB(USB_BASE + MUSB_IND_RXCOUNT_OFFSET);
#endif
                musb_read_packet(ep_idx, g_musb_udc.out_ep[ep_idx].xfer_buf, read_count);
//                HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~(USB_RXCSRL1_RXRDY);

                g_musb_udc.out_ep[ep_idx].xfer_buf += read_count;
                g_musb_udc.out_ep[ep_idx].actual_xfer_len += read_count;
                g_musb_udc.out_ep[ep_idx].xfer_len -= read_count;

                if ((read_count < g_musb_udc.out_ep[ep_idx].ep_mps) || (g_musb_udc.out_ep[ep_idx].xfer_len == 0)) {
                    // Disable usb Rx interrupt after receiving expected number of data
                    // NOTE: Here we don't clear OutPktRdy flag, and this indicate bus to avoid receiving
                    //       new usb OUT packet from host (Respond PID NAK to host to require retransmission)
                    HWREGB(USB_BASE + MUSB_RXIE_OFFSET) &= ~(1 << ep_idx);
                    usbd_event_ep_out_complete_handler(0, ep_idx, g_musb_udc.out_ep[ep_idx].actual_xfer_len);
                } else {
                    // Clear OutPktRdy flag to indicate bus to receive new usb OUT packet from host to device Rx FIFO
                    HWREGB(USB_BASE + MUSB_IND_RXCSRL_OFFSET) &= ~(USB_RXCSRL1_RXRDY);
                }
            }

            rxis &= ~(1 << ep_idx);
        }
        ep_idx++;
    }

    musb_set_active_ep(old_ep_idx);
}

void USB_IRQHandler(void)
{
    USBD_IRQHandler(0);
}
