#ifndef __STEPUPTEST_H
#define	__STEPUPTEST_H

#include "stm32f10x.h"
#include "stdbool.h"

typedef struct {
	u16 StartCurrent;
	u16 StopCurrent;
	u16 Step;
	u16 TimeInterval;
	u16 ProtectVolt;
}StepUpTestParamsStruct;

typedef struct {
	u16 CurrentTime;
  u8  TestOverFlag;
}StepUpTestStateStruct;

//see .c for details
void StepUpTest_Init(StepUpTestParamsStruct* test_Params);

//see .c for details
void StepUpTest_UI_Init(u16* testTime);

//see .c for details
void RunAStepUpTest(void);

//see .c for details
u16 GetTestParam(const char askString[], u16 min, u16 max, u16 defaultValue, u16 step, char unitString[], u8 fastSpeed);

#endif /* __STEPUPTEST_H */
