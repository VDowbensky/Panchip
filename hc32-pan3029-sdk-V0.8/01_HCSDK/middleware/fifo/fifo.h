
#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdbool.h>
#include <stdint.h>

#define RX_FIFO_SIZE    256
#define TX_FIFO_SIZE    256

/*!
 * FIFO structure
 */
typedef struct Fifo_s
{
    int32_t begin;    //start index of fifo
    int32_t end;      //end index of fifo
    int32_t size;     //max size of fifo
    int32_t num;      //numer of datas in fifo
    uint8_t *buf;     //data buffer of fifo
}Fifo_t;

/*!
 * Initializes the FIFO structure
 *
 * \param [IN] fifo   Pointer to the FIFO object
 * \param [IN] buffer Buffer to be used as FIFO
 * \param [IN] size   Size of the buffer
 */
void fifo_init( Fifo_t *fifo, uint8_t *buffer, uint16_t size );

/*!
 * Pushes data to the FIFO
 *
 * \param [IN] fifo Pointer to the FIFO object
 * \param [IN] data Data to be pushed into the FIFO
 */
void fifo_push( Fifo_t *fifo, uint8_t data );

/*!
 * Pops data from the FIFO
 *
 * \param [IN] fifo Pointer to the FIFO object
 * \retval data     Data popped from the FIFO
 */
uint8_t fifo_pop( Fifo_t *fifo );

/*!
 * Flushes the FIFO
 *
 * \param [IN] fifo   Pointer to the FIFO object
 */
void fifo_flush( Fifo_t *fifo );

/*!
 * Checks if the FIFO is empty
 *
 * \param [IN] fifo   Pointer to the FIFO object
 * \retval isEmpty    true: FIFO is empty, false FIFO is not empty
 */
bool fifo_is_empty( Fifo_t *fifo );

/*!
 * Checks if the FIFO is full
 *
 * \param [IN] fifo   Pointer to the FIFO object
 * \retval isFull     true: FIFO is full, false FIFO is not full
 */
bool fifo_is_full( Fifo_t *fifo );

/*!
 * Get data num in the fifo
 *
 * \param [IN] fifo   Pointer to the FIFO object
 * \retval num        data num in the fifo
 */
int32_t fifo_data_num_get(Fifo_t *fifo);

#endif // __FIFO_H__

