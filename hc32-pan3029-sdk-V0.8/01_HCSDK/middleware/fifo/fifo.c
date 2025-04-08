/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file fifo.c
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#include <stdlib.h>
#include "fifo.h"

static uint16_t fifo_next_index( Fifo_t *fifo, uint16_t index )
{
    return ((index + 1)%fifo->size);
}

void fifo_init( Fifo_t *fifo, uint8_t *buffer, uint16_t size )
{
    __disable_irq();
    fifo->begin = 0;
    fifo->end = 0;
    fifo->buf = buffer;
    fifo->size = size;
    __enable_irq();
}

void fifo_push( Fifo_t *fifo, uint8_t data )
{
    __disable_irq();
    fifo->end = fifo_next_index( fifo, fifo->end );
    fifo->buf[fifo->end] = data;
    __enable_irq();
}

uint8_t fifo_pop( Fifo_t *fifo )
{
    __disable_irq();
    int index = fifo_next_index( fifo, fifo->begin );
    uint8_t data = fifo->buf[index];
    fifo->begin = index;
    __enable_irq();
    
    return data;
}

void fifo_flush( Fifo_t *fifo )
{
    __disable_irq();
    fifo->begin = 0;
    fifo->end = 0;
    __enable_irq();
}

bool fifo_is_empty( Fifo_t *fifo )
{
    __disable_irq();
    bool cond = (fifo->begin == fifo->end);
    __enable_irq();
    return cond;
}

bool fifo_is_full( Fifo_t *fifo )
{
    __disable_irq();
    bool cond = ( fifo_next_index( fifo, fifo->end ) == fifo->begin );
    __enable_irq();

    return cond;
}

int32_t fifo_data_num_get(Fifo_t *fifo)
{
    __disable_irq();
    int num;
    if ( fifo->begin == fifo->end )
    {
        num = 0;
    }
    else
    {
        /* empty bytes of buffer between begin and end*/
        num = abs(fifo->begin - fifo->end)-1;

        /* data num in the fifo */
        num = fifo->size - num;
    }
    __enable_irq();

    return num;
}

