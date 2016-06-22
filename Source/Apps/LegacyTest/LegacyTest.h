#ifndef __LEGACYTEST_H
#define	__LEGACYTEST_H

#include "stm32f10x.h"
#include "stdbool.h"

#define LEGACY_TEST 2

typedef struct {
	u16 Current;
	u16 ProtectVolt;
}Legacy_Test_Param_Struct;

void RunLegacyTest(u8* status,Legacy_Test_Param_Struct* test_Params);

#endif /* __STEPUPTEST_H */
