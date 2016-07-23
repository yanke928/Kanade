#include "stm32f10x.h"

#include "Hex2Bin.h"

#include "ff.h"

#include "UI.h"

#include "Keys.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//#define BOOTADD	0x08008000 

bool ReadLineInBuff(char *strLine, char* pBuff, u32 nSize, bool bInit)
{
	static u32 nStart;
	static u32 nPos;
	static u32 nRemain = 0;
	u32 len;
	if (bInit)
	{
		nStart = 0;
		nPos = 0;
		return true;
	}
	if (nStart == 0 && nPos == 0 && nRemain)
	{
		for (nPos = nStart; nPos < nSize; nPos++)
		{
			if (pBuff[nPos] == '\n' &&  strLine[nRemain - 1] == '\r')
			{
				len = nPos - nStart;
				strLine[nRemain - 1] = '\0';
				nStart = nPos + 1;
				nRemain = 0;
				return true;
			}

			if (pBuff[nPos] == '\r' && pBuff[nPos + 1] == '\n')
			{
				len = nPos - nStart;
				if (len >= 60)
				{
					return false;
				}
				memcpy(strLine + nRemain, pBuff + nStart, len);
				strLine[len + nRemain] = '\0';
				nStart = nPos + 2;
				nRemain = 0;
				return true;
			}
		}
	}

	if (nStart >= nSize)
	{
		return false;
	}


	for (nPos = nStart; nPos < nSize; nPos++)
	{
		if (pBuff[nPos] == '\r' && pBuff[nPos + 1] == '\n')
		{
			len = nPos - nStart;
			if (len >= 60)
			{
				return false;
			}
			memcpy(strLine, pBuff + nStart, len);
			strLine[len] = '\0';
			nStart = nPos + 2;
			nRemain = 0;
			return true;
		}
	}

	len = nPos - nStart;
	if (len >= 60)
	{
		return false;
	}
	memcpy(strLine, pBuff + nStart, len);
	strLine[len] = '\0';
	nRemain = len;
	nStart = nSize;
	return false;
}

char Hex2Bin(char* pChar)
{

	char h, l;
	h = pChar[0];
	l = pChar[1];

	if (l >= '0'&&l <= '9')
		l = l - '0';
	else if (l >= 'a' && l <= 'f')
		l = l - 'a' + 0xa;
	else if (l >= 'A' && l <= 'F')
		l = l - 'A' + 0xa;

	if (h >= '0'&&h <= '9')
		h = h - '0';
	else if (h >= 'a' && h <= 'f')
		h = h - 'a' + 0xa;
	else if (h >= 'A' &&h <= 'F')
		h = h - 'A' + 0xa;

	return (char)h * 16 + l;

}

bool CheckHexFile(char *source, u32 MaxDataSize, char *hexLineBuffer)
{
	char binBuffer[30];
	u16 lenTemp, loopTemp;
	u16 checksum;
	u32 bootAdd;
	//static char hexLineBuffer[60];

	if (MaxDataSize == 0 || source == NULL)
	{
		return false;
	}
	ReadLineInBuff(hexLineBuffer, source, MaxDataSize, true);

	while (ReadLineInBuff(hexLineBuffer, source, MaxDataSize, false))
	{
		lenTemp = strlen(hexLineBuffer);
		if (hexLineBuffer[0] != ':' || lenTemp < 11)
		{
			return false;
		}
		checksum = 0;
		for (loopTemp = 0; loopTemp < (lenTemp / 2); loopTemp++)
		{
			binBuffer[loopTemp] = Hex2Bin(hexLineBuffer + (loopTemp * 2) + 1);
			checksum += binBuffer[loopTemp];
		}
		if (checksum & 0xff)
		{
			return false;
		}

		if (0x05 == binBuffer[3])
		{
			bootAdd = (u32)binBuffer[4] << 24 | (u32)binBuffer[5] << 16 | (u32)binBuffer[6] << 8 | (u32)binBuffer[7];
			if ((bootAdd & FLASH_APP_ADDR) == FLASH_APP_ADDR)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	}
	return 	true;
}

void ShowFatfsErrorCode(u8 err)
{
	char string[20] = "ErrCode:";

	if (err / 100)
	{
		string[8] = err / 100 + '0';
		string[9] = err % 100 / 10 + '0';
		string[10] = err % 10 + '0';
		string[11] = 0;
	}
	else if (err / 10)
	{
		string[8] = err / 10 + '0';
		string[9] = err % 10 + '0';
		string[10] = 0;
	}
	else
	{
		string[8] = err + '0';
		string[9] = 0;
	}
	ShowDialogue("Error", "fatfs:Error", string);
}

bool CheckAHexFile(FIL* firmware)
{
	FRESULT res;
	u32 byteRead;
	u32 fileSize = firmware->fsize;
	u32 sizeRead = 0;
	char U8B[STM32_SECTOR_SIZE];
	char hexLineBuffer[60];
	bool checkOK;
	ProgressBar_Init();
	for (;;)
	{
		res = f_read(firmware, U8B, sizeof U8B, &byteRead);
		sizeRead += byteRead;
		UpdateProgressBar(0, fileSize, sizeRead);
		if (res || byteRead == 0)
		{
			if (res != FR_OK)
			{
				ShowFatfsErrorCode(res);
				while (1);
			}
			break; /* error or eof */
		}
		if (!CheckHexFile(U8B, byteRead, hexLineBuffer))
		{
			checkOK = false;
			break;
		}
		checkOK = true;
	}
	return checkOK;
}

bool SD2ROM(char *source, UINT MaxDataSize, char *hexLineBuffer, char *stringBuff)
{
	char binBuffer[30];
	u16 lenTemp, loopTemp;
	u16 checksum;
	u16 tempOne = 0;
	volatile u8 lineType, byteCount;

	volatile static u16 buffAddress = 0;
	volatile static u16 Page = 0;


	if (MaxDataSize == 0 || source == NULL)
	{
		return false;
	}
	ReadLineInBuff(hexLineBuffer, source, MaxDataSize, true);

	while (ReadLineInBuff(hexLineBuffer, source, MaxDataSize, false))
	{
		lenTemp = strlen(hexLineBuffer);
		if (hexLineBuffer[0] != ':' || lenTemp < 11)
		{
			return false;
		}
		checksum = 0;
		for (loopTemp = 0; loopTemp < (lenTemp / 2); loopTemp++)
		{
			binBuffer[loopTemp] = Hex2Bin(hexLineBuffer + (loopTemp * 2) + 1);
			checksum += binBuffer[loopTemp];
		}
		if (checksum & 0xff)
		{
			return false;
		}
		else
		{
			lineType = binBuffer[3];
			byteCount = binBuffer[0];

			switch (lineType)
			{
			case 0:
				if (buffAddress + byteCount >= STM32_SECTOR_SIZE)
				{
					tempOne = STM32_SECTOR_SIZE - buffAddress;
					byteCount = byteCount - tempOne;
					memcpy(stringBuff + buffAddress, binBuffer + 4, tempOne);

					STMFLASH_WriteOnePage(FLASH_APP_ADDR + Page*STM32_SECTOR_SIZE, (u16*)stringBuff, STM32_SECTOR_SIZE / 2);
					buffAddress = 0;
					Page++;
				}
				memcpy(stringBuff + buffAddress, binBuffer + 4 + tempOne, byteCount);
				if (tempOne)
				{
					tempOne = 0;
				}
				buffAddress += byteCount;
				break;
			case 1:
				tempOne = buffAddress;
				STMFLASH_WriteOnePage(FLASH_APP_ADDR + STM32_SECTOR_SIZE*Page, (u16*)stringBuff, (tempOne + 1) / 2);
				return true;
			case 2:
				break;
			case 4:
				break;
			case 3:
				break;
			default:
				break;
			}
		}

	}
	return true;
}

bool WriteHexToROM(FIL* firmware)
{
	FRESULT res;
	char U8B[STM32_SECTOR_SIZE * 2];
	char hexLineBuffer[60];
	char U8Buff[STM32_SECTOR_SIZE * 2];
	u32 byteRead;
	u32 sizeRead = 0;
	u32 fileSize = firmware->fsize;
	ProgressBar_Init();
	res = f_lseek(firmware, 0);
	if (res)
	{
		ShowFatfsErrorCode(res);
		while (1);
	}
	for (;;)
	{
		res = f_read(firmware, U8B, sizeof U8B, &byteRead);
		sizeRead += byteRead;
		UpdateProgressBar(0, fileSize, sizeRead);
		if (res || byteRead == 0)
		{
			if (res != FR_OK)
			{
				ShowFatfsErrorCode(res);
				while (1);
			}
			break; /* error or eof */
		}
		if (!SD2ROM(U8B, byteRead, hexLineBuffer, U8Buff))
		{
			return false;
		}
	}
	return true;
}
