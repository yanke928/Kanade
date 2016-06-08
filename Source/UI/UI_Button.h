#ifndef __UI_BUTTON_H
#define	__UI_BUTTON_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "SSD1306.h"

typedef struct 
{
 char *ButtonString;
 u8 ButtonNum;
 u8 DefaultValue;
 OLED_PositionStruct * Positions;
}UI_Button_Param_Struct;

extern xQueueHandle UI_ButtonMsg;

//see .c for details
void UI_Button_Init(UI_Button_Param_Struct * buttonParams);

#endif /* __UI_BUTTON_H */
