#ifndef __MOHA2BIN_H
#define __MOHA2BIN_H

#include <stdbool.h>

#include "stm32f10x.h"
#include "stm32_flash.h"

#include "ff.h"

u8 CheckAMohaFile(FIL* firmware);

u8 WriteFirmwareToROM(FIL* firmware);

#endif 
