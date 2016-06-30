#ifndef __UI_LISTVIEW_H
#define	__UI_LISTVIEW_H

#include "stm32f10x.h"

#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct
{
	char *ItemNames[4];
	char *sprintfCommandStrings[4];
	float *DataPointers[4];
	bool Item1AutoNum;
	u8 ItemPositions[5];
	u8 Item1AutoNumStart;
	u8 Item1AutoNumStep;
	u8 FastSpeed;
	u16 ItemNum;
	u16 ListLength;
	u16 DefaultPos;
}ListView_Param_Struct;

extern xQueueHandle UI_ListViewMsg; 

void UI_ListView_Init(ListView_Param_Struct * listViewParams);

void UI_ListView_DeInit(void);

#endif /* __UI_LISTVIEW_H */
