#ifndef __UI_CONFIRMATION_H
#define	__UI_CONFIRMATION_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

bool GetConfirmation(char subString0[],char subString1[]);

#endif /* __UI_CONFIRMATION_H */
