#ifndef __UI_LISTVIEW_H
#define	__UI_LISTVIEW_H

#include "stm32f10x.h"

#include "stdbool.h"

typedef struct
{
	char *ItemNames[4];
	char *sprintfCommandStrings[4];
	bool Item1AutoNum;
	float *DataPointers[4];
	u8 ItemPositions[5];
	u8 Item1AutoNumStart;
	u8 Item1AutoNumStep;
	u8 FastSpeed;
	u16 ItemNum;
	u16 *DataAutoNumPointer;
	u16 ListLength;
	u16 DefaultPos;
}ListView_Param_Struct;

#endif /* __UI_LISTVIEW_H */
