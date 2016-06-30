//File Name    UI_Dialgram.c
//Description : Dialgram UI

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "Keys.h" 

#include "UI_Utilities.h" 
#include "UI_Dialgram.h"

#include "ExceptionHandle.h" 

#define UI_DIALGRAM_BROSWER_PRIORITY tskIDLE_PRIORITY+2

typedef struct
{
	u16 RecordLength;
	float MinValues[2];
	float MaxValues[2];
	float *DataSets[2];
	u16   LastPos;
	u16   NewPos;
	bool  ChangePos;
	bool  DrawPos;
}DialgramConfigurationStruct;

typedef struct
{
	u16 RecordLength;
	float MinValue;
	float MaxValue;
	float *DataSet;
	u16   LastPos;
	u16   NewPos;
	bool  ChangePos;
	bool  DrawPos;
}SingleCurveConfigurationStruct;

enum { PosChanged = true, DontChangePos = false };

enum { DrawCoordinate = true, DontDrawCoordinate = false };

enum { LeftHalf = 1, RightHalf = 0 };

xQueueHandle UI_DialogueMsg;

/**
  * @brief  Draw a single curve

  * @param  A struct which contains:

			ChangePos:Did the position change or not

			DataSet:DataSet which is being drawn

			DrawPos:Draw the coordinate on the given position or not

			LastPos(given if coordinate drawing is enabled):
Last-drawn postion,used to remove the last coordinate

			NewPos(given if coordinate drawing is enabled):
New position of the coordinate which is being drawn

			MaxValue:Max value of the dataSet,used to determine the
position of the points of each data

			MinValue:Min value of the dataSet,used to determine the
position of the points of each data

			RecordLength:Length of the dataSet

	  @retval None
  */
void DrawSingleCurve(SingleCurveConfigurationStruct curveConfig)
{
	u16 i;
	float x1, x2;
	u8 y1, y2;

	/*If position is changed and coordinate drawing is enabled*/
	if (curveConfig.ChangePos == PosChanged&&curveConfig.DrawPos == DrawCoordinate)
	{
		/*Calculate the last-drawn coordinate*/
		y1 = 3 + (u8)((float)55 *
			((float)(curveConfig.MaxValue - curveConfig.DataSet[curveConfig.LastPos]) /
			(float)(curveConfig.MaxValue - curveConfig.MinValue)));

		x1 = ((float)(curveConfig.LastPos) / (float)(curveConfig.RecordLength - 1)) * 127;

		/*Remove the last-drawn coordinate*/
		OLED_DrawHorizonalLine(y1, 0, 127, UNDRAW);
		OLED_DrawVerticalLine(x1, 0, 63, UNDRAW);

		/*Especially,if the last-drawn coordinate is located at the end of the dataSet,
			remove the coordinate in x=127*/
		if (curveConfig.LastPos == curveConfig.RecordLength - 1)
		{
			x1 = 127;

			y1 = 3 + (u8)((float)55 *
				((float)(curveConfig.MaxValue - curveConfig.DataSet[curveConfig.RecordLength - 1]) /
				(float)(curveConfig.MaxValue - curveConfig.MinValue)));

			OLED_DrawHorizonalLine(y1, 0, 127, UNDRAW);
			OLED_DrawVerticalLine(x1, 0, 63, UNDRAW);
		}
	}
	/*Reset x1*/
	x1 = 0;
	/*Draw new curve and selected coordinate(if enabled)*/
	for (i = 0; i < curveConfig.RecordLength - 1; i++)
	{
		/*Draw line between dataSet[i] and dataSet[i+1]*/
		y1 = 3 + (u8)
			((float)55 * ((float)(curveConfig.MaxValue - curveConfig.DataSet[i]) /
			(float)(curveConfig.MaxValue - curveConfig.MinValue)));

		y2 = 3 + (u8)
			((float)55 * ((float)(curveConfig.MaxValue - curveConfig.DataSet[i + 1]) /
			(float)(curveConfig.MaxValue - curveConfig.MinValue)));

		x2 = ((float)(i + 1) / (float)(curveConfig.RecordLength - 1)) * 127;

		OLED_DrawAnyLine((u8)x1, y1, (u8)x2, y2, 1);

		/*Draw coordinate if this position is just the postion selected*/
		if ((curveConfig.NewPos == i) && (curveConfig.DrawPos == DrawCoordinate))
		{
			OLED_DrawHorizonalLine(y1, 0, 127, DRAW);
			OLED_DrawVerticalLine(x1, 0, 63, DRAW);
		}
		/*Keep forward*/
		x1 = ((float)(i + 1) / (float)(curveConfig.RecordLength - 1)) * 127;
	}
	/*Especially,if the coordinate is located at the end of the dataSet,
		draw the coordinate in x=127*/
	if ((curveConfig.NewPos == curveConfig.RecordLength - 1) &&
		(curveConfig.DrawPos == DrawCoordinate))
	{
		x1 = 127;

		y1 = 3 + (u8)
			((float)55 *
			((float)(curveConfig.MaxValue - curveConfig.DataSet[curveConfig.RecordLength - 1]) /
				(float)(curveConfig.MaxValue - curveConfig.MinValue)));
		OLED_DrawHorizonalLine(y1, 0, 127, DRAW);
		OLED_DrawVerticalLine(x1, 0, 63, DRAW);
	}
}

/**
  * @brief  Draw a full dialgram with two curves

  * @param  A struct

			See DrawSingleCurve() for details

	  @retval None
  */
void DrawDialgram(DialgramConfigurationStruct dialgramConfig)
{

	SingleCurveConfigurationStruct curveConfig;
	curveConfig.RecordLength = dialgramConfig.RecordLength;
	curveConfig.MinValue = dialgramConfig.MinValues[0];
	curveConfig.MaxValue = dialgramConfig.MaxValues[0];
	curveConfig.DataSet = dialgramConfig.DataSets[0];
	curveConfig.LastPos = dialgramConfig.LastPos;
	curveConfig.NewPos = dialgramConfig.NewPos;
	curveConfig.ChangePos = dialgramConfig.ChangePos;
	curveConfig.DrawPos = dialgramConfig.DrawPos;

	DrawSingleCurve(curveConfig);

	curveConfig.RecordLength = dialgramConfig.RecordLength;
	curveConfig.MinValue = dialgramConfig.MinValues[1];
	curveConfig.MaxValue = dialgramConfig.MaxValues[1];
	curveConfig.DataSet = dialgramConfig.DataSets[1];
	curveConfig.LastPos = dialgramConfig.LastPos;
	curveConfig.NewPos = dialgramConfig.NewPos;
	curveConfig.ChangePos = dialgramConfig.ChangePos;
	curveConfig.DrawPos = dialgramConfig.DrawPos;

	DrawSingleCurve(curveConfig);
}

/**
  * @brief  Draw grids and min/max values for the dialgram

  * @param  sprintfCommandStrings:Sprintf command strings which
format the max/min values

			maxValues[2]: Maximum value for dateSet0 and dataSet1

			drawMinAndMax:Draw or don't draw min/max values

			gridsDensity:Given in HighDensity and LowDensity

	  @retval None
  */
void DrawDashedGridsAndMinMax(const char *sprintfCommandStrings[2], float maxValues[2], float minValues[2], bool drawMinAndMax, u8 gridsDensity)
{
	u8 i;
	u8 tempLength;
	char tempString[10];
	/*Draw grids*/
	for (i = 0; i < 6; i++)
	{
		DrawHorizonalDashedGrid(3 + i * 11, DRAW, gridsDensity);
	}
	if (gridsDensity == HighDensity)
		for (i = 0; i < 11; i++)
		{
			DrawVerticalDashedGrid(3 + i * 12, DRAW, gridsDensity);
		}

	/*Return if draw max/min isn't needed*/
	if (drawMinAndMax == false) return;
	/*Show max/min values*/
	sprintf(tempString, sprintfCommandStrings[0], maxValues[0]);
	OLED_ShowAnyString(0, 0, tempString, NotOnSelect, 8);
	sprintf(tempString, sprintfCommandStrings[0], minValues[0]);
	OLED_ShowAnyString(0, 56, tempString, NotOnSelect, 8);
	sprintf(tempString, sprintfCommandStrings[1], maxValues[1]);
	tempLength = GetStringGraphicalLength(tempString);
	OLED_ShowAnyString(127 - 6 * tempLength, 0, tempString, NotOnSelect, 8);
	sprintf(tempString, sprintfCommandStrings[1], minValues[1]);
	tempLength = GetStringGraphicalLength(tempString);
	OLED_ShowAnyString(127 - 6 * tempLength, 56, tempString, NotOnSelect, 8);
}

/**
  * @brief  Show the data currently-selected

  * @param  sprintfCommandStrings[2]:
Sprintf command strings used to format the data

			currentPos:Position of the being-drawn data in dataSet

			leftOrRight:This param determines the currently-selected
data is shown on the left side of the screen or right

			dataSetPointers:Pointers for dataSets which contain
the being-drawn data

			dataNumPointer:Pointers of the serial No.s for each
data in dataSets,which contains the serial No. of the being-drawn data

			dataNumSprintfCommandString[10]:
Sprintf command strings used to format the serial No. of being-drawn data

			claer:This param determines if the function "DRAW" the given
data or just "CLEAR" the area where data is drawn

	  @retval None
  */
void DrawCurrentPosData(const char *sprintfCommandStrings[2], u16 currentPos,
	u8 leftOrRight, float *dataSetPointers[2],const char *dataNumSprintfCommandString, u8 item1AutoNumStart,
	u8 item1AutoNumStep, bool clear)
{
	u16 i = 0;
	u8 stringLengths[3];
	u16 maxLength;
	char strings[3][10];
	u8 xPos;
	/*Sprintf the data strings and the serial No. string*/
	for (i = 0; i < 2; i++)
	{
		sprintf(strings[i], sprintfCommandStrings[i], dataSetPointers[i][currentPos]);
	}
	sprintf(strings[2], dataNumSprintfCommandString, item1AutoNumStart + item1AutoNumStep*currentPos);
	/*Get the lengths of the those strings*/
	for (i = 0; i < 3; i++)
	{
		stringLengths[i] = GetStringGraphicalLength(strings[i]);
	}
	/*Compare the strings,get the max length among strings*/
	maxLength = stringLengths[0];
	for (i = 1; i < 3; i++)
	{
		if (stringLengths[i] > maxLength)
			maxLength = stringLengths[i];
	}
	/*Set drawing position according to given param "leftOrRight"*/
	if (leftOrRight == LeftHalf)
	{
		xPos = 0;
	}
	else
	{
		xPos = 127 - maxLength * 6;
	}
	/*If the command is clear the respective area ,clear and return*/
	if (clear == true)
	{
		OLED_FillRect(xPos, 0, xPos + maxLength * 6, 35, UNDRAW);
		return;
	}
	/*Show serial No. string and data strings*/
	OLED_ShowAnyString(xPos, 0, strings[2], NotOnSelect, 12);
	for (i = 0; i < 2; i++)
	{
		OLED_ShowAnyString(xPos, (i + 1) * 12, strings[i], NotOnSelect, 12);
	}
}

void Dialgram_Broswer(void *pvParameters)
{
	int currentPos = 0;
	u16 lastPos = 0;
	u8 screenSide;
	u8 i;
	bool modeChanged = true;
	bool posChanged = true;
	bool posChangedDrawt = false;
	bool sideChanged = true;
	bool posDataUncleared = false;
	Key_Message_Struct keyMessage;
	DialgramConfigurationStruct dialgramConfig;
	Dialgram_Param_Struct* dialgram_Params = (Dialgram_Param_Struct*)pvParameters;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	/*Draw grids and min/max values(making defualt in non-browse mode)*/
	DrawDashedGridsAndMinMax(dialgram_Params->MaxAndMinSprintfCommandStrings,
		dialgram_Params->MaxValues, dialgram_Params->MinValues, false, HighDensity);
	/*Set the param stucture for dialgram drawing*/
	dialgramConfig.DataSets[0] = dialgram_Params->DataPointers[0];
	dialgramConfig.DataSets[1] = dialgram_Params->DataPointers[1];
	dialgramConfig.RecordLength = dialgram_Params->RecordLength;
	dialgramConfig.MaxValues[0] = dialgram_Params->MaxValues[0];
	dialgramConfig.MaxValues[1] = dialgram_Params->MaxValues[1];
	dialgramConfig.MinValues[0] = dialgram_Params->MinValues[0];
	dialgramConfig.MinValues[1] = dialgram_Params->MinValues[1];
	dialgramConfig.ChangePos = DontChangePos;
	/*Drawing coordinate is disabled(making defualt in non-browse mode)*/
	dialgramConfig.DrawPos = DontDrawCoordinate;
	dialgramConfig.LastPos = lastPos;
	dialgramConfig.NewPos = currentPos;
	/*Draw the dialgram*/
	DrawDialgram(dialgramConfig);
	OLED_Refresh_Gram();
	// while(1) OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
	/*Set the rate of longPress-FastMoving to 20 sets per second*/
	SetKeyBeatRate(20);
	ClearKeyEvent(keyMessage);
	for (;;)
	{
		/*If it is not in browse mode(default),draw the full dialgram and
		clear the flag to stand-by*/
		for (;;)
		{
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			OLED_Clear();
			modeChanged = false;
			DrawDashedGridsAndMinMax(dialgram_Params->MaxAndMinSprintfCommandStrings,
				dialgram_Params->MaxValues, dialgram_Params->MinValues, true, HighDensity);
			dialgramConfig.ChangePos = DontChangePos;
			dialgramConfig.DrawPos = DontDrawCoordinate;
			dialgramConfig.LastPos = lastPos;
			dialgramConfig.NewPos = currentPos;
			DrawDialgram(dialgramConfig);
			OLED_Refresh_Gram();
			xSemaphoreGive(OLEDRelatedMutex);
			xQueueReceive(Key_Message, &keyMessage, portMAX_DELAY);
			if (keyMessage.KeyEvent == MidLong)
			{
				modeChanged = true; break;
			}
			else if (keyMessage.KeyEvent == MidClick)
			{
				i = 0; goto Done;
			}
			else if (keyMessage.KeyEvent == MidDouble)
			{
				i = 1; goto Done;
			}
		}
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		OLED_Clear();
		for (;;)
		{
			/*If modeChanged(usually switched from Non-browse mode) or the side(on screen) of
			the data changed,clear the OLED to make a full reDraw*/
			if (modeChanged == true || sideChanged == true)
			{
				OLED_Clear();
				OLED_Refresh_Gram();
			}
			/*If anyone of position,side,mode changed(Or posChangedDrawt == false,which
			means the dialgram may not fully-reDrawn) ,reDraw the dialgram,clear the flags
			after operation*/
			if (posChanged == true || posChangedDrawt == false || sideChanged == true || modeChanged == true)
			{
			ReDrawDialgram:
				dialgramConfig.ChangePos = PosChanged;
				dialgramConfig.DrawPos = DrawCoordinate;
				dialgramConfig.LastPos = lastPos;
				dialgramConfig.NewPos = currentPos;
				DrawDialgram(dialgramConfig);
				posChanged = false;
				posChangedDrawt = true;
				sideChanged = false;
				modeChanged = false;
				DrawDashedGridsAndMinMax(dialgram_Params->MaxAndMinSprintfCommandStrings,
					dialgram_Params->MaxValues, dialgram_Params->MinValues, false, LowDensity);
				OLED_Refresh_Gram();
			}
			/*If the selected point is in the first-half of the dataSets,show
			the detailed data of selected point on the right-half of the screen*/
			if (currentPos < (dialgram_Params->RecordLength / 2))
			{
				if (screenSide == LeftHalf)
					sideChanged = true;
				screenSide = RightHalf;
			}
			/*If the selected point is in the second-half of the dataSets,show
			the detailed data of selected point on the left-half of the screen*/
			else
			{
				if (screenSide == RightHalf)
					sideChanged = true;
				screenSide = LeftHalf;
			}
			//xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY);
			/*ReDraw the detailed data of selected point as soon as key released*/
			if (LEFT_KEY == KEY_OFF&&RIGHT_KEY == KEY_OFF)
			{
				DrawCurrentPosData(dialgram_Params->DataSprintfCommandStrings, currentPos,
					screenSide, dialgram_Params->DataPointers, dialgram_Params->DataNumSprintfCommandString,
					dialgram_Params->Item1AutoNumStart, dialgram_Params->Item1AutoNumStep, false);
				OLED_Refresh_Gram();
				posDataUncleared = true;
			}
			/*If any key is pressed,clear the detailed-data-area immediately,then make a full-reDraw*/
			if (LEFT_KEY == KEY_ON || RIGHT_KEY == KEY_ON)
			{
				if (posDataUncleared == true)
				{
					DrawCurrentPosData(dialgram_Params->DataSprintfCommandStrings, currentPos,
						screenSide, dialgram_Params->DataPointers, dialgram_Params->DataNumSprintfCommandString,
						dialgram_Params->Item1AutoNumStart, dialgram_Params->Item1AutoNumStep, true);
					OLED_Refresh_Gram();
					posDataUncleared = false;
					goto ReDrawDialgram;
				}
			}
			if (xQueueReceive(Key_Message, &keyMessage, 0) == pdPASS)
			{
				if (keyMessage.KeyEvent == LeftClick || keyMessage.AdvancedKeyEvent == LeftContinous)
				{
					lastPos = currentPos;
					currentPos--;
					posChanged = true;
					if (currentPos < 0)
					{
						posChanged = false;
						currentPos = 0;
					}
				}
				else if (keyMessage.KeyEvent == RightClick || keyMessage.AdvancedKeyEvent == RightContinous)
				{
					lastPos = currentPos;
					currentPos++;
					posChanged = true;
					if (currentPos > dialgram_Params->RecordLength - 1)
					{
						posChanged = false;
						currentPos = dialgram_Params->RecordLength - 1;
					}
				}
				else if (keyMessage.KeyEvent == MidLong)
				{
					break;
				}
				else if (keyMessage.KeyEvent == MidClick)
				{
					i = 0; goto Done;
				}
				else if (keyMessage.KeyEvent == MidDouble)
				{
					i = 1; goto Done;
				}
			}
			vTaskDelay(10 / portTICK_RATE_MS);
		}
		xSemaphoreGive(OLEDRelatedMutex);
	}
Done:
	xSemaphoreGive(OLEDRelatedMutex);
	xQueueSend(UI_DialogueMsg, &i, portMAX_DELAY);
	vTaskDelay(100);
	vTaskDelete(NULL);
}

/**
  * @brief  See UI_Dialgram_Broswer for details

  * @param  Dialgram param struct pointer

  */
void UI_Dialgram_Init(Dialgram_Param_Struct * dialgramParams)
{
	portBASE_TYPE success;
	UI_DialogueMsg = xQueueCreate(1, sizeof(int));
	success=xTaskCreate(Dialgram_Broswer, "UI_Dialgram_Broswer",
		384, dialgramParams, UI_DIALGRAM_BROSWER_PRIORITY, NULL);
	if(success!=pdPASS) ApplicationNewFailed("UI_Dialgram_Browswer");
}
