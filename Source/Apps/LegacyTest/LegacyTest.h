#ifndef __LEGACYTEST_H
#define	__LEGACYTEST_H

#include "stm32f10x.h"
#include "stdbool.h"

#include "sdcard.h"
#include "sdcardff.h"
#include "EBProtocol.h"

#define LEGACY_TEST 2

enum LegacyTestMode{ConstantCurrent=0,ConstantPower=1};

typedef struct {
	u16 Current;
	u16 ProtectVolt;
	u16 Power;
	u8 TestMode;
}Legacy_Test_Param_Struct;

void RunLegacyTest(u8* status,Legacy_Test_Param_Struct* test_Params);

void StartRecord(u8* status);

void StopRecord(u8* status,u8 reason);

#endif /* __STEPUPTEST_H */
