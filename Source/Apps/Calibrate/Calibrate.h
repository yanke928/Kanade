#ifndef __CALIBRATE_H
#define	__CALIBRATE_H

#include "stm32f10x.h"

#define CALIBRATION_DATA_ADDRESS 0x0803e800

typedef struct
{
	float Refk;
	float Refb;
  double VoltageCoeficient;
  double CurrentCoeficient;
}Calibration_t;

extern Calibration_t* const Calibration_Data;

extern Calibration_t Calibration_Backup;

extern const Calibration_t Calibration_Default;

void Digital_Load_Calibrate(void);

void CheckCalibrationData(void);

void Voltage_Calibrate(void);

void CalibrateSelect(void);

#endif /* __CALIBRATE_H */
