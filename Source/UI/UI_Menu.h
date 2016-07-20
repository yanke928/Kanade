#ifndef __UI_MENU_H
#define	__UI_MENU_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct
{
 const char** ItemStrings;
 u8 DefaultPos;
 u8 ItemNum;
 u8 FastSpeed;
}UI_Menu_Param_Struct;

extern xQueueHandle UI_MenuMsg;

//see .c for details
void UI_Menu_Init(UI_Menu_Param_Struct * menuParams);

//see .c for details
void UI_Menu_DeInit(void);

#endif /* __UI_MENU_H */
