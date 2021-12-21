#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "main.h"
#include "fifo.h"

#define U1_BUF_LENGHT 256
#define U1_FIFO_BUF_LENGTH 512



void usart1_init(void);
void usart1_SendData(uint8_t *data, uint16_t len);
void User_printf(const char *format,...);

extern fifo_s_t u1_Fifo;

#endif
