//File Name   £ºUI_Utilities.c
//Description : UI Utilities

#include <string.h>
#include <stdio.h>

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
  * @brief  Get length of given string

  * @param  string[]:Given string

	  @retval Length of given string
  */
u8 GetStringLength(char string[])
{
	u8 length;
	for (length = 0; string[length] != 0; length++);
	return(length);
}

