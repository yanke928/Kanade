#ifndef __EXCEPTIONHANDLE_H
#define	__EXCEPTIONHANDLE_H

#include "stm32f10x.h"

void ApplicationNewFailed(char * appName);

void EBD_Exception_Handler_Init(void);

#endif /* __EXCEPTIONHANDLE_H */
