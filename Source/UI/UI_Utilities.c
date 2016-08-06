//File Name     UI_Utilities.c
//Description : UI Utilities

#include <string.h>
#include <stdio.h>

#include "SSD1306.h" 

#include "ff.h" 

#include "UI_Dialogue.h" 

#include "UI_Utilities.h" 

/**
  * @brief  Get central position(Start Position) of the string between x1,x2

  * @param  startPos:Position of start grid

			endPos:  Position of end grid

			stringLength:Length of the string

	  @retval Start position of the target string
  */
u8 GetCentralPosition(u8 startPos, u8 endPos, u8 stringLength)
{
	u8 pos;
	pos = startPos + (endPos - startPos) / 2;
	pos = pos - stringLength * 3;
	return(pos);
}

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

/**
  * @brief Get string length in bytes

	  @retval The offset
  */
u16 GetStringLengthInBytes(const char *string)
{
	u16 i = 0;
	while (string[i] != 0) i++;
	i++;
	return i;
}


/**
  * @brief  Draw or unDraw a vertical dashed grid with given density

* @param    x:Position

			drawOrUnDraw:Draw(1) or unDraw(1)

			gridsDensity:2 pixels per dot(HighDensity)

			gridsDensity:4 pixels per dot(LowDensity)

	  @retval None
  */
void DrawVerticalDashedGrid(u8 x, bool drawOrUnDraw, u8 gridsDensity)
{
	u8 i;
	/*If highDensity is given*/
	if (gridsDensity == HighDensity)
		for (i = 0; i < 64; i++)
		{
			if (i % 2 == 0)
			{
				OLED_DrawPoint(x, i, drawOrUnDraw);
			}
		}
	/*If lowDensity is given*/
	else
	{
		for (i = 0; i < 64; i++)
		{
			if ((i - 1) % 4 == 0)
			{
				OLED_DrawPoint(x, i, drawOrUnDraw);
			}
		}
	}
}

/**
  * @brief  Draw or unDraw a horizonal dashed grid with given density

* @param    y:Position

			drawOrUnDraw:Draw(1) or unDraw(1)

			gridsDensity:2 pixels per dot(HighDensity)

			gridsDensity:4 pixels per dot(LowDensity)

	  @retval None
  */
void DrawHorizonalDashedGrid(u8 y, bool drawOrUnDraw, u8 gridsDensity)
{
	u8 i;
	/*If highDensity is given*/
	if (gridsDensity == HighDensity)
		for (i = 0; i < 128; i++)
		{
			if (i % 2 == 0)
			{
				OLED_DrawPoint(i, y, drawOrUnDraw);
			}
		}
	/*If lowDensity is given*/
	else
		for (i = 0; i < 128; i++)
		{
			if ((i - 1) % 4 == 0)
			{
				OLED_DrawPoint(i, y, drawOrUnDraw);
			}
		}
}

/**
  * @brief  Show Disk IO Status

  * @retval None
  */
void ShowDiskIOStatus(u8 res)
{
	char tempString[20];
	if (res == FR_OK)
	{
		ShowSmallDialogue("File Created", 1000,true);
		return;
	}
	if (res == FR_NO_PATH)
	{
		ShowSmallDialogue("Path Not Found", 1000,true);
		return;
	}
	if (res == FR_INVALID_DRIVE)
	{
		ShowSmallDialogue("Invalid Disk", 1000,true);
		return;
	}
	if (res == FR_INVALID_NAME)
	{
		ShowSmallDialogue("Invalid Path", 1000,true);
		return;
	}
	if (res == FR_DENIED)
	{
		ShowSmallDialogue("Disk Full", 1000,true);
		return;
	}
	if (res == FR_EXIST)
	{
		ShowSmallDialogue("File Exist", 1000,true);
		return;
	}
	if (res == FR_NOT_READY)
	{
		ShowSmallDialogue("Disk Not Ready", 1000,true);
		return;
	}
	if (res == FR_WRITE_PROTECTED)
	{
		ShowSmallDialogue("Disk Protected", 1000,true);
		return;
	}
	if (res == FR_INT_ERR)
	{
		ShowSmallDialogue("Int. IO Error", 1000,true);
		return;
	}
	if (res == FR_NO_FILESYSTEM)
	{
		ShowSmallDialogue("No FATFS", 1000,true);
		return;
	}
	if (res == FR_NOT_ENABLED)
	{
		ShowSmallDialogue("Mount Failed", 1000,true);
		return;
	}
	if (res == FR_DISK_ERR)
	{
		ShowSmallDialogue("Disk Error", 1000,true);
		return;
	}
	sprintf(tempString,"Unknown_Err:%d",res);
	ShowSmallDialogue(tempString, 1000,true);
}
