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

/**
 * @brief Converts a nibble to an hexadecimal character
 *
 * @param [IN] a Nibble to be converted
 * @retval hexChar Converted hexadecimal character
 */
int8_t print_to_hexchar(uint8_t a)
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

/**
 * @brief Converts a byte array to a hexadecimal string
 * @param [IN] hex_data Pointer to the byte array
 * @param [IN] Size Size of the byte array
 * @param [OUT] output Pointer to the output string buffer
 */
void print_to_hex(const uint8_t *hex_data, uint16_t Size, char *output)
{
    int i;
    for (i = 0; i < Size; i++)
    {
        output[i * 2] = print_to_hexchar((hex_data[i] >> 4) & 0x0F);
        output[i * 2 + 1] = print_to_hexchar(hex_data[i] & 0x0F);
    }
    output[Size * 2] = '\0'; /* Null-terminate the string */
}

/**
 * @brief prints a byte array in hexadecimal format
 * @param [IN] Buffer  Byte array to be printed
 * @param [IN] Size    Size of the byte array
 */
void print_hex(uint8_t *Buffer, uint16_t Size)
{
    int i;
    for (i = 0; i < Size; i++)
    {
        printf("%02X ", Buffer[i]);
    }
    printf("\r\n");
}

/**
 * @brief Computes the CRC-16-CCITT checksum for a given data buffer
 * @param [IN] data Pointer to the data buffer
 * @param [IN] length Length of the data buffer
 * @retval crc16_ccitt Computed CRC-16-CCITT checksum
 */
uint16_t crc16_ccitt(const uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    const uint16_t polynomial = 0x1021; // x^16 + x^12 + x^5 + 1

    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8; // 将字节移到高位

        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ polynomial;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}
