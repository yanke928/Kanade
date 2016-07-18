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
u32 GetTestParam(const char askString[], u32 min, u32 max, u32 defaultValue, u32 step, char unitString[], u8 fastSpeed);

#endif /* __STEPUPTEST_H */
