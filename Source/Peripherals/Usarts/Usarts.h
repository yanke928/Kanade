#ifndef __USARTS_H
#define	__USARTS_H

#include "stm32f10x.h"
#include "misc.h"
#include <stdio.h>

void USART1_Init(void);

void USART1_TX_DMA_Init(void);

#endif /* __USARTS_H */
