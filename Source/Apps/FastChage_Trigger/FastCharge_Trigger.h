#ifndef __FASTCHARGE_TRIGGER_H
#define	__FASTCHARGE_TRIGGER_H

#include "stm32f10x.h"

//see .c for details
void FastChargeTriggerUI(bool* fastchargeTriggeredFlag);

//see .c for details
void ReleaseFastCharge(bool* fastchargeTriggeredFlag);

#endif /* __FASTCHARGE_TRIGGER_H */
