#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"

#include "misc.h"

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

#define UPDATE_FLAG_ADDR 0x08007800

void AppUpdate(void);

void NVIC_DeInit(void);

void ClearUpdateFlag(void);

__asm void SystemReset(void)
{
 MOV R0, #1           //; 
 MSR FAULTMASK, R0    //; 清除FAULTMASK 禁止一切中断产生
 LDR R0, =0xE000ED0C  //;
 LDR R1, =0x05FA0004  //; 
 STR R1, [R0]         //; 系统软件复位   
 
deadloop
    B deadloop        //; 死循环使程序运行不到下面的代码
}
 

int main(void)
{
	bool tempered = false;
	RCC_DeInit();
	NVIC_DeInit();
  DMA_DeInit(DMA2_Channel4);
  DMA_DeInit(DMA1_Channel1);
	NVIC_SetVectorTable(NVIC_VectTab_FLASH ,0);
	STM32_Init();
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  PWR_BackupAccessCmd(ENABLE);		
	Keys_GPIO_Init();
	if (LEFT_KEY == KEY_ON || BKP_ReadBackupRegister(BKP_DR2) == 0x0001)
	{
	update:
		OLED_Init();
		OLED_Clear();
		if (tempered)
		{
			ShowSystemTempered();
			while (1);
		}
		else if(BKP_ReadBackupRegister(BKP_DR2) != 0x0001)
			ShowHomePage();
		SDCard_Init();
		W25Q64_Init();
		if(BKP_ReadBackupRegister(BKP_DR2) == 0x0001)
		{
		 ClearUpdateFlag();
		 AppUpdate();   
		}
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
	tempered = true;
	goto update;
}

void NVIC_DeInit(void) 
{ 
  u32 index = 0; 
   
  NVIC->ICER[0] = 0xFFFFFFFF; 
  NVIC->ICER[1] = 0x000007FF; 
  NVIC->ICPR[0] = 0xFFFFFFFF; 
  NVIC->ICPR[1] = 0x000007FF; 
   
  for(index = 0; index < 0x0B; index++) 
  { 
     NVIC->IP[index] = 0x00000000; 
  }  
}

void AppUpdate(void)
{
	FIL firmware;
	bool success;
	if ((!SDCardMountStatus) && (!SPIFlashMountStatus)) ShowNoStorage();
	if (f_open(&firmware, "0:/Kanade.hex", FA_OPEN_EXISTING | FA_READ) == FR_OK) goto update;
	if (f_open(&firmware, "1:/Kanade.hex", FA_OPEN_EXISTING | FA_READ) == FR_OK) goto update;
	ShowNoFile();
	while (1);
update:
	ShowCheckingFile();
	success = CheckAHexFile(&firmware);
	if (!success)
	{
		ShowCheckNotOK();
		while (1);
	}
	ShowUpdating();
	success = WriteHexToROM(&firmware);
	f_close(&firmware);
	f_mount(NULL, "0:/", 1);
	f_mount(NULL, "1:/", 1);
	if (success)
	{
		ShowUpdateSuccess();
		while (1)
		{
			if (MIDDLE_KEY == KEY_ON) 
			SystemReset();
		}
	}
	else
	{
		ShowUpdateFailed();
	}
	while (1);
}

void ClearUpdateFlag()
{	
 BKP_WriteBackupRegister(BKP_DR2, 0x0000);
}

