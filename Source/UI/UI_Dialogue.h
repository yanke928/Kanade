#ifndef __UI_DIALOGUE_H
#define	__UI_DIALOGUE_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "SSD1306.h"

void ShowDialogue(char titleString[],char subString0[],char subString1[]);

#endif /* __UI_DIALOGUE_H */
