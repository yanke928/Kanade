//File Name   MassStorage.c
//Description MassStorage App

#include "stm32f10x.h"
#include <stdio.h>

#include "stm32f10x_rtc.h"

#include "LED.h"

#include "FreeRTOS_Standard_Include.h"

#include "mass_mal.h"
#include "Keys.h"
#include "usb_pwr.h"
#include "usb_lib.h"
#include "LED_Animate.h"
#include "SSD1306.h"

#include "MultiLanguageStrings.h"

#include "Settings.h"

#include "UI_Dialogue.h"
#include "UI_Button.h"

#include "MassStorage.h"

const LEDAnimateSliceStruct MAL_Read_Animate[] =
{
 {Cyan,50,6,false},//Cyan with brightness of 6
 {Cyan,50,0,false},//Black
 {0,0,0,END}//End of the animation
};

const LEDAnimateSliceStruct MAL_Write_Animate[] =
{
 {Red,50,6,false},//Red with brightness of 2
 {Red,50,0,false},//Black
 {0,0,0,END}//End of the animation
};

void MassStorage_App()
{
	bool success;
	bool exitFlag = false;
	u8 i;
	u8 status;
	UI_Button_Param_Struct button_params;
	OLED_PositionStruct position[4] = { {68,44} };
	const char * unMountString[1];
	
	unMountString[0]=UnMountUSBMassStorage_Str[CurrentSettings->Language];
	
	success = MAL_Mount();
	if (!success)
	{
		ShowSmallDialogue(SettingsMountFailed_Str[CurrentSettings->Language], 1000, true);
		return;
	}
	ShowDialogue(USBMassStorage_Str[CurrentSettings->Language],
		USBMassStorageStatus_Str[CurrentSettings->Language], "",false,false);
	
	/*Set the positions for the voltage options*/
	button_params.ButtonStrings = unMountString;
	button_params.ButtonNum = 1;
	button_params.DefaultValue = 0;
	button_params.Positions = position;
	UI_Button_Init(&button_params);
	for (;;)
	{
		if (exitFlag && (!Usb_Status_Reg)) break;
		if (xQueueReceive(UI_ButtonMsg, &i, 0) == pdPASS) exitFlag = true;
		if (status != Usb_Status_Reg)
		{
			if (Usb_Status_Reg & 0x01)
			{
				LED_Animate_Init(MAL_Write_Animate);
				OLED_ShowAnyString(60, 16,
					USBMassStorageStatusWriting_Str[CurrentSettings->Language], NotOnSelect, 16);
			}
			else if (Usb_Status_Reg & 0x02)
			{
				LED_Animate_Init(MAL_Read_Animate);
				OLED_ShowAnyString(60, 16,
					USBMassStorageStatusReading_Str[CurrentSettings->Language], NotOnSelect, 16);
			}
			else
			{
				LED_Animate_DeInit();
				OLED_ShowAnyString(60, 16,
					USBMassStorageStatusIdling_Str[CurrentSettings->Language], NotOnSelect, 16);
				if (exitFlag) break;
			}
			status = Usb_Status_Reg;
		}
		Usb_Status_Reg = 0;
		vTaskDelay(100 / portTICK_RATE_MS);
	}
  vTaskDelay(50 / portTICK_RATE_MS);
  ClearKeyEvent();
	OLED_Clear_With_Mutex_TakeGive();
	LED_Animate_DeInit();
	PowerOff();
	UI_Button_DeInit();
}

