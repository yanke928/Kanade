//File Name   £ºUI_Button.c
//Description : UI button

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "FreeRTOS_CLI.h"

#include "SSD1306.h"
#include "UI_Button.h" 
#include "Keys.h" 

#define UI_BUTTON_HANDLER_PRIORITY tskIDLE_PRIORITY+2

xQueueHandle UI_ButtonMsg; 

/**
  * @brief  Show buttons on screen

  * @param  ButtonString:A string which contains the strings
	  of the buttons,use'\n'to seperate two strings
		e.g."string1\nstring2\nstring3"

						ButtonNum:The quantity of the buttons that displayed

						DefaultValue:The default selection of the buttons

						Positions:A struct array contains the positions
	  of the buttons

	@retval The handler will send the result to UI_ButtonMsg when confirmation got
  */
void UI_Button_Handler(void *pvParameters)
{
 Key_Message_Struct keyMessage;
 UI_Button_Param_Struct* buttonParams=(UI_Button_Param_Struct*)pvParameters;
 int selection = buttonParams->DefaultValue;
 u8 i;
 u8 p = 1;
 u8 stringsAddr[4];//Relative individual string addr in buttonString
 unsigned char lengths[4];//Lengths of every seperate string
 char buttons[4][12];//Cache contains the button strings
 stringsAddr[0] = 0;//Set addr[0] to the first byte of buttonString
	
	/*Find the addrs for every string in buttonString*/
	for (i = 0; buttonParams->ButtonString[i] != 0; i++)
	{
		if (buttonParams->ButtonString[i] == '%')
		{
			stringsAddr[p] = i + 1;
			p++;
		}
	}
	
	/*Make the sepearated strings into cache,get the lengths and
	place the end for sepearated strings*/
	for (i = 0; i < buttonParams->ButtonNum; i++)
	{
		for (p = 0; buttonParams->ButtonString[stringsAddr[i] + p] != '%'&&
		    buttonParams->ButtonString[stringsAddr[i] + p] != 0; p++)
		{
			buttons[i][p] = buttonParams->ButtonString[stringsAddr[i] + p];
		}
		lengths[i] = p;
		buttons[i][p] = 0;
	}
	
	/*Print the strings with respective positions to screen*/
	xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
	SetUpdateOLEDJustNow();
	for (i = 0; i < buttonParams->ButtonNum; i++)
	{
		OLED_ShowAnyString(buttonParams->Positions[i].x, buttonParams->Positions[i].y, 
		buttons[i], NotOnSelect, 12);
	}
  ClearKeyEvent(keyMessage);
	ResetUpdateOLEDJustNow();
	xSemaphoreGive(OLEDRelatedMutex);
	for(;;)
	{
		xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
		SetUpdateOLEDJustNow();
		/*Print the selection box to strings*/
		for (i = 0; i <buttonParams->ButtonNum; i++)
		{
			//Draw the box if button[i] is selected,undraw if not 
			if (selection == i)
				OLED_DrawRect(buttonParams->Positions[i].x - 2, buttonParams->Positions[i].y - 2, 
			  buttonParams->Positions[i].x + lengths[i] * 6 + 1, buttonParams->Positions[i].y + 13, DRAW);
			else
				OLED_DrawRect(buttonParams->Positions[i].x - 2, buttonParams->Positions[i].y - 2,
			buttonParams->Positions[i].x + lengths[i] * 6 + 1, buttonParams->Positions[i].y + 13, UNDRAW);
		}
		ResetUpdateOLEDJustNow();
		xSemaphoreGive(OLEDRelatedMutex);
		for(;;)
		{
		 if (MIDDLE_KEY == KEY_ON)
		 {
			 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
		   SetUpdateOLEDJustNow();
		   OLED_ShowAnyString(buttonParams->Positions[selection].x, buttonParams->Positions[selection].y, 
			 buttons[selection], OnSelect, 12);
			 while (MIDDLE_KEY == KEY_ON);
			 {
			  vTaskDelay(10/portTICK_RATE_MS);
			 }
			 OLED_ShowAnyString(buttonParams->Positions[selection].x, buttonParams->Positions[selection].y, 
			 buttons[selection], NotOnSelect, 12);
			 vTaskDelay(20/portTICK_RATE_MS);
			 ResetUpdateOLEDJustNow();
		   xSemaphoreGive(OLEDRelatedMutex);
			 xQueueSend(UI_ButtonMsg,&selection, 100/portTICK_RATE_MS);
			 vTaskDelete(NULL);
		 }
		 else
		 {
		  if(xQueueReceive(Key_Message, & keyMessage, 0 )==pdPASS)
			{
			 switch(keyMessage.KeyEvent)
			 {
				 case LeftClick:selection--;
			   if (selection < 0) selection = buttonParams->ButtonNum - 1;break;
				 case RightClick:selection++;
			   if (selection > buttonParams->ButtonNum - 1) selection = 0;
			 }
			 break;
			}
			vTaskDelay(10/portTICK_RATE_MS);
		 }
		}
	}
}

/**
  * @brief  Show buttons on screen

  * @param  buttonString:A string which contains the strings
	  of the buttons,use'\n'to seperate two strings
		e.g."string1\nstring2\nstring3"

						buttonsNum:The quantity of the buttons that displayed

						defaultValue:The default selection of the buttons

						positions:A struct array contains the positions
	  of the buttons

	@retval The handler will send the result to UI_ButtonMsg when confirmation got
  */
void UI_Button_Init(UI_Button_Param_Struct * buttonParams)
{
	UI_ButtonMsg=xQueueCreate(1, sizeof(u8));
	xTaskCreate(UI_Button_Handler, "UI_Button Handler",
	256, buttonParams, UI_BUTTON_HANDLER_PRIORITY, NULL);
}		

