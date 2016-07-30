#include <stdbool.h>
#include "stm32f10x.h"

#include "SSD1306.h"

#include "UI.h"

/**
  * @brief  Get Graphical length of given string

  * @param  string[]:Given string

	  @retval Length of given string
  */
u8 GetStringGraphicalLength(const char string[])
{
	u8 length = 0;
	for (; *string != 0; string++)
	{
		if (*string > 127)
		{
			string += 2;
			length++;
		}
		length++;
	}
	return(length);
}

void ShowDialogue(const char* titleString, const char* subString1, const char* subString2)
{
	u8 length;
	OLED_Clear();
	OLED_DrawRect(0, 0, 127, 63, 1);
	OLED_ShowAnyString(3, 1, titleString, NotOnSelect, 12);
	length = GetStringGraphicalLength(subString1);
	OLED_ShowAnyString(64 - length * 4, 18, subString1, NotOnSelect, 16);
	length = GetStringGraphicalLength(subString2);
	OLED_ShowAnyString(64 - length * 4, 40, subString2, NotOnSelect, 16);
	OLED_InvertRect(1, 1, 126, 13);
	OLED_Refresh_Gram();
}

void ShowHomePage()
{
	ShowDialogue("Firmware Update", "Press MiddleKey", "to continue...");
}

void ShowNoStorage()
{
	ShowDialogue("Error", "No storage!!", "System halted!!");
}

void ShowNoFile()
{
	ShowDialogue("Error", "No Firmware!!", "System halted!!");
}

void ShowCheckingFile()
{
	ShowDialogue("Firmware Update", "Checking...", "");
}

void ShowUpdating()
{
	ShowDialogue("Firmware Update", "Updating...", "");
}

void ShowCheckNotOK()
{
	ShowDialogue("Error", "Moha Firmware", "tempered!!");
}

void ShowSystemTempered()
{
	ShowDialogue("System Tempered", "System needs", "to be fixed!!");
}

void ShowUpdateFailed()
{
	ShowDialogue("Error", "Update failed", "while writing!!");
}

void ShowUpdateSuccess()
{
	ShowDialogue("Omededo", "Update success!", "Press to reset!");
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

void ProgressBar_Init()
{
	OLED_FillRect(12, 40, 116, 54, 0);
	OLED_DrawRect(12, 42, 116, 54, 1);
}

void UpdateProgressBar(u32 min, u32 max, u32 current)
{
	u8 finishedPixels = 100 * (float)(current - min) / (float)(max - min);
	OLED_FillRect(14, 44, 14 + finishedPixels, 53, 1);
	OLED_FillRect(15 + finishedPixels, 44, 115, 53, 0);
	OLED_Refresh_Gram();
}
