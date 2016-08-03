//File Name   Flash_Test.c
//Description Flash test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x_flash.h"

#include "SSD1306.h"

#include "Flash_Test.h"

#include "UI_Dialogue.h"


#define FLASH_TEST_START_ADDR 0x0807b000
#define FLASH_SECTOR_SIZE 2048

void Flash_Test_Run()
{
 u32 times=0;
 u32 i;
 char tempString[20];
 bool invertBlock;
 bool tempered=false;
 FLASH_Unlock();
 FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
 OLED_Clear();
 ShowDialogue("Flash Test","","");
 for(;;)
 {
  FLASH_ErasePage(FLASH_TEST_START_ADDR);
  for(i=0;i<FLASH_SECTOR_SIZE/4;i++)
  {
   if(invertBlock)
   FLASH_ProgramWord(FLASH_TEST_START_ADDR+i*4,0xff00);
   else 
   FLASH_ProgramWord(FLASH_TEST_START_ADDR+i*4,0x00ff);
  }
  for(i=0;i<FLASH_SECTOR_SIZE/4;i++)
  {
   if(invertBlock)
   {
    if(*(u32*)(FLASH_TEST_START_ADDR+i*4)!=0xff00)
      {
       tempered=true;
       break;
      }
   }
   else 
   {
    if(*(u32*)(FLASH_TEST_START_ADDR+i*4)!=0x00ff)
      {
       tempered=true;
       break;
      }
   }
  }
  times++;
  sprintf(tempString,"%d",times);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
  OLED_ShowAnyString(5,16,tempString,NotOnSelect,16);
  OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
  invertBlock=!invertBlock;
  if(tempered) break;
 }
 while(1);
}
