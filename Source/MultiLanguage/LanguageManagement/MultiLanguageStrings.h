#ifndef __MULTILANGUAGESTRINGS_H
#define	__MULTILANGUAGESTRINGS_H

#include "stm32f10x.h"
#include "SSD1306.h"

#define LanguageNum 4

#define SETTINGS_START_ADDR 0x0801d800

extern const char * SystemInit_Str[LanguageNum];
extern const char * Capacity_Str[LanguageNum];
extern const char * NoSD_Str[LanguageNum];
extern const char * WaitingForEBD_Str[LanguageNum];
extern const char * EBDConnected_Str[LanguageNum];
extern const char * Confirmation_Str[LanguageNum];
extern const char * ConfirmCancel_Str[LanguageNum];
extern const char * RecordConfirm_Str[LanguageNum];
extern const char * QCMTKConfirm_Str[LanguageNum] ;
extern const char * Saved_Str[LanguageNum] ;
extern const char * Settings_Str[LanguageNum];

extern const OLED_PositionStruct ComfirmationPositions[LanguageNum][2];

#endif /* __MULTILANGUAGESTRINGS_H */













