/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file at_base_cmd.c
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#include <at.h>
#include <stdlib.h>
#include <string.h>

#ifdef AT_USING_SERVER

#define AT_ECHO_MODE_CLOSE             0
#define AT_ECHO_MODE_OPEN              1

extern at_server_t at_get_server(void);

static at_result_t at_exec(void)
{
	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT", PN_NULL, PN_NULL, PN_NULL, PN_NULL, at_exec);

static at_result_t at_show_cmd_exec(void)
{
	extern void rt_at_server_print_all_cmd(void);

	rt_at_server_print_all_cmd();

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT&L", PN_NULL, PN_NULL, PN_NULL, PN_NULL, at_show_cmd_exec);

#endif /* AT_USING_SERVER */

