#ifndef __STEPUPTEST_H
#define	__STEPUPTEST_H

#include "stm32f10x.h"

typedef struct {
	u16 StartCurrent;
	u16 StopCurrent;
	u16 Step;
	u16 TimeInterval;
	u16 ProtectVolt;
}StepUpTestParamsStruct;

//see .c for details
void StepUpTest_Init(StepUpTestParamsStruct* test_Params);

//see .c for details
void StepUpTest_UI_Init(void);

//see .c for details
void RunAStepUpTest(void);

#endif /* __STEPUPTEST_H */
