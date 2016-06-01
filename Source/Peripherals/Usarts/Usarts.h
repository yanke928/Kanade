#ifndef __USARTS_H
#define	__USARTS_H

#include "stm32f10x.h"
#include "misc.h"
#include <stdio.h>

void UART1SendByte(unsigned char SendData);

void USART1_Init(void);

void USART1_TX_DMA_Init(void);

void USART1SendString(char *s);

#endif /* __USARTS_H */
