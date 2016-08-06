#ifndef __UI_DIALOGUE_H
#define	__UI_DIALOGUE_H

#include "stm32f10x.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//see .c for details
void ShowDialogue(const char titleString[], const char subString0[], const char subString1[],bool titleCentered,bool stringCentered);

//see .c for details
void ShowSmallDialogue(const char string[],u16 time,bool occupyThread);

#endif /* __UI_DIALOGUE_H */
