#ifndef __SETTINGS_H
#define	__SETTINGS_H

#include "stm32f10x.h"

#define FLASH_SETTINGS_BLOCK 0x0801d800

#define FLASH_SETTINGS_ADDR 0x0801d800

void Settings(void);

void Settings_Init(void);

typedef struct
{
 u8 Language;
}Settings_Struct;

extern Settings_Struct* CurrentSettings;

#endif /* __SETTINGS_H */
