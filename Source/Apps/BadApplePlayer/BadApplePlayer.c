//File Name   £ºBadApplePlayer.c
//Description : BadApplePlayer 

#include "BadApplePlayer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sdcardff.h" 
#include "sdcard.h" 
#include "ff.h" 

#include "FreeRTOS_CLI.h"

#include "SSD1306.h"

#define BAD_APPLE_PLAYER_PRIORITY tskIDLE_PRIORITY+5

void BadApplePlayer(void *pvParameters)
{
 FRESULT res;
 FIL video;
 portTickType xLastWakeTime;
 u32 currentAddr;
 u32 maxAddr;
 u32 fileSize;
 u32 dataRead=0;
 char tempString[5];
 res = f_open(&video, "0:/BadApple.bin", FA_READ);
 if(res!=FR_OK)
 {
	OLED_ShowString(0,0,"No File");
  vTaskDelete(NULL);
 }
 fileSize = f_size(&video);
 maxAddr=fileSize/1024;
 xLastWakeTime=xTaskGetTickCount();
 UpdateOLEDJustNow=true;
 while(1)
 {
	for(currentAddr=0;currentAddr<maxAddr;currentAddr++)	
	{
		f_read(&video, OLED_GRAM, 1024, &dataRead);
		OLED_Refresh_Gram();
		sprintf(tempString,"%d",currentAddr);
		OLED_ShowString(0,0,tempString);
		vTaskDelayUntil(&xLastWakeTime, 50 / portTICK_RATE_MS);
	} 
	f_lseek(&video,0);
	dataRead=0;
 }
}

void BadApplePlayer_Init()
{
  xTaskCreate(BadApplePlayer,"Bad Apple Player",
	512,NULL,BAD_APPLE_PLAYER_PRIORITY,NULL); 
}



