/**
 *******************************************************************************
 * @file     bytes.c
 * @create   2024-11-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */

#include "bytes.h"

uint16_t BytesToUint16(uint8_t *p)
{
    return (p[1]<<8) | p[0];
}

uint32_t BytesToUint24(uint8_t *p)
{
    return (p[2]<<16) | (p[1]<<8) | p[0];
}

uint32_t BytesToUint32(uint8_t *p)
{
    return (p[3]<<24) | (p[2]<<16)  | (p[1]<<8) | p[0];
}

uint16_t BytesToUint16Be(uint8_t *p)
{
    return (p[0]<<8) | p[1];
}

uint32_t BytesToUint24Be(uint8_t *p)
{
    return (p[0]<<16) | (p[1]<<8) | p[2];
}

uint32_t BytesToUint32Be(uint8_t *p)
{
    return (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
}

int Uint16ToBytes(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v>>8);
    
    return sizeof(uint16_t);
}

int Uint24ToBytes(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v>>8);
    p[2] = (uint8_t)(v>>16);
    
    return 3;
}

int Uint32ToBytes(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v>>8);
    p[2] = (uint8_t)(v>>16);
    p[3] = (uint8_t)(v>>24);
    
    return sizeof(uint32_t);
}
