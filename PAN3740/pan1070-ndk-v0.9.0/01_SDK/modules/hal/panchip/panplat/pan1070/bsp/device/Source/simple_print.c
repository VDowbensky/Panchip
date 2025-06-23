/**
 *******************************************************************************
 * @file     simple_print.c
 * @create   2025-03-15
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2025 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "cmsis_compiler.h"
#include "simple_print.h"

/*******************************************************************************
 * Simple printf-like log output implementations
 ******************************************************************************/
#define ALPHA(fmt) (((fmt) & 0x60) - '0' - 10 + 1)

/* (Internal) Print out a single character */
__STATIC_FORCEINLINE void printchar(int c)
{
    /*
     * Here we just call the low-level driver SendChar() API in retarget.c
     * which can handle both UART log and RTT log
     */
    extern void SendChar(int ch);
    SendChar(c);
}

/* (Internal) Convert value to string, storing characters downwards */
__STATIC_FORCEINLINE int convert_value(uint32_t num, unsigned int base, unsigned int alpha, char *buftop)
{
    int i = 0;

    do {
        unsigned int c = num % base;
        if (c >= 10) {
            c += alpha;
        }
        buftop[--i] = c + '0';
        num /= base;
    } while (num);

    if (base == 16) {
        /* Add the prefix "0x" */
        buftop[--i] = 'x';
        buftop[--i] = '0';
    }

    return -i;
}

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
void prints(const char* prefix, const char *str, const char* suffix)
{
    prefix--;
    while (*++prefix != '\0') {
        printchar(*prefix);
    }

    str--;
    while (*++str != '\0') {
        printchar(*str);
    }

    suffix--;
    while (*++suffix != '\0') {
        printchar(*suffix);
    }
}

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
void printu(const char* prefix, uint32_t num, const char* suffix)
{
    char buf[10];   // 10 should be enough for uint32_t (Max 4294967295)
    char *data;
    int data_len;

    data_len = convert_value(num, 10, 0, buf + sizeof(buf));
    data = buf + sizeof(buf) - data_len;

    prefix--;
    while (*++prefix != '\0') {
        printchar(*prefix);
    }

    while (--data_len >= 0) {
        printchar(*data++);
    }

    suffix--;
    while (*++suffix != '\0') {
        printchar(*suffix);
    }
}

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
void printx(const char* prefix, uint32_t num, const char* suffix)
{
    char buf[10];   // 10 should be enough for uint32_t with "0x" prefix (Max 0xFFFFFFFF)
    char *data;
    int data_len;

    data_len = convert_value(num, 16, ALPHA('X'), buf + sizeof(buf));
    data = buf + sizeof(buf) - data_len;

    prefix--;
    while (*++prefix != '\0') {
        printchar(*prefix);
    }

    while (--data_len >= 0) {
        printchar(*data++);
    }

    suffix--;
    while (*++suffix != '\0') {
        printchar(*suffix);
    }
}
