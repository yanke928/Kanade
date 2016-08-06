#ifndef __STEPUPTEST_H
#define	__STEPUPTEST_H

#include "stm32f10x.h"
#include "stdbool.h"

typedef struct {
	int StartCurrent;
	int StopCurrent;
	int Step;
	int TimeInterval;
	int ProtectVolt;
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
int GetTestParam(const char askString[], int min, int max, int defaultValue, int step, char unitString[], u8 fastSpeed);

#endif /* __STEPUPTEST_H */
