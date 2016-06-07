//File Name   £ºUI_Menu.c
//Description : UI Menu

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "Keys.h" 

#include "UI_Utilities.h" 

#include "UI_Menu.h" 

#define UI_MENU_HANDLER_PRIORITY tskIDLE_PRIORITY+2

xQueueHandle UI_MenuMsg; 

void UI_Menu_Handler(void *pvParameters)
{
	int currentPos, i, p, q, currentRelativePos, lastRelativePos;
	UI_Menu_Param_Struct * menuParams=pvParameters;
	bool allContentChanged;
	bool relativePosChanged;
  Key_Message_Struct keyMessage;
	char itemStrings[10][20];
	u8 stringsAddr[10];
	u8 selection;
	u8 displayedItemNum=(menuParams->ItemNum>=5?5:menuParams->ItemNum);
	currentPos = menuParams->DefaultPos;
	currentRelativePos = 0;
	allContentChanged = true;
	relativePosChanged = false;
	lastRelativePos = 0;
	stringsAddr[0] = 0;
	p = 1;
	/*Find the addrs for every string in string*/
	for (i = 0; menuParams->ItemString[i] != 0; i++)
	{
		if (menuParams->ItemString[i] == '%')
		{
			stringsAddr[p] = i + 1;
			p++;
		}
	}
	for (i = 0; i < menuParams->ItemNum; i++)
	{
		for (p = 0; menuParams->ItemString[stringsAddr[i] + p] !=
			'%'&&menuParams->ItemString[stringsAddr[i] + p] != 0; p++)
		{
			itemStrings[i][p] = menuParams->ItemString[stringsAddr[i] + p];
		}
		itemStrings[i][p] = 0;
	}
	SetKeyBeatRate(menuParams->FastSpeed);
  for(;;)
	{
		ReDraw:
		xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
		if (allContentChanged == true)
		{
			for (i = 0; i < displayedItemNum; i++)
			{
				p = GetStringLength(itemStrings[currentPos + i]);
				q = GetCentralPosition(0, 127, p);
				OLED_FillRect(1, i * 12 + 1, 126, i * 12 + 12, 0);
				OLED_ShowAnyString(q, i * 12 + 1, (char *)itemStrings[currentPos + i], NotOnSelect, 12);
			}
			OLED_DrawRect(0, 0, 127, displayedItemNum*12, DRAW);
			for(i = 1; i < displayedItemNum; i++)
			{
			 OLED_DrawHorizonalLine(12*i, 0, 127, DRAW);
			}
			OLED_InvertRect(1, currentRelativePos * 12 + 1, 126, currentRelativePos * 12 + 12);
			allContentChanged = false;
		}
		if (relativePosChanged == true)
		{
			OLED_InvertRect(1, lastRelativePos * 12 + 1, 126, lastRelativePos * 12 + 12);
			OLED_InvertRect(1, currentRelativePos * 12 + 1, 126, currentRelativePos * 12 + 12);
			lastRelativePos = currentRelativePos;
			relativePosChanged = false;
		}	 
	  xSemaphoreGive(OLEDRelatedMutex);
		xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY );	
		if(keyMessage.KeyEvent==LeftClick||keyMessage.AdvancedKeyEvent==LeftContinous)
		{
			if (currentRelativePos == 0)
			{
				currentPos--;
				if (currentPos == -1)
				{
					currentPos = 0;
					goto ReDraw;
				}
				allContentChanged = true;
				goto ReDraw;
			}
			else
			{
				currentRelativePos--;
				relativePosChanged = true;
				goto ReDraw;
			}
		}
		else if(keyMessage.KeyEvent==RightClick||keyMessage.AdvancedKeyEvent==RightContinous)
		{
			if (currentRelativePos == displayedItemNum-1)
			{
				currentPos++;
				if (currentPos == menuParams->ItemNum - (displayedItemNum-1))
				{
					currentPos = menuParams->ItemNum - displayedItemNum;
					goto ReDraw;
				}
				allContentChanged = true;
				goto ReDraw;
			}
			else
			{
				currentRelativePos++;
				relativePosChanged = true;
				goto ReDraw;
			}		 
		}
		else if(keyMessage.KeyEvent == MidDouble)
		{
		 selection=255;
		 xQueueSend(UI_MenuMsg,&selection, 100/portTICK_RATE_MS);
		 vTaskDelete(NULL);
		}
		else if(keyMessage.KeyEvent == MidClick)
		{
		 selection=currentPos + currentRelativePos;
		 xQueueSend(UI_MenuMsg,&selection, 100/portTICK_RATE_MS);
		 vTaskDelete(NULL);		 
		}
	}
}

void UI_Menu_Init(UI_Menu_Param_Struct * menuParams)
{
	UI_MenuMsg=xQueueCreate(1, sizeof(u8));
	xTaskCreate(UI_Menu_Handler, "UI_Menu Handler",
	256, menuParams, UI_MENU_HANDLER_PRIORITY, NULL);
}		

