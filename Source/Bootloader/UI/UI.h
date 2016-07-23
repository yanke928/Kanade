#ifndef __UI_H
#define __UI_H

#include "stm32f10x.h"

void ShowDialogue(const char* titleString,const char* subString1,const char* subString2);

void ShowNoStorage(void);

void ShowNoFile(void);

void ShowHomePage(void);

void ShowSystemTempered(void);

void ShowCheckingFile(void);

void ShowCheckNotOK(void);

void ShowUpdating(void);

void ShowUpdateSuccess(void);

void UpdateProgressBar(u32 min,u32 max,u32 current);

void ProgressBar_Init(void);

void ShowUpdateFailed(void);

#endif
