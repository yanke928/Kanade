#ifndef __EXCEPTIONHANDLE_H
#define	__EXCEPTIONHANDLE_H

#include "stm32f10x.h"

void ApplicationNewFailed(const char * appName);

void EBD_Exception_Handler_Init(void);

void ShowFault(char * string);

#endif /* __EXCEPTIONHANDLE_H */
