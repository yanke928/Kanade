#ifndef __HEX2BIN_H
#define __HEX2BIN_H

#include <stdbool.h>

#include "stm32f10x.h"
#include "stm32_flash.h"

#include "ff.h"

typedef union
{
char u8[STM32_SECTOR_SIZE];
u16 u16[STM32_SECTOR_SIZE/2];
}Buffer_t;

u8 CheckAHexFile(FIL* firmware);

u8 WriteHexToROM(FIL* firmware);

void ShowFatfsErrorCode(u8 err);

#endif 
