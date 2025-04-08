/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file at_utils.c
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#include <at.h>
#include <stdlib.h>
#include <stdio.h>
#include "hc32_ddl.h"

static char send_buf[AT_CMD_MAX_LEN];
static rt_size_t last_cmd_len = 0;

extern int App_Uart0DmaSend(u8 *buf, int len);

/**
 * dump hex format data to console device
 *
 * @param name name for hex object, it will show on log header
 * @param buf hex buffer
 * @param size buffer size
 */
void at_print_raw_cmd(const char *name, const char *buf, rt_size_t size)
{
#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')
#define WIDTH_SIZE           32

	rt_size_t i, j;

	for (i = 0; i < size; i += WIDTH_SIZE)
	{
		rt_kprintf("[D/AT] %s: %04X-%04X: ", name, i, i + WIDTH_SIZE);
		for (j = 0; j < WIDTH_SIZE; j++)
		{
			if (i + j < size)
			{
				rt_kprintf("%02X ", buf[i + j]);
			}
			else
			{
				rt_kprintf("   ");
			}
			if ((j + 1) % 8 == 0)
			{
				rt_kprintf(" ");
			}
		}
		rt_kprintf("  ");
		for (j = 0; j < WIDTH_SIZE; j++)
		{
			if (i + j < size)
			{
				rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
			}
		}
		rt_kprintf("\n");
	}
}

RT_WEAK rt_size_t at_utils_send(const void *buffer, rt_size_t size)
{
#ifdef DDL_UART_DMA_YES
	return App_Uart0DmaSend((uint8_t*)buffer, size);
#else
	return uart0_sendBuf((uint8_t*)buffer, size);
#endif
}

rt_size_t at_vprintf(const char *format, va_list args)
{
	last_cmd_len = vsnprintf(send_buf, sizeof(send_buf), format, args);
	if(last_cmd_len > sizeof(send_buf))
		last_cmd_len = sizeof(send_buf);

#ifdef AT_PRINT_RAW_CMD
	at_print_raw_cmd("sendline", send_buf, last_cmd_len);
#endif

	return at_utils_send(send_buf, last_cmd_len);
}

rt_size_t at_vprintfln(const char *format, va_list args)
{
	rt_size_t len;

	last_cmd_len = vsnprintf(send_buf, sizeof(send_buf) - 2, format, args);
	if(last_cmd_len > sizeof(send_buf) - 2)
		last_cmd_len = sizeof(send_buf) - 2;
	rt_memcpy(send_buf + last_cmd_len, "\r\n", 2);

	len = last_cmd_len + 2;

#ifdef AT_PRINT_RAW_CMD
	at_print_raw_cmd("sendline", send_buf, len);
#endif

	return at_utils_send(send_buf, len);
}


