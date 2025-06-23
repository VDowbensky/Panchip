/*
 * Copyright (c) 2010, 2013-2014 Wind River Systems, Inc.
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PanSeries.h"
#include <cmsis_compiler.h>
#include <stdarg.h>
#include <cbprintf.h>
#include <printk.h>

/**
 * @file
 * @brief Low-level debug output
 *
 * Low-level debugging output. Platform installs a character output routine at
 * init time. If no routine is installed, a nop routine is called.
 */

/**
 * @brief Default character output routine that does nothing
 * @param c Character to swallow
 *
 * Note this is defined as a weak symbol, allowing architecture code
 * to override it where possible to enable very early logging.
 *
 * @return 0
 */
/* LCOV_EXCL_START */
__WEAK int arch_printk_char_out(int c)
{
    /*
     * Here we just call the low-level driver SendChar() API in retarget.c
     * which can handle both UART log and RTT log
     */
    extern void SendChar(int ch);
    SendChar(c);

    return 0;
}
/* LCOV_EXCL_STOP */

int (*_char_out)(int) = arch_printk_char_out;

/**
 * @brief Install the character output routine for printk
 *
 * To be called by the platform's console driver at init time. Installs a
 * routine that outputs one ASCII character at a time.
 * @param fn putc routine to install
 *
 * @return N/A
 */
void __printk_hook_install(int (*fn)(int))
{
    _char_out = fn;
}

/**
 * @brief Get the current character output routine for printk
 *
 * To be called by any console driver that would like to save
 * current hook - if any - for later re-installation.
 *
 * @return a function pointer or NULL if no hook is set
 */
void *__printk_get_hook(void)
{
    return _char_out;
}


struct out_context {
    int count;
};

static int char_out(int c, void *ctx_p)
{
    struct out_context *ctx = ctx_p;

    ctx->count++;
    return _char_out(c);
}


void vprintk(const char *fmt, va_list ap)
{
    struct out_context ctx = { 0 };

    cbvprintf(char_out, &ctx, fmt, ap);
}


/**
 * @brief Output a string
 *
 * Output a string on output installed by platform at init time. Some
 * printf-like formatting is available.
 *
 * Available formatting:
 * - %x/%X:  outputs a number in hexadecimal format
 * - %s:     outputs a null-terminated string
 * - %p:     pointer, same as %x with a 0x prefix
 * - %u:     outputs a number in unsigned decimal format
 * - %d/%i:  outputs a number in signed decimal format
 *
 * Field width (with or without leading zeroes) is supported.
 * Length attributes h, hh, l, ll and z are supported. However, integral
 * values with %lld and %lli are only printed if they fit in a long
 * otherwise 'ERR' is printed. Full 64-bit values may be printed with %llx.
 *
 * @param fmt formatted string to output
 *
 * @return N/A
 */
void printk(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    vprintk(fmt, ap);

    va_end(ap);
}


struct str_context {
    char *str;
    int count;
};

static int str_out(int c, struct str_context *ctx)
{
    if (ctx->str == NULL) {
        ctx->count++;
        return c;
    }

    ctx->str[ctx->count++] = c;

    return c;
}

int vsprintk(char *str, const char *fmt, va_list ap)
{
    struct str_context ctx = { str, 0 };

    cbvprintf(str_out, &ctx, fmt, ap);

    str[ctx.count] = '\0';

    return ctx.count;
}

int sprintk(char *str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsprintk(str, fmt, ap);
    va_end(ap);

    return ret;
}


struct nstr_context {
    char *str;
    int max;
    int count;
};

static int nstr_out(int c, struct nstr_context *ctx)
{
    if (ctx->str == NULL || ctx->count >= ctx->max) {
        ctx->count++;
        return c;
    }

    if (ctx->count == ctx->max - 1) {
        ctx->str[ctx->count++] = '\0';
    } else {
        ctx->str[ctx->count++] = c;
    }

    return c;
}

int vsnprintk(char *str, size_t size, const char *fmt, va_list ap)
{
    struct nstr_context ctx = { str, size, 0 };

    cbvprintf(nstr_out, &ctx, fmt, ap);

    if (ctx.count < ctx.max) {
        str[ctx.count] = '\0';
    }

    return ctx.count;
}

int snprintk(char *str, size_t size, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsnprintk(str, size, fmt, ap);
    va_end(ap);

    return ret;
}
