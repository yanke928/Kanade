//File Name     Digital Load.c
//Description : Digital Load API

#include "Digital_Load.h"
#include "PWM_Ref.h"

void Set_Constant_Current(float curt)
{
 float voltageREF=(float)SENSOR_RESISTANCE*(float)SENSOR_REFERENCE_GAIN*curt/1000;
 Set_RefVoltageTo(voltageREF);
}
