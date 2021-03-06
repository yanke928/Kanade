#include "stm32f10x.h"

#include "Moha2Bin.h"

#include "ff.h"

#include "SSD1306.h"

#include "UI.h"

#include "Keys.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static unsigned int CRC32[256];
static char Init = 0;

unsigned int  GetCRC32(unsigned char *buf, int len);

/**
  * @brief   Get "moha" file info
	
  * @retval : 0(it is not a "moha" file or illegal)
	length(effective data length in bytes)
  */
u32 GetMohaFileInfo(FIL* firmware)
{
	FRESULT res;
	u32 byteRead;
	u32 size;
	u32 tag;
	u8 buff[5];
	f_lseek(firmware, 0);
	
	/*Read "moha" tag*/
	res = f_read(firmware, buff, sizeof("moha") - 1, &byteRead);
	if (res != FR_OK) return 0;

	/*Check "moha" tag*/
	buff[4] = 0;
	tag = strcmp((char*)buff, "moha");
	if (tag != 0) return 0;

	/*Get effective data length*/
	res = f_read(firmware, &size, sizeof(u32), &byteRead);
	if (res != FR_OK) return 0;

	return size;
}

/**
  * @brief   Run "moha" file CRC check
	
  * @retval : 255(Bad "moha" file)
  0(CRC check passed)
  and fatfs errCode
  */
u8 CheckAMohaFile(FIL* firmware)
{
	FRESULT res;
	u32 byteRead;
	u32 sizeRead = 8;
	u32 blockCount;
	u32 binarySize;
	u32 fileSize;
	u32 i;

	u32 expectedCRC;
	u32 actualCRC;

	u8 buff[STM32_SECTOR_SIZE + 4];

	ProgressBar_Init();

	/*Get effective binary length and calc block count*/
	binarySize = GetMohaFileInfo(firmware);
	if (binarySize == 0) return 255;
	blockCount = binarySize / STM32_SECTOR_SIZE + 1;

	fileSize = firmware->fsize;

  /*Point to effective data*/
	f_lseek(firmware, 8);

	for (i = 0; i < blockCount; i++)
	{
		/*Read a block*/
		res = f_read(firmware, buff, sizeof buff, &byteRead);
		sizeRead += byteRead;
		if (res != FR_OK) return res;
		if (byteRead == 0) return 255;
		
		actualCRC = GetCRC32(buff, STM32_SECTOR_SIZE);
		expectedCRC = *(u32*)(buff + STM32_SECTOR_SIZE);
		
		if (actualCRC != expectedCRC) return 255;
		UpdateProgressBar(0, fileSize, sizeRead);
	}
	return 0;
}


/**
  * @brief   Write "moha" file to ROM
	
  * @retval  255:An error occurred while writing

             0: Operation succeeds
  */
u8 WriteFirmwareToROM(FIL* firmware)
{
	FRESULT res;
	u32 blockCount;
	u32 binarySize;
	u32 fileSize;
	u32 byteRead;
	u32 sizeRead = 8;
	u8 buff[STM32_SECTOR_SIZE + 4];
	u32 i;

	ProgressBar_Init();
	UpdateProgressBar(0, 1000, 0);

	binarySize = GetMohaFileInfo(firmware);
	blockCount = binarySize / STM32_SECTOR_SIZE + 1;

	fileSize = firmware->fsize;

	f_lseek(firmware, 8);

	for (i = 0; i < blockCount; i++)
	{
		res = f_read(firmware, buff, sizeof buff, &byteRead);
		sizeRead += byteRead;
		if (res != FR_OK) return 255;
		if (byteRead == 0) return 255;
		STMFLASH_WriteOnePage(FLASH_APP_ADDR + STM32_SECTOR_SIZE*i, (u16*)buff, STM32_SECTOR_SIZE / 2);
		UpdateProgressBar(0, fileSize, sizeRead);
	}
	return 0;
}

/**
  * @brief   Init CRC value table
  */
static void Init_table()
{
	int   i, j;
	unsigned int  crc;
	for (i = 0; i < 256; i++)
	{
		crc = i;
		for (j = 0; j < 8; j++)
		{
			if (crc & 1)
			{
				crc = (crc >> 1) ^ 0xEDB88320;
			}
			else
			{
				crc = crc >> 1;
			}
		}
		CRC32[i] = crc;
	}
}


/**
  * @brief   Get CRC result 

    @param buf: Pointer to the data being checked

           len:Length of data being checked
  */
unsigned int  GetCRC32(unsigned char *buf, int len)
{
	unsigned int  ret = 0xFFFFFFFF;
	int   i;
	if (!Init)
	{
		Init_table();
		Init = 1;
	}
	for (i = 0; i < len; i++)
	{
		ret = CRC32[((ret & 0xFF) ^ buf[i])] ^ (ret >> 8);
	}
	ret = ~ret;
	return ret;
}


