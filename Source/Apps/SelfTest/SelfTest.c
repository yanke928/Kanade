//File Name   SelfTest.c
//Description Self test

#include <string.h>

#include "stm32f10x.h"

#include "SSD1306.h"
#include "Music.h"
#include "LED.h"
#include "Cooling_Fan.h"
#include "FastCharge_Trigger_Circuit.h"
#include "MCP3421.h"

#include "ff.h"

#include "sdcard.h"
#include "sdcardff.h"
#include "W25Q64.h"
#include "W25Q64ff.h"

#include "UI_Print.h"

#include "ExceptionHandle.h" 

#include "SelfTest.h"

#define WriteToTerminal(str) strcat(terminal,str)

void LEDBuzzerFanTest(char * terminal);
void SD_Card_Test(char *terminal);
void SPI_Flash_Test(char *terminal);
void DataPin_Control_Test(char *terminal);

typedef void(*TestItem)(char*);

TestItem TestItems[] = {
LEDBuzzerFanTest,
DataPin_Control_Test,
SD_Card_Test,
SPI_Flash_Test,
NULL
};

void SelfTest()
{
	char* termialBuff = pvPortMalloc(200);
	u8 i;
	if (termialBuff == NULL) ShowFault("A Malloc Err");
	OLED_Clear();
	termialBuff[0] = 0;
	for (i = 0;; i++)
	{
		if (TestItems[i] == NULL) break;
		else
		{
			TestItems[i](termialBuff);
			OLED_Clear();
			UI_PrintMultiLineString(0, 0, 127, 63, termialBuff, NotOnSelect, 8);
			vTaskDelay(500);
		}
	}
	while (1) vTaskDelay(100);
}

void LEDBuzzerFanTest(char * terminal)
{
	LED_Animate_Init(LED_Test);
	SoundStart(Tori_No_Uta);
	Fan_Send_Command(Turn_On);
	WriteToTerminal("LED&Buzzer&Fan On\n");
}

enum {
	File_Create_Failure,
	File_Write_Failure,
	File_Length_Mismatch,
	File_Close_Failure,
	File_Open_Failure,
	File_Read_Failure,
	File_Verify_Failure,
	File_Delete_Failure,
	RW_OK
};

int MemTest(char volume)
{
	FRESULT res;
	FIL testFile;
	u8 buff[128];
	u16 i, j;
	u32 byteWritten;

	sprintf((char*)buff, "%c:/TestFile", volume);

	res = f_open(&testFile, (char*)buff, FA_WRITE | FA_OPEN_ALWAYS);
	if (res != FR_OK)
	{
		return(File_Create_Failure);
	}

	memset(buff, '0', sizeof(buff));

	for (i = 0; i < 2000; i++)
	{
		res = f_write(&testFile, buff, sizeof(buff), &byteWritten);
		if (res != FR_OK)
		{
			return(File_Write_Failure);
		}
		else if (byteWritten != sizeof(buff))
		{
			return(File_Length_Mismatch);
		}
	}

	res = f_close(&testFile);
	if (res != FR_OK)
	{
		return(File_Close_Failure);
	}

	sprintf((char*)buff, "%c:/TestFile", volume);

	res = f_open(&testFile, (char*)buff, FA_READ);
	if (res != FR_OK)
	{
		return(File_Open_Failure);
	}

	for (i = 0; i < 2000; i++)
	{
		res = f_read(&testFile, buff, sizeof(buff), &byteWritten);
		if (res != FR_OK)
		{
			return(File_Read_Failure);
		}
		for (j = 0; j < byteWritten; j++)
		{
			if (buff[j] != '0')
			{
				return(File_Verify_Failure);
			}
		}
	}

	res = f_close(&testFile);
	if (res != FR_OK)
	{
		return(File_Close_Failure);
	}

	sprintf((char*)buff, "%c:/TestFile", volume);

	res = f_unlink((char*)buff);
	if (res != FR_OK)
	{
		return(File_Delete_Failure);
	}

	return(RW_OK);
}

void SD_Card_Test(char *terminal)
{
	int res;
	if (!SDCardMountStatus)
	{
		WriteToTerminal("SD:Mount failed\n");
	}
	else
	{
		res = MemTest('0');
		switch (res)
		{
		case RW_OK:WriteToTerminal("SD:R/W OK\n"); break;
		case File_Create_Failure:WriteToTerminal("SD:Create failed\n"); break;
		case File_Write_Failure:WriteToTerminal("SD:Write failed\n"); break;
		case File_Length_Mismatch:WriteToTerminal("SD:Length mismatch\n"); break;
		case File_Close_Failure:WriteToTerminal("SD:Close failed\n"); break;
		case File_Open_Failure:WriteToTerminal("SD:Open failed\n"); break;
		case File_Read_Failure:WriteToTerminal("SD:Read failed\n"); break;
		case File_Verify_Failure:WriteToTerminal("SD:Verify failed\n"); break;
		case File_Delete_Failure:WriteToTerminal("SD:Delete failed\n");
		}
	}
}

void SPI_Flash_Test(char *terminal)
{
	int res;
	if (!SPIFlashMountStatus)
	{
		WriteToTerminal("Flash:Mount failed\n");
	}
	else
	{
		res = MemTest('1');
		switch (res)
		{
		case RW_OK:WriteToTerminal("Flash:R/W OK\n"); break;
		case File_Create_Failure:WriteToTerminal("Flash:Create failed\n"); break;
		case File_Write_Failure:WriteToTerminal("Flash:Write failed\n"); break;
		case File_Length_Mismatch:WriteToTerminal("Flash:Length mismatch\n"); break;
		case File_Close_Failure:WriteToTerminal("Flash:Close failed\n"); break;
		case File_Open_Failure:WriteToTerminal("Flash:Open failed\n"); break;
		case File_Read_Failure:WriteToTerminal("Flash:Read failed\n"); break;
		case File_Verify_Failure:WriteToTerminal("Flash:Verify failed\n"); break;
		case File_Delete_Failure:WriteToTerminal("Flash:Delete failed\n");
		}
	}
}

void DataPin_Control_Test(char *terminal)
{
	FastCharge_Trigger_GPIO_Enable();

	SetDM_GND();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDM >= 0.2)
	{
		WriteToTerminal("DM_DP:DM-GND Failed\n");
		return;
	}

	SetDM_0V6();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDM >= 0.8 || CurrentMeterData.VoltageDM <= 0.4)
	{
		WriteToTerminal("DM_DP:DM-0V6 Failed\n");
		return;
	}

	SetDM_2V7();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDM >= 2.9 || CurrentMeterData.VoltageDM <= 2.5)
	{
		WriteToTerminal("DM_DP:DM-2V7 Failed\n");
		return;
	}

	SetDM_3V3();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDM >= 3.4 || CurrentMeterData.VoltageDM <= 3.1)
	{
		WriteToTerminal("DM_DP:DM-2V7 Failed\n");
		return;
	}

	SetDP_GND();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDP > 0.2)
	{
		WriteToTerminal("DM_DP:DP-GND Failed\n");
		return;
	}

	SetDP_0V6();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDP >= 0.8 || CurrentMeterData.VoltageDP <= 0.4)
	{
		WriteToTerminal("DM_DP:DP-0V6 Failed\n");
		return;
	}

	SetDP_2V7();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDP >= 2.9 || CurrentMeterData.VoltageDP <= 2.5)
	{
		WriteToTerminal("DM_DP:DP-2V7 Failed\n");
		return;
	}

	SetDP_3V3();
	vTaskDelay(200 / portTICK_RATE_MS);
	if (CurrentMeterData.VoltageDP >= 3.4 || CurrentMeterData.VoltageDP <= 3.1)
	{
		WriteToTerminal("DM_DP:DP-3V3 Failed\n");
		return;
	}
	FastCharge_Trigger_Release();
	WriteToTerminal("DM_DP:OK\n");
}
