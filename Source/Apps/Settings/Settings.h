#ifndef __SETTINGS_H
#define	__SETTINGS_H

#include "stm32f10x.h"

#define FLASH_SETTINGS_BLOCK 0x0801c000

#define FLASH_SETTINGS_ADDR 0x0801c000

void Settings(void);

void Settings_Init(void);

typedef struct
{
 u8 Language;
 u8 EBD_Model;
 u16 InternalTemperature_Max;
 u16 ExternalTemperature_Max;
 u16 Protection_Resume_Gap;
}Settings_Struct;

extern Settings_Struct* CurrentSettings;

void GetNecessarySettings(void);

#endif /* __SETTINGS_H */
