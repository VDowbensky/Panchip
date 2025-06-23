#ifndef USB_DFU__H_
#define USB_DFU__H_

#include "PanSeries.h"
#include <stdint.h>
#include "musbfsfc.h"
#include "descript.h"
#include "endpoint.h"
#include "flash_manager.h"

// PAN107x SDK doesn't have a standalone controller image, this macro is just for code compatibility
#define CONTROLLER_HEADER_ADDR              0x0

#define APP_HEADER_ADDR                     FLASH_AREA_IMAGE_START
#define HEADER_SIZE                         APP_IMG_HEADER_SIZE
#define APP_ENTRY_ADDR                      APP_HEADER_ADDR + HEADER_SIZE

#define USB_DFU_ENTER_FLAG                  0x5A    // DFU flag, stores in last byte of bootloader partition

void usb_init(void);
bool get_plug_in_status(void);
void usb_self_plug_out(void);
void usb_self_plug_in(void);
void dfu_printf_info_init(void);
bool is_dfu_flag_valid(void);
void on_usb_dfu_enter(void);

#endif
