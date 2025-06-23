/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __CBPRINTF_H__
#define __CBPRINTF_H__

#include <cmsis_compiler.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __sizeof_ptr == 8
typedef signed long ssize_t;
#else
typedef signed int ssize_t;
#endif

/**
 * @defgroup cbprintf_apis Formatted Output APIs
 * @ingroup support_apis
 * @{
 */

/** @brief Signature for a cbprintf callback function.
 *
 * This function expects two parameters:
 *
 * * @p c a character to output.  The output behavior should be as if
 *   this was cast to an unsigned char.
 * * @p ctx a pointer to an object that provides context for the
 *   output operation.
 *
 * The declaration does not specify the parameter types.  This allows a
 * function like @c fputc to be used without requiring all context pointers to
 * be to a @c FILE object.
 *
 * @return the value of @p c cast to an unsigned char then back to
 * int, or a negative error code that will be returned from
 * cbprintf().
 */
typedef int (*cbprintf_cb)(/* int c, void *ctx */);


/** @brief *printf-like output through a callback.
 *
 * This is essentially printf() except the output is generated
 * character-by-character using the provided @p out function.  This allows
 * formatting text of unbounded length without incurring the cost of a
 * temporary buffer.
 *
 * All formatting specifiers of C99 are recognized, and most are supported if
 * the functionality is enabled.
 *
 * @param out the function used to emit each generated character.
 *
 * @param ctx context provided when invoking out
 *
 * @param format a standard ISO C format string with characters and conversion
 * specifications.
 *
 * @param ... arguments corresponding to the conversion specifications found
 * within @p format.
 *
 * @return the number of characters printed, or a negative error value
 * returned from invoking @p out.
 */
int cbprintf(cbprintf_cb out, void *ctx, const char *format, ...);

/** @brief varargs-aware *printf-like output through a callback.
 *
 * This is essentially vsprintf() except the output is generated
 * character-by-character using the provided @p out function.  This allows
 * formatting text of unbounded length without incurring the cost of a
 * temporary buffer.
 *
 * @param out the function used to emit each generated character.
 *
 * @param ctx context provided when invoking out
 *
 * @param format a standard ISO C format string with characters and conversion
 * specifications.
 *
 * @param ap a reference to the values to be converted.
 *
 * @return the number of characters generated, or a negative error value
 * returned from invoking @p out.
 */
int cbvprintf(cbprintf_cb out, void *ctx, const char *format, va_list ap);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CBPRINTF_H__ */
