#ifndef __UI_CLOCK_H
#define	__UI_CLOCK_H

#include "stm32f10x.h"

#include <stdbool.h>

//see .c for details
void OLED_Show1624String(u8 x,u8 y,char* string);

//see .c for details
void OLED_Show1216String(u8 x,u8 y,char* string);

#endif /* __UI_CLOCK_H */
