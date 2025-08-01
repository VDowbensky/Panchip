#ifndef __myUart3_H
#define __myUart3_H

#include "myUart.h"
#include <stdint.h>

void myUart3_init(uint32_t baudrate, UART_CALLBACK cb);
void myUart3_printf(uint16_t sendLen, char *fmt, ...);

#endif


