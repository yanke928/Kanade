//File Name    ListView.c
//Description : View data in list

#include <string.h>
#include <stdio.h>

#include "UI_Button.h" 
#include "UI_Dialogue.h" 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "Keys.h"

#include "Settings.h"

#include "UI_ListView.h"
#include "UI_Utilities.h"

#include "ExceptionHandle.h"

#define UI_LISTVIEW_HANDLER_PRIORITY tskIDLE_PRIORITY+2

xQueueHandle UI_ListViewMsg;

xTaskHandle UI_ListView_Handle=NULL;

void UI_ListView_Handler(void *pvParameters)
{
	ListView_Param_Struct* listView_Params = pvParameters;
	Key_Message_Struct key_Message;
	u16 pos[5];
	char tempString[29];
	int currentPos;
	int currentRelativePos;
	u16 itemStringLengths[4];
	u16 lastSelected = listView_Params->DefaultPos;
	u16 lengthTemp;
	u16 posTemp;
	u16 i, p, m;
	u8 displayLineNumInSinglePage = listView_Params->ListLength > 5 ? 5 : listView_Params->ListLength;
	//	u16* dataNumPointer;
	float * dataPointers[4];
	bool updateNumTab = true;
	bool firstDraw = true;
	bool currentPosChanged = false;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	OLED_DrawRect(0, 0, 127, 23 + 10 * (displayLineNumInSinglePage - 1), DRAW);
	OLED_DrawHorizonalLine(13, 0, 127, 1);
	xSemaphoreGive(OLEDRelatedMutex);
	SetKeyBeatRate(listView_Params->FastSpeed);
	/*Get the lengths of the itemNames*/
	for (i = 0; i < listView_Params->ItemNum; i++)
	{
		itemStringLengths[i] = GetStringGraphicalLength(listView_Params->ItemNames[i]);
	}
	pos[0] = listView_Params->ItemPositions[0];
	/*Make the positions of the itemStrings centural between two grids*/
	for (i = 0; i < listView_Params->ItemNum; i++)
	{
		pos[i] = GetCentralPosition(listView_Params->ItemPositions[i],
			listView_Params->ItemPositions[i + 1], itemStringLengths[i]) - 1;
	}
	for (i = 0; i < listView_Params->ItemNum; i++)
	{
		OLED_ShowAnyString(pos[i], 1, listView_Params->ItemNames[i], NotOnSelect, 12);
	}
	/*Draw vertical grids*/
	for (i = 0; i < listView_Params->ItemNum; i++)
	{
		OLED_DrawVerticalLine(listView_Params->ItemPositions[i] - 2, 0, 23 + 10 * (displayLineNumInSinglePage - 1), 1);
	}
	/*Draw horizonal grids*/
	for (i = 0; i < displayLineNumInSinglePage; i++)
	{
		OLED_DrawHorizonalLine(23 + 10 * i, 0, 127, 1);
	}
	if (listView_Params->Item1AutoNum == true)
	{
		for (i = 0; i < listView_Params->ItemNum - 1; i++)
		{
			dataPointers[i + 1] = listView_Params->DataPointers[i];
		}
		//		dataNumPointer = listView_Params->DataAutoNumPointer;
		//		for (i = 0; i < listView_Params->ListLength; i++)
		//		{
		//			dataNumPointer[i] = listView_Params->Item1AutoNumStart + i*listView_Params->Item1AutoNumStep;
		//		}
	}
	else
	{
		for (i = 0; i < listView_Params->ItemNum; i++)
		{
			dataPointers[i] = listView_Params->DataPointers[i];
		}
	}
	currentPos = listView_Params->DefaultPos;
	currentRelativePos = 0;
	ClearKeyEvent(key_Message);
	for (;;)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		if (listView_Params->Item1AutoNum == true) p = 1;
		else p = 0;
		for (i = 0; i < listView_Params->ItemNum - p; i++)
		{
			for (m = 0; m < displayLineNumInSinglePage; m++)
			{
				sprintf(tempString, listView_Params->sprintfCommandStrings[i], dataPointers[i + p][currentPos + m]);
				lengthTemp = GetStringGraphicalLength(tempString);
				posTemp = GetCentralPosition(listView_Params->ItemPositions[i + p],
					listView_Params->ItemPositions[i + p + 1], lengthTemp);
				OLED_ShowAnyString(posTemp - 1, 15 + 10 * m, tempString, NotOnSelect, 8);
			}
		}

		if (listView_Params->Item1AutoNum == true)
		{
			if (currentPosChanged == true)
			{
				currentPosChanged = false;
				for (i = 0; i < displayLineNumInSinglePage; i++)
					OLED_FillRect(listView_Params->ItemPositions[0], 15 + 10 * i,
						listView_Params->ItemPositions[1] - 4, 22 + 10 * i, 0);
				firstDraw = true;
				updateNumTab = true;
			}
			for (i = 0; i < displayLineNumInSinglePage; i++)
			{
				sprintf(tempString, "%d", listView_Params->Item1AutoNumStart + (currentPos + i)*listView_Params->Item1AutoNumStep);
				lengthTemp = GetStringGraphicalLength(tempString);
				posTemp = GetCentralPosition(listView_Params->ItemPositions[0],
					listView_Params->ItemPositions[1], lengthTemp);
				if (currentRelativePos != i&&lastSelected != i)
					OLED_ShowAnyString(posTemp - 1, 15 + 10 * i, tempString, NotOnSelect, 8);
				if (currentRelativePos == i&&updateNumTab == true)
				{
					if (firstDraw == false)
						OLED_InvertRect(listView_Params->ItemPositions[0], 15 + 10 * lastSelected,
							listView_Params->ItemPositions[1] - 4, 22 + 10 * lastSelected);
					else firstDraw = false;
					OLED_ShowAnyString(posTemp - 1, 15 + 10 * i, tempString, NotOnSelect, 8);
					OLED_InvertRect(listView_Params->ItemPositions[0], 15 + 10 * i,
						listView_Params->ItemPositions[1] - 4, 22 + 10 * i);
					lastSelected = currentRelativePos;
					updateNumTab = false;
				}
			}
		}
		OLED_Refresh_Gram();
		xSemaphoreGive(OLEDRelatedMutex);
		xQueueReceive(Key_Message, &key_Message, portMAX_DELAY);
		if (key_Message.KeyEvent == RightClick || key_Message.AdvancedKeyEvent == RightContinous)
		{
			currentRelativePos++;
			updateNumTab = true;
			if (currentRelativePos >= displayLineNumInSinglePage)
			{
				currentPosChanged = true;
				currentPos++;
				if (currentPos + displayLineNumInSinglePage > listView_Params->ListLength)
				{
					currentPos = listView_Params->ListLength - displayLineNumInSinglePage;
					currentPosChanged = false;
					updateNumTab = false;
				}
				currentRelativePos = displayLineNumInSinglePage - 1;
			}
		}
		else if (key_Message.KeyEvent == LeftClick || key_Message.AdvancedKeyEvent == LeftContinous)
		{
			currentRelativePos--;
			updateNumTab = true;
			if (currentRelativePos < 0)
			{
				currentPosChanged = true;
				currentPos--;
				if (currentPos < 0)
				{
					currentPos = 0;
					currentPosChanged = false;
					updateNumTab = false;
				}
				currentRelativePos = 0;
			}
		}
		else if (key_Message.KeyEvent == MidDouble)
		{
			currentPos = 32767;
			xQueueSend(UI_ListViewMsg, &currentPos, portMAX_DELAY);
			for(;;) vTaskDelay(portMAX_DELAY);
		}
		else if (key_Message.KeyEvent == MidClick)
		{
			xQueueSend(UI_ListViewMsg, &currentPos, portMAX_DELAY);
			for(;;) vTaskDelay(portMAX_DELAY);
		}
	}
}

/**
  * @brief  See UI_ListView_Handler for details

  * @param  ListView param struct pointer

  */
void UI_ListView_Init(ListView_Param_Struct * listViewParams)
{
	UI_ListViewMsg = xQueueCreate(1, sizeof(int));
	CreateTaskWithExceptionControl(UI_ListView_Handler, "UI_ListView Handler",
		300, listViewParams, UI_LISTVIEW_HANDLER_PRIORITY, &UI_ListView_Handle);
}

/**
  * @brief  DeInit ListView
  */
void UI_ListView_DeInit()
{
 if(UI_ListView_Handle!=NULL)
	 vTaskDelete(UI_ListView_Handle);
 vQueueDelete(UI_ListViewMsg);
 UI_ListView_Handle=NULL;
}

	
	

