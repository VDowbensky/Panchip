/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file at_server.c
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <at.h>
#include "fifo.h"
#include "hc32_ddl.h"
#include "ev_hc32f460_lqfp100_v2.h"
#include <at_log.h>
#include "pan_port.h"

#define LOG_TAG              "at.svr"

#define ESC_KEY                 0x1B
#define BACKSPACE_KEY           0x08
#define DELECT_KEY              0x7F

#ifdef AT_USING_SERVER

#define AT_CMD_CHAR_0                  '0'
#define AT_CMD_CHAR_9                  '9'
#define AT_CMD_QUESTION_MARK           '?'
#define AT_CMD_EQUAL_MARK              '='
#define AT_CMD_L_SQ_BRACKET            '['
#define AT_CMD_R_SQ_BRACKET            ']'
#define AT_CMD_L_ANGLE_BRACKET         '<'
#define AT_CMD_R_ANGLE_BRACKET         '>'
#define AT_CMD_COMMA_MARK              ','
#define AT_CMD_SEMICOLON               ';'
#define AT_CMD_CR                      '\r'
#define AT_CMD_LF                      '\n'

static struct at_server s_at_server;
static at_server_t p_at_server = &s_at_server;
static at_cmd_t cmd_table = PN_NULL;
static rt_size_t cmd_num;

extern rt_size_t at_utils_send(const void *buffer, rt_size_t   size);
extern void at_vprintf(const char *format, va_list args);
extern void at_vprintfln(const char *format, va_list args);
extern uint8_t tx_carrywave_flag;
/**
 * AT server send data to AT device
 *
 * @param format the input format
 */
void at_server_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	at_vprintf(format, args);
	va_end(args);
}

/**
 * AT server send data and newline to AT device
 *
 * @param format the input format
 */
void at_server_printfln(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	at_vprintfln(format, args);
	va_end(args);
}

/**
 * AT server request arguments parse arguments
 *
 * @param req_args request arguments
 * @param req_expr request expression
 *
 * @return  -1 : parse arguments failed
 *           0 : parse without match
 *          >0 : The number of arguments successfully parsed
 */
int at_req_parse_args(const char *req_args, const char *req_expr, ...)
{
	va_list args;
	int req_args_num = 0;

	RT_ASSERT(req_args);
	RT_ASSERT(req_expr);

	va_start(args, req_expr);
	req_args_num = vsscanf(req_args, req_expr, args);
	va_end(args);

	return req_args_num;
}

/**
 * AT server send command execute result to AT device
 *
 * @param result AT command execute result
 */
void at_server_print_result(at_result_t result)
{
	switch (result)
	{
	case AT_RESULT_OK:
		//at_server_printfln("");
		at_server_printfln("OK");
		break;

	case AT_RESULT_FAILE:
		//at_server_printfln("");
		at_server_printfln("ERROR");
		break;

	case AT_RESULT_NULL:
		break;

	case AT_RESULT_CMD_ERR:
		at_server_printfln("ERR CMD MATCH FAILED!");
		at_server_print_result(AT_RESULT_FAILE);
		break;

	case AT_RESULT_CHECK_FAILE:
		at_server_printfln("ERR CHECK ARGS FORMAT FAILED!");
		at_server_print_result(AT_RESULT_FAILE);
		break;

	case AT_RESULT_PARSE_FAILE:
		at_server_printfln("ERR PARSE ARGS FAILED!");
		at_server_print_result(AT_RESULT_FAILE);
		break;

	default:
		break;
	}
}

/**
 *  AT server print all commands to AT device
 */
void rt_at_server_print_all_cmd(void)
{
	rt_size_t i = 0;

	at_server_printfln("Commands list : ");

	for (i = 0; i < cmd_num; i++)
	{
		at_server_printf("%s", cmd_table[i].name);

		if (cmd_table[i].args_expr)
		{
			at_server_printfln("%s", cmd_table[i].args_expr);
		}
		else
		{
			at_server_printf("%c%c", AT_CMD_CR, AT_CMD_LF);
		}
	}
}

/**
 * Send data to AT Client by uart device.
 *
 * @param server current AT server object
 * @param buf   send data buffer
 * @param size  send fixed data size
 *
 * @return >0: send data size
 *         =0: send failed
 */
rt_size_t at_server_send(at_server_t server, const char *buf, rt_size_t size)
{
	RT_ASSERT(buf);

	if (server == PN_NULL)
	{
		LOG_E("input AT Server object is NULL, please create or get AT Server object!");
		return 0;
	}

	return at_utils_send(buf, size);
}

at_server_t at_get_server(void)
{
	RT_ASSERT(p_at_server);
	RT_ASSERT(p_at_server->status != AT_STATUS_UNINITIALIZED);

	return p_at_server;
}

static rt_err_t at_check_args(const char *args, const char *args_format)
{
	rt_size_t left_sq_bracket_num = 0, right_sq_bracket_num = 0;
	rt_size_t left_angle_bracket_num = 0, right_angle_bracket_num = 0;
	rt_size_t comma_mark_num = 0;
	rt_size_t i = 0;

	RT_ASSERT(args);
	RT_ASSERT(args_format);

	for (i = 0; i < strlen(args_format); i++)
	{
		switch (args_format[i])
		{
			case AT_CMD_L_SQ_BRACKET:
				left_sq_bracket_num++;
			break;

			case AT_CMD_R_SQ_BRACKET:
				right_sq_bracket_num++;
			break;

			case AT_CMD_L_ANGLE_BRACKET:
				left_angle_bracket_num++;
			break;

			case AT_CMD_R_ANGLE_BRACKET:
				right_angle_bracket_num++;
			break;

			default:
			break;
		}
	}

    if (left_sq_bracket_num != right_sq_bracket_num || left_angle_bracket_num != right_angle_bracket_num
            || left_sq_bracket_num > left_angle_bracket_num)
    {
		return -RT_ERROR;
    }

    for (i = 0; i < strlen(args); i++)
    {
		if (args[i] == AT_CMD_COMMA_MARK)
		{
			comma_mark_num++;
		}
    }

    if ((comma_mark_num + 1 < left_angle_bracket_num - left_sq_bracket_num)
            || comma_mark_num + 1 > left_angle_bracket_num)
    {
		return -RT_ERROR;
    }

	return RT_EOK;
}

static rt_err_t at_cmd_process(at_cmd_t cmd, const char *cmd_args)
{
	at_result_t result = AT_RESULT_OK;

	RT_ASSERT(cmd);
	RT_ASSERT(cmd_args);

	if (cmd_args[0] == AT_CMD_EQUAL_MARK && cmd_args[1] == AT_CMD_QUESTION_MARK && cmd_args[2] == AT_CMD_CR)
	{
		if (cmd->test == PN_NULL)
		{
			at_server_print_result(AT_RESULT_CMD_ERR);
			return -RT_ERROR;
		}

		result = cmd->test();
		at_server_print_result(result);
	}
    else if (cmd_args[0] == AT_CMD_QUESTION_MARK && cmd_args[1] == AT_CMD_CR)
    {
        if (cmd->query == PN_NULL)
        {
            at_server_print_result(AT_RESULT_CMD_ERR);
            return -RT_ERROR;
        }

        result = cmd->query();
        at_server_print_result(result);
    }
    else if (cmd_args[0] == AT_CMD_EQUAL_MARK
            || (cmd_args[0] >= AT_CMD_CHAR_0 && cmd_args[0] <= AT_CMD_CHAR_9 && cmd_args[1] == AT_CMD_CR))
    {
        if (cmd->setup == PN_NULL)
        {
            at_server_print_result(AT_RESULT_CMD_ERR);
            return -RT_ERROR;
        }

        if(at_check_args(cmd_args, cmd->args_expr) < 0)
        {
            at_server_print_result(AT_RESULT_CHECK_FAILE);
            return -RT_ERROR;
        }

        result = cmd->setup(cmd_args);
        at_server_print_result(result);
    }
    else if (cmd_args[0] == AT_CMD_CR)
    {
        if (cmd->exec == PN_NULL)
        {
            at_server_print_result(AT_RESULT_CMD_ERR);
            return -RT_ERROR;
        }

        result = cmd->exec();
        at_server_print_result(result);
    }
    else
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

/*
 *  @brief letter toupper
 *  @param[in] char byte
 *  @param[out] char byte
 */
char toupper(char c)
{
	if( ( c >= 'a' ) && ( c <= 'z' ) )
		return c + ( 'A' - 'a' );
	return c;
}

static at_cmd_t at_find_cmd(const char *cmd)
{
	rt_size_t i = 0;

	RT_ASSERT(cmd_table);

	for (i = 0; i < cmd_num; i++)
	{
		if (!strcasecmp(cmd, cmd_table[i].name))
		{
			return &cmd_table[i];
		}
	}
	return PN_NULL;
}

static rt_err_t at_cmd_get_name(const char *cmd_buffer, char *cmd_name)
{
	rt_size_t cmd_name_len = 0, i = 0;

	RT_ASSERT(cmd_name);
	RT_ASSERT(cmd_buffer);

	for (i = 0; i < strlen(cmd_buffer) + 1; i++)
	{
		if (*(cmd_buffer + i) == AT_CMD_QUESTION_MARK || *(cmd_buffer + i) == AT_CMD_EQUAL_MARK
			|| *(cmd_buffer + i) == AT_CMD_CR)
		{
			cmd_name_len = i;
			if ( cmd_name_len >= AT_CMD_NAME_LEN )
			{
				break;
			}
			memcpy(cmd_name, cmd_buffer, cmd_name_len);
			*(cmd_name + cmd_name_len) = '\0';

			return RT_EOK;
		}
	}

	return -RT_ERROR;
}

static int at_get_char(char *ch)
{
	if ( uart_fifo_read((u8*)ch, 1) != 1 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static void at_server_parser(at_server_t server, char ch)
{
	at_cmd_t cur_cmd = PN_NULL;
	char *cur_cmd_args = PN_NULL;
	static char last_ch;

	RT_ASSERT(server);
	RT_ASSERT(server->status != AT_STATUS_UNINITIALIZED);

	if (server->echo_mode)
	{
		if (ch == AT_CMD_CR || (ch == AT_CMD_LF && last_ch != AT_CMD_CR))
		{
			at_server_printf("%c%c", AT_CMD_CR, AT_CMD_LF);
		}
		else if (ch == AT_CMD_LF)
		{
			// skip the end sign check
		}
		else if (ch == BACKSPACE_KEY || ch == DELECT_KEY)
		{
			if (server->cur_recv_len)
			{
				server->recv_buffer[--server->cur_recv_len] = 0;
				at_server_printf("\b \b");
			}
			
			return;
		}
		else
		{
			at_server_printf("%c", ch);
		}
	}

	last_ch = ch;
	server->recv_buffer[server->cur_recv_len++] = ch;
		
	server->cur_recv_len %= AT_SERVER_RECV_BUFF_LEN;

	char *cmd = server->recv_buffer;

	if ( server->cur_recv_len == 1 )
	{
		if ( cmd[0] != 'a' && cmd[0] != 'A' )
		{
			at_server_print_result(AT_RESULT_CMD_ERR);
			goto exit;
		}
	}
	else if ( server->cur_recv_len == 2 )
	{
		char ats[3];

		memcpy(ats, cmd, 2);
		ats[0] = toupper(ats[0]);
		ats[1] = toupper(ats[1]);
		ats[2] = 0;
		if ( !strstr(ats, "AT") )
		{
			at_server_print_result(AT_RESULT_CMD_ERR);
			goto exit;
		}
	}

	if(!strstr(server->recv_buffer, server->end_mark))
	{
		return;
	}

	/** letter to upper */
	for( int i = 0; i < server->cur_recv_len; i++ )
	{
		server->recv_buffer[i] = toupper(server->recv_buffer[i]);
	}

	static char cur_cmd_name[AT_CMD_NAME_LEN] = { 0 };
	if (at_cmd_get_name(server->recv_buffer, cur_cmd_name) < 0)
	{
		at_server_print_result(AT_RESULT_CMD_ERR);
		goto exit;
	}

	cur_cmd = at_find_cmd(cur_cmd_name);
	if (!cur_cmd)
	{
		at_server_print_result(AT_RESULT_CMD_ERR);
		goto exit;
	}

	cur_cmd_args = server->recv_buffer + strlen(cur_cmd_name);
	if (at_cmd_process(cur_cmd, cur_cmd_args) < 0)
	{
		goto exit;
	}

exit:
    memset(server->recv_buffer, 0x00, AT_SERVER_RECV_BUFF_LEN);
    server->cur_recv_len = 0;
}

#if defined(__ICCARM__) || defined(__ICCRX__)               /* for IAR compiler */
#pragma section="RtAtCmdTab"
#endif

void at_server_init(void)
{
    /* initialize the AT commands table.*/
#if defined(__CC_ARM)                                 /* ARM C Compiler */
    extern const int RtAtCmdTab$$Base;
    extern const int RtAtCmdTab$$Limit;
    cmd_table = (at_cmd_t)&RtAtCmdTab$$Base;
    cmd_num = (at_cmd_t)&RtAtCmdTab$$Limit - cmd_table;
#elif defined (__ICCARM__) || defined(__ICCRX__)      /* for IAR Compiler */
    cmd_table = (at_cmd_t)__section_begin("RtAtCmdTab");
    cmd_num = (at_cmd_t)__section_end("RtAtCmdTab") - cmd_table;
#elif defined (__GNUC__)                             /* for GCC Compiler */
    extern const int __rtatcmdtab_start;
    extern const int __rtatcmdtab_end;
    cmd_table = (at_cmd_t)&__rtatcmdtab_start;
    cmd_num = (at_cmd_t) &__rtatcmdtab_end - cmd_table;
#endif /* defined(__CC_ARM) */

    p_at_server->echo_mode = 0;
    p_at_server->status = AT_STATUS_UNINITIALIZED;

    memset(p_at_server->recv_buffer, 0x00, AT_SERVER_RECV_BUFF_LEN);
    p_at_server->cur_recv_len = 0;

    p_at_server->get_char = at_get_char;
    memcpy(p_at_server->end_mark, AT_CMD_END_MARK, sizeof(AT_CMD_END_MARK));

    uart_rx_fifo_flush();

	p_at_server->enable = 1;
	p_at_server->status = AT_STATUS_INITIALIZED;
    printf("AT server initialize success.");
}

void at_server_process(void)
{
    char ch;

    if ( p_at_server->status != AT_STATUS_INITIALIZED)
    {
		printf("AT err");
        return;
    }
	
    if(tx_carrywave_flag != 1)
	{
		rf_irq_process();
	}

    if ( p_at_server->get_char(&ch) > 0 )
    {
        at_server_parser(p_at_server, ch);
    }
}

void at_server_clean(void)
{
	p_at_server->enable = 0;
	memset(p_at_server->recv_buffer, 0x00, AT_SERVER_RECV_BUFF_LEN);
	p_at_server->cur_recv_len = 0;
	p_at_server->status = AT_STATUS_UNINITIALIZED;

	char ch;
	while ( p_at_server->get_char(&ch) );
}

RT_WEAK void at_port_reset(void)
{
	NVIC_SystemReset();
}

RT_WEAK void at_port_factory_reset(void)
{
	LOG_E("The factory reset for AT server is not implement.");
}

#endif /* AT_USING_SERVER */

