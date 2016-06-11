#ifndef __UI_DIALOGUE_H
#define	__UI_DIALOGUE_H

#include "stm32f10x.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//see .c for details
void ShowDialogue(char titleString[],char subString0[],char subString1[]);

//see .c for details
void ShowSmallDialogue(char string[],u16 time,bool occupyThread);

#endif /* __UI_DIALOGUE_H */
