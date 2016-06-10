//File Name   ExceptionHandle.c
//Description : Exception handlers 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "UI_Dialogue.h"
#include "UI_Utilities.h"
#include "SSD1306.h"

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

