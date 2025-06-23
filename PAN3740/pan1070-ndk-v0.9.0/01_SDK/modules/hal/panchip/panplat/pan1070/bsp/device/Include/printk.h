/*
 * Copyright (c) 2010-2012, 2014 Wind River Systems, Inc.
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <cmsis_compiler.h>
#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @brief Print kernel debugging message.
 *
 * This routine prints a kernel debugging message to the system console.
 * Output is send immediately, without any mutual exclusion or buffering.
 *
 * A basic set of conversion specifier characters are supported:
 *   - signed decimal: \%d, \%i
 *   - unsigned decimal: \%u
 *   - unsigned hexadecimal: \%x (\%X is treated as \%x)
 *   - pointer: \%p
 *   - string: \%s
 *   - character: \%c
 *   - percent: \%\%
 *
 * Field width (with or without leading zeroes) is supported.
 * Length attributes h, hh, l, ll and z are supported. However, integral
 * values with %lld and %lli are only printed if they fit in a long
 * otherwise 'ERR' is printed. Full 64-bit values may be printed with %llx.
 * Flags / precision / floating-point attributes are not supported.
 *
 * @param fmt Format string.
 * @param ... Optional list of format arguments.
 *
 * @return N/A
 */
extern void printk(const char *fmt, ...);

extern void vprintk(const char *fmt, va_list ap);
extern int sprintk(char *str, const char *fmt, ...);
extern int vsprintk(char *str, const char *fmt, va_list ap);
extern int snprintk(char *str, size_t size, const char *fmt, ...);
extern int vsnprintk(char *str, size_t size, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* __PRINTK_H__ */
