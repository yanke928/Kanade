//File Name   LegacyTest.c
//Description LegacyTest

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "EBProtocol.h"
#include "sdcard.h"
#include "sdcardff.h"

#include "UI_Adjust.h"

#include "MultiLanguageStrings.h"

#include "Settings.h"

#include "StepUpTest.h"

#include "LegacyTest.h"

#define LEGACY_TEST_CURRENT_MAX 5000

void LegacyTest_Handler(void *pvParameters)
{
 
}

void RunLegacyTest(u8* status,Legacy_Test_Param_Struct* test_Params)
{
 *status=LEGACY_TEST;
 test_Params->Current=
 GetTestParam(LegacyTestSetCurrent_Str[CurrentSettings->Language], 100, LEGACY_TEST_CURRENT_MAX,
 1000, 100, "mA", 20);
 test_Params->ProtectVolt=
 GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language],0,
 (int)(1000 * CurrentMeterData.Voltage)/10*10>0?(1000 * CurrentMeterData.Voltage)/10*10:100,
 (int)(900 * CurrentMeterData.Voltage)/10*10>0?(900 * CurrentMeterData.Voltage) / 10 * 10:100, 10, "mV", 25);
 if(!SDCardMountStatus)
 {
  
 }
}
