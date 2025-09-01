/**
 * @file      utilities.h
 *
 * @brief     Helper functions implementation
 *
 * @copyright Revised BSD License, see section \ref LICENSE.
 *
 */
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdint.h>

/**
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS         1
#endif

#ifndef FAIL
#define FAIL            0
#endif

/**
 * @brief Initializes the pseudo random generator initial value
 *
 * @param [IN] seed Pseudo random generator initial value
 */
void srand1(uint32_t seed);

int32_t rand1(void);

/**
 * @brief Computes a random number between min and max
 *
 * @param [IN] min range minimum value
 * @param [IN] max range maximum value
 * @retval random random value in range min..max
 */
int32_t randr(int32_t min, int32_t max);

/**
 * @brief Converts a nibble to an hexadecimal character
 *
 * @param [IN] a Nibble to be converted
 * @retval hexchar Converted hexadecimal character
 */
int8_t print_to_hexchar(uint8_t a);

/**
 * @brief Converts a byte array to a hexadecimal string
 * @param [IN] hex_data Pointer to the byte array
 * @param [IN] Size Size of the byte array
 * @param [OUT] output Pointer to the output string buffer
 */
void print_to_hex(const uint8_t *hex_data, uint16_t Size, char *output);

/**
 * @brief prints a byte array in hexadecimal format
 * @param [IN] Buffer  Byte array to be printed
 * @param [IN] Size    Size of the byte array
 */
void print_hex(uint8_t *Buffer, uint16_t Size);

/**
 * @brief Computes the CRC-16-CCITT checksum for a given data buffer
 * @param [IN] data Pointer to the data buffer
 * @param [IN] length Length of the data buffer
 * @retval crc16_ccitt Computed CRC-16-CCITT checksum
 */
uint16_t crc16_ccitt(const uint8_t *data, uint8_t length);

#endif // __UTILITIES_H__
