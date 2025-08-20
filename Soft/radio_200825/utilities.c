/**
 * @file      utilities.h
 *
 * @brief     Helper functions implementation
 *
 * @copyright Revised BSD License, see section \ref LICENSE.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "utilities.h"

/**
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

int32_t rand1(void)
{
    return ((next = next * 1103515245L + 12345L) % RAND_LOCAL_MAX);
}

void srand1(uint32_t seed)
{
    next = seed;
}
// Standard random functions redefinition end

int32_t randr(int32_t min, int32_t max)
{
    return (int32_t)rand1() % (max - min + 1) + min;
}

void memcpy1(uint8_t *dst, const uint8_t *src, uint16_t size)
{
    while (size--)
    {
        *dst++ = *src++;
    }
}

void memcpyr(uint8_t *dst, const uint8_t *src, uint16_t size)
{
    dst = dst + (size - 1);
    while (size--)
    {
        *dst-- = *src++;
    }
}

void memset1(uint8_t *dst, uint8_t value, uint16_t size)
{
    while (size--)
    {
        *dst++ = value;
    }
}

int8_t Nibble2HexChar(uint8_t a)
{
    if (a < 10)
    {
        return '0' + a;
    }
    else if (a < 16)
    {
        return 'A' + (a - 10);
    }
    else
    {
        return '?';
    }
}

void print_hex(uint8_t *buffer, uint16_t size)
{
    // uint16_t i;

    // for (i = 0; i < size; i++)
    // {
    //     printf("%02X ", buffer[i]);
    // }
    // printf("\r\n");
}

void hex_to_string(const uint8_t *hex_data, uint16_t size, char *output)
{
    for (uint16_t i = 0; i < size; i++)
    {
        output[i * 2] = Nibble2HexChar((hex_data[i] >> 4) & 0x0F);
        output[i * 2 + 1] = Nibble2HexChar(hex_data[i] & 0x0F);
    }
    output[size * 2] = '\0'; // Null-terminate the string
}
