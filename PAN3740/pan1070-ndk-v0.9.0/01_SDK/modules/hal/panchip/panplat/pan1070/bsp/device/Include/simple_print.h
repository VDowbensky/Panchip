#ifndef __SIMPLE_PRINT_H__
#define __SIMPLE_PRINT_H__

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * Helper Macro Functions for simple printf-like log output mechanism
 ******************************************************************************/
#ifndef CONFIG_SIMPLE_PRINT_ENABLE
#define CONFIG_SIMPLE_PRINT_ENABLE              1
#endif

#if CONFIG_SIMPLE_PRINT_ENABLE
#define SIMPLE_PRINTS(prefix, str, suffix)      prints(prefix, str, suffix)
#define SIMPLE_PRINTU(prefix, num, suffix)      printu(prefix, num, suffix)
#define SIMPLE_PRINTX(prefix, num, suffix)      printx(prefix, num, suffix)
#else
#define SIMPLE_PRINTS(prefix, str, suffix)
#define SIMPLE_PRINTU(prefix, num, suffix)
#define SIMPLE_PRINTX(prefix, num, suffix)
#endif

/*******************************************************************************
 * Simple printf-like log output implementations
 ******************************************************************************/
/**
 * @brief Output an string with possible prefix and suffix
 *
 * Output a string to UART or RTT.
 * e.g. prints("PANCHIP", "Hello ", " World!\n") => "Hello PANCHIP World!\n"
 *
 * @param str       string to output
 * @param prefix    prefix string printing out before 'str', pass NULL if not needed
 * @param suffix    suffix string printing out after 'str', pass NULL if not needed
 *
 * @note This function is equal with %s-format-specifier of printk() and printf(),
 *       but can significantly save the stack usage.
 *       Actually the stack use of the 3 APIs are:
 *       - Function Name        : prints()  printk("%s")  printf("%s")
 *       - Stack Depth (Bytes)  : 16        128           188
 *
 * @return N/A
 */
void prints(const char* prefix, const char *str, const char* suffix);

/**
 * @brief Output an integer in decimal format with possible prefix and suffix
 *
 * Output a decimal integer (with possible prefix and/or suffix) to UART or RTT.
 * e.g. printu(200, "number = ", "\n") => "number = 200\n"
 *
 * @param num       32-bit integer to output
 * @param prefix    prefix string printing out before 'num', pass NULL if not needed
 * @param suffix    suffix string printing out after 'num', pass NULL if not needed
 *
 * @note This function is equal with %u-format-specifier of printk() and printf(),
 *       but can significantly save the stack usage.
 *       Actually the stack use of the 3 APIs are:
 *       - Function Name        : printu()  printk("%u")  printf("%u")
 *       - Stack Depth (Bytes)  : 76        132           208
 *
 * @return N/A
 */
void printu(const char* prefix, uint32_t num, const char* suffix);

/**
 * @brief Output an integer in hexadecimal format with possible prefix and suffix
 *
 * Output a hexadecimal integer (with possible prefix and/or suffix) to UART or RTT.
 * e.g. printx(200, "number = ", "\n") => "number = 0xC8\n"
 *
 * @param num       32-bit integer to output
 * @param prefix    prefix string printing out before 'num', pass NULL if not needed
 * @param suffix    suffix string printing out after 'num', pass NULL if not needed
 *
 * @note This function is equal with %X-format-specifier of printk() and printf(),
 *       but can significantly save the stack usage.
 *       Actually the stack use of the 3 APIs are:
 *       - Function Name        : printx()  printk("%X")  printf("%X")
 *       - Stack Depth (Bytes)  : 76        132           208
 *
 * @return N/A
 */
void printx(const char* prefix, uint32_t num, const char* suffix);

#endif // __SIMPLE_PRINT_H__
