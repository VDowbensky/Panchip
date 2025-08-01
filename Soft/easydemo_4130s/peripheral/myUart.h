#ifndef __MYUART_H
#define __MYUART_H
#include <stdio.h>	
#include <stdint.h>	
#include <stdbool.h>	
#include <string.h>	

typedef void (*UART_CALLBACK)(uint8_t *buf, uint16_t count);

#define USART_REC_LEN  			255  	//

typedef struct 
{
    bool isValid;
    uint8_t packet[USART_REC_LEN];
    uint16_t len;
}uartPacket_ts;

void myUart1_init(uint32_t baudrate, UART_CALLBACK cb);
void myUart1_sendByte(uint8_t src);
void myUart1_sendArray(uint8_t *src, uint16_t srclen);
#endif


