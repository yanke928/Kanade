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
 u8 frame[1024];
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
 OLED_WR_Byte(0xa0, OLED_CMD);
 OLED_WR_Byte(0xc8, OLED_CMD);
 while(1)
 {
	for(currentAddr=0;currentAddr<maxAddr;currentAddr++)	
	{
		res=f_read(&video, frame, 1023, &dataRead);
		if(res!=FR_OK) goto ReadFileFailed;
//		res=f_read(&video, OLED_GRAM+512, 512, &dataRead);
//		if(res!=FR_OK) goto ReadFileFailed;
		Draw_BMP(0,0,128,7,frame);
		f_lseek(&video,currentAddr*1024);
		vTaskDelayUntil(&xLastWakeTime, 33 / portTICK_RATE_MS);
	} 
	f_lseek(&video,0);
	dataRead=0;
 }
 ReadFileFailed:
 {
  OLED_ShowString(0,0,"Read File Failed");
	 vTaskDelete(NULL);
 }
}

void BadApplePlayer_Init()
{
  xTaskCreate(BadApplePlayer,"Bad Apple Player",
	600,NULL,BAD_APPLE_PLAYER_PRIORITY,NULL); 
}



