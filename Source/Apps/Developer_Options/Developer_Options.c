//File Name   Developer_Options.c
//Description Developer Options

#include <stdio.h>  
#include <string.h>  

#include "stm32f10x.h"

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"
#include "Digital_Load.h"

#include "UI_Menu.h"
#include "UI_Dialogue.h"

#include "Settings.h"

#include "MultiLanguageStrings.h"

#include "Developer_Options.h"

void Set_Digital_Load_Params(void);

/**
  * @brief  Developer options

  * @retval None
  */
void Developer_Options()
{
	UI_Menu_Param_Struct menuParams;
	const char* stringTab[1];
	u8 selection;

	stringTab[0] = "Load Mode";

	menuParams.ItemStrings = stringTab;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = 1;
	menuParams.FastSpeed = 10;
	OLED_Clear_With_Mutex_TakeGive();

	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	OLED_Clear_With_Mutex_TakeGive();

	switch (selection)
	{
	case 0: Set_Digital_Load_Params(); break;
	}
}

/**
  * @brief  Set difital load params

  * @retval None
  */
void Set_Digital_Load_Params()
{
	UI_Menu_Param_Struct menuParams;
	u8 selection;
	u8 i;
	const char* stringTab[DIGITAL_LOAD_PARAMS_NUM];

	/*Set stringTab to descriptors of digital_load_params*/
	for (i = 0; i < DIGITAL_LOAD_PARAMS_NUM; i++)
	{
		stringTab[i] = Digital_Load_Params[i]->Descriptor;
	}

	menuParams.ItemStrings = stringTab;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = DIGITAL_LOAD_PARAMS_NUM;
	menuParams.FastSpeed = 10;

	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	/*Out of range,return*/
	if (selection >= DIGITAL_LOAD_PARAMS_NUM) return;
	else
	{
		/*Backup settings*/
		memcpy(&SettingsBkp, CurrentSettings, sizeof(Settings_Struct));
		/*Set params*/
		SettingsBkp.Digital_Load_Params_Mode = selection;
		/*Save settings*/
		SaveSettings();
		ShowSmallDialogue((char *)Saved_Str[CurrentSettings->Language], 1000, true);
	}
}
