//File Name   ExceptionHandle.c
//Description : Exception handlers 

#include <stdio.h>

#include "FreeRTOS_Standard_Include.h"

#include "UI_Dialogue.h"
#include "UI_Utilities.h"
#include "UI_Print.h"

#include "SSD1306.h"
#include "Temperature_Sensors.h"
#include "Keys.h"
#include "LED_Animate.h"
#include "Music.h"
#include "Digital_Load.h"
#include "VirtualRTC.h"

#include "USBMeter.h"
#include "LegacyTest.h"

#include "Settings.h"
#include "MultiLanguageStrings.h"

#include "ExceptionHandle.h"

#define EBD_EXCEPTION_HANDLER_PRIORITY tskIDLE_PRIORITY+6

/**
  * @brief This hook function helps to report stackoverflow exception

  * @retval None
  */
void vApplicationStackOverflowHook(TaskHandle_t xTask,
	signed char *pcTaskName)
{
	u8 t;
	xSemaphoreGive(OLEDRelatedMutex);
	SetUpdateOLEDJustNow();
	OLED_Clear();
	ShowDialogue("StackOverFlow", "", "",false,false);
	OLED_ShowAnyString(4, 16, "StackOverFlow in", NotOnSelect, 12);
	OLED_ShowAnyString(4, 28, "\"", NotOnSelect, 12);
	OLED_ShowAnyString(10, 28, (char *)pcTaskName, NotOnSelect, 12);
	t = GetStringGraphicalLength((char *)pcTaskName);
	OLED_ShowAnyString(10 + t * 6, 28, "\"", NotOnSelect, 12);
	taskENTER_CRITICAL();
	while (1);
}

/**
  * @brief Show application create failed

  * @retval None
  */
void ApplicationNewFailed(const char * appName)
{
	char tempString[60];
	xSemaphoreGive(OLEDRelatedMutex);
	taskENTER_CRITICAL();
	OLED_Clear();
	ShowDialogue("App Create Failure", "", "",false,false);
	sprintf(tempString, "RAM full while creating task \"%s\"", appName);
	UI_PrintMultiLineString(4, 15, 125, 63, tempString, NotOnSelect, 12);
	OLED_Refresh_Gram();
	for (;;);
}

/**
  * @brief Show xx Fault occurred

  * @retval None
  */
void ShowFault(char * string)
{
	xSemaphoreGive(OLEDRelatedMutex);
	taskENTER_CRITICAL();
	//SetUpdateOLEDJustNow();
	OLED_Clear();
	ShowDialogue("System Fault", "", "",false,false);
	OLED_ShowAnyString(4, 16, string, NotOnSelect, 16);
	OLED_ShowAnyString(4, 42, "Occurred!!", NotOnSelect, 16);
	OLED_Refresh_Gram();
	for (;;);
}

void Show_OverHeat_Temperature(u8 sensor)
{
	char tempString[20];
	u8 addr;
	u8 length;
	OLED_ShowAnyString(1, 42, "               ", NotOnSelect, 16);
	if(sensor==0)
	sprintf(tempString, "%.1fC>>%.1fC", MOSTemperature, 
	(float)CurrentSettings->Protect_Settings.InternalTemperature_Max-CurrentSettings->Protect_Settings.Protection_Resume_Gap);
	else
	sprintf(tempString, "%.1fC>>%.1fC", ExternalTemperature, 
	(float)CurrentSettings->Protect_Settings.ExternalTemperature_Max-CurrentSettings->Protect_Settings.Protection_Resume_Gap);		
	length = GetStringGraphicalLength(tempString);
	addr = 63 - length * 4;
	OLED_ShowAnyString(addr, 42, tempString, NotOnSelect, 16);
}

/**
  * @brief  This function helps to deal with overheat exception

  * @retval None
  */
void System_OverHeat_Exception_Handler(u8 status, Legacy_Test_Param_Struct* params)
{
	bool i;
	u8 sensor;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	SoundStart(Alarm);
	if(MOSTemperature > CurrentSettings->Protect_Settings.InternalTemperature_Max)
	{
	 sensor=0;
	}
	else sensor=1;
	Show_OverHeat_Temperature(sensor);
	if (status != USBMETER_ONLY)
	{
	  if(sensor==0)
		ShowDialogue(SystemOverHeat_Str[CurrentSettings->Language],
			TestPaused_Str[CurrentSettings->Language], "",false,false);
		else
		ShowDialogue(ExternalOverHeat_Str[CurrentSettings->Language],
			TestPaused_Str[CurrentSettings->Language], "",false,false);			
		vTaskSuspend(RecordHandle);
		VirtualRTC_Pause();
		if (status == LEGACY_TEST)
		{
      Send_Digital_Load_Command(0,Load_Stop);
			vTaskDelay(200/portTICK_RATE_MS);
		}
	}
	else
	{
		if(sensor==0)
		ShowDialogue(SystemOverHeat_Str[CurrentSettings->Language],
			SystemOverHeat_Str[CurrentSettings->Language], "",false,false);
		else
		ShowDialogue(ExternalOverHeat_Str[CurrentSettings->Language],
			ExternalOverHeat_Str[CurrentSettings->Language], "",false,false);			
	}
	for (;;)
	{
		Show_OverHeat_Temperature(sensor);
		vTaskDelay(500 / portTICK_RATE_MS);
		if (((MOSTemperature < CurrentSettings->Protect_Settings.InternalTemperature_Max-CurrentSettings->Protect_Settings.Protection_Resume_Gap)
			  &&(sensor==0))||
		   ((ExternalTemperature < CurrentSettings->Protect_Settings.ExternalTemperature_Max-CurrentSettings->Protect_Settings.Protection_Resume_Gap)
			  &&(sensor==1)))
		{
			if (status == LEGACY_TEST)
			{
				StartOrRecoverLoad(params, &i);
			}
			break;
		}
	}
	SoundStop();
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	if (status != USBMETER_ONLY)
	{
		vTaskResume(RecordHandle);
		VirtualRTC_Resume();
	}
}

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	ShowFault("An NMI Fault");
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	ShowFault("A Hard Fault");
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	ShowFault("A Mem Fault");
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	ShowFault("A Bus Fault");
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	ShowFault("An Usage Fault");
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
  //void SVC_Handler(void)
  //{
  //}

  /**
	* @brief  This function handles Debug Monitor exception.
	* @param  None
	* @retval None
	*/
void DebugMon_Handler(void)
{
	ShowFault("DebugMon Fault");
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval : None
  */
  //void PendSV_Handler(void)
  //{
  //}

  /**
	* @brief  This function handles SysTick Handler.
	* @param  None
	* @retval : None
	*/
	//void SysTick_Handler(void)
	//{

	//}

