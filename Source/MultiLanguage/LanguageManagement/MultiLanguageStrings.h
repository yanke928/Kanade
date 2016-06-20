#ifndef __MULTILANGUAGESTRINGS_H
#define	__MULTILANGUAGESTRINGS_H

#include "stm32f10x.h"
#include "SSD1306.h"

#define LanguageNum 4

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
extern const char * SetYear_Str[LanguageNum];
extern const char * SetYearUnit_Str[LanguageNum];
extern const char * SetMonth_Str[LanguageNum];
extern const char * SetMonthUnit_Str[LanguageNum];
extern const char * SetDay_Str[LanguageNum];
extern const char * SetDayUnit_Str[LanguageNum];
extern const char * SetHour_Str[LanguageNum];
extern const char * SetHourUnit_Str[LanguageNum];
extern const char * SetMin_Str[LanguageNum];
extern const char * SetMinUnit_Str[LanguageNum];
extern const char * SetSec_Str[LanguageNum];
extern const char * SetSecUnit_Str[LanguageNum];
extern const char * TimeSetting_Str[LanguageNum];
extern const char * StepUpConfirm_Str[LanguageNum];
extern const char * StartCurrentGet_Str[LanguageNum];
extern const char * EndCurrentGet_Str[LanguageNum];
extern const char * StepCurrentGet_Str[LanguageNum];
extern const char * TimeIntervalGet_Str[LanguageNum];
extern const char * ProtVoltageGet_Str[LanguageNum];
extern const char * RunTime_Str[LanguageNum];
extern const char * CurrentVoltage_Str[LanguageNum];
extern const char * CurrentCurrent_Str[LanguageNum];
extern const char * StepUpTestRunning_Str[LanguageNum];
extern const char * StepUpTestProtected_Str[LanguageNum];
extern const char * StepUpTestDone_Str[LanguageNum];
extern const char * StepUpTestDialgram_Str[LanguageNum];
extern const char * StepUpTestList_Str[LanguageNum];
extern const char * StepUpTestDialgramTime_Str[LanguageNum];
extern const char * StepUpTestDialgramCurrent_Str[LanguageNum];
extern const char * StepUpTestDialgramVoltage_Str[LanguageNum];
extern const char * StepUpTestExitBroswer_Str[LanguageNum];
extern const char * LegacyTestConfirm_Str[LanguageNum];

extern const OLED_PositionStruct ComfirmationPositions[LanguageNum][2];

#endif /* __MULTILANGUAGESTRINGS_H */













