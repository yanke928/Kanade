//File Name   ExceptionHandle.c
//Description : Exception handlers 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "UI_Dialogue.h"
#include "UI_Utilities.h"

#include "SSD1306.h"
#include "Keys.h"

#include "ExceptionHandle.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    signed char *pcTaskName )
{
 u8 t;
 xSemaphoreGive(OLEDRelatedMutex);
 SetUpdateOLEDJustNow();
 OLED_Clear();
 ShowDialogue("StackOverFlow","","");
 OLED_ShowAnyString(4,16,"StackOverFlow in",NotOnSelect,12);
 OLED_ShowAnyString(4,28,"'",NotOnSelect,12);
 OLED_ShowAnyString(10,28,(char *)pcTaskName,NotOnSelect,12);
 t=GetStringLength((char *)pcTaskName);
 OLED_ShowAnyString(10+t*6,28,"'",NotOnSelect,12);
 taskENTER_CRITICAL();
 while(1);
}

void ShowFault(char * string)
{
 xSemaphoreGive(OLEDRelatedMutex);
 taskENTER_CRITICAL();
 SetUpdateOLEDJustNow();	
 OLED_Clear();
 ShowDialogue("System Fault","","");
 OLED_ShowAnyString(4,16,string,NotOnSelect,16);
 OLED_ShowAnyString(4,42,"Occurred!!",NotOnSelect,16); 
 while(1)
 {
  if(MIDDLE_KEY==KEY_ON)
	{
	 while(MIDDLE_KEY==KEY_ON)
	 {
	  OLED_Clear();
	 }
	 return;
	}
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

