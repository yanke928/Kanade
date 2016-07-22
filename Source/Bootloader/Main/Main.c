#include "stm32f10x.h"

#include "SSD1306.h"
#include "Keys.h"
#include "JumpToApp.h"

#include "sdcard.h"
#include "sdcardff.h"
#include "W25Q64.h"
#include "W25Q64ff.h"

#include "Hex2Bin.h"

#include "ff.h"
#include "UI.h"

void AppUpdate(void);

int main(void)
{
	bool tempered=false;
	STM32_Init();
	Keys_GPIO_Init();
	if (LEFT_KEY == KEY_ON)
	{
		update:
		OLED_Init();
		if(tempered)
		{
		 ShowSystemTempered();
		 while(1);
		}
		else
		ShowHomePage();
		SDCard_Init();
		W25Q64_Init();
		while (1)
		{
			if (MIDDLE_KEY == KEY_ON)
			{
				while (MIDDLE_KEY == KEY_ON);
				AppUpdate();
			}
		}
	}
	Jump2App();
	tempered=true;
	goto update;
}

void AppUpdate(void)
{
	FIL firmware;
	bool checkOK;
	if ((!SDCardMountStatus) && (!SPIFlashMountStatus)) ShowNoStorage();
	if (f_open(&firmware, "0:/Kanade.hex", FA_OPEN_EXISTING|FA_READ) == FR_OK) goto update;
	if (f_open(&firmware, "1:/Kanade.hex", FA_OPEN_EXISTING|FA_READ) == FR_OK) goto update;
	ShowNoFile();
	while (1);
update:
	ShowCheckingFile();
	checkOK=CheckAHexFile(&firmware);
	if(!checkOK) 
	{
	 ShowCheckNotOK();
	 while(1);
	}
	ShowUpdating();
	WriteHexToROM(&firmware);
  f_close(&firmware);
  f_mount(NULL,"0:/",1);
  f_mount(NULL,"1:/",1);
	while (1);
}



