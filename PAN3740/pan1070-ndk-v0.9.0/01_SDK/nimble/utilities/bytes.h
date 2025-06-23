/**
 *******************************************************************************
 * @file     bytes.h
 * @create   2024-11-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#ifndef BYTES_H
#define BYTES_H

#include <stdint.h>

uint16_t BytesToUint16(uint8_t *p);
uint32_t BytesToUint24(uint8_t *p);
uint32_t BytesToUint32(uint8_t *p);

uint16_t BytesToUint16Be(uint8_t *p);
uint32_t BytesToUint24Be(uint8_t *p);
uint32_t BytesToUint32Be(uint8_t *p);

int Uint16ToBytes(uint8_t *p, uint16_t v);
int Uint24ToBytes(uint8_t *p, uint32_t v);
int Uint32ToBytes(uint8_t *p, uint32_t v);

#endif
