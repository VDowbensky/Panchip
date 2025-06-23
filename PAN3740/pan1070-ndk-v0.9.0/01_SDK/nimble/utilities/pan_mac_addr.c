/**
 *******************************************************************************
 * @file     pan_mac_addr.c
 * @create   2024-11-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

 #include <stdint.h>
 #include "PanSeries.h"
 #include "app_log.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/


 /*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
extern uint8_t m_chip_mac[6]; // from soc.c

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/


/*******************************************************************************
 * Function Define
 ******************************************************************************/
uint8_t pan10x_mac_addr_get(uint8_t *mac)
{
	uint8_t addr_null[6] = {0, 0, 0, 0, 0, 0};

	if (memcmp(m_chip_mac, addr_null, 6) == 0) {
		APP_LOG_WRN("Warnning: No chip mac addr\n");
		return 1;
	} else {
		memcpy(mac, m_chip_mac, 6);
		return 0;
	}
}

uint8_t pan10x_roll_mac_addr_get(uint8_t *mac)
{
	uint8_t addr_temp[6];
	uint8_t addr_null[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	FMC_ReadInfoArea(FLCTL, 0x100, CMD_DREAD, addr_temp, sizeof(addr_temp));

	if (memcmp(addr_temp, addr_null, 6) == 0) {
		APP_LOG_WRN("Warnning: No chip roll mac addr\n");
		return 1;
	} else {
		memcpy(mac, addr_temp, 6);
		return 0;
	}
}

char *addr_to_str(const void *addr)
{
    static char buf[6 * 2 + 5 + 1];
    const uint8_t *u8p;

    u8p = addr;
    sprintk(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
            u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    return buf;
}
