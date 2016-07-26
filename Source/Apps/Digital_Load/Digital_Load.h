#ifndef __DIGITAL_LOAD_H__
#define __DIGITAL_LOAD_H__

#include "Digital_Load_Conf.h"
#include "Digital_Load.h"
#include <stdbool.h>

#define Emergency_Stop() PWMRef_SetToGND()

void Set_Constant_Current(float curt);

#endif /* __DIGITAL_LOAD_H__ */
