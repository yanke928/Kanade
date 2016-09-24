#ifndef __MULTILANGUAGESTRINGS_H
#define	__MULTILANGUAGESTRINGS_H

#include "stm32f10x.h"
#include "SSD1306.h"

#define LanguageNum 4

/*Mount related*/
extern const char* const SettingsUnmountFailed_Str[LanguageNum];
extern const char* const SettingsUnmounted_Str[LanguageNum];
extern const char* const SettingsMounted_Str[LanguageNum];
extern const char* const SettingsMountFailed_Str[LanguageNum];
extern const char* const SDCardRemovedAccidentally_Str[LanguageNum];

/*Model settings related*/
extern const char* const SetModelHint_Str[LanguageNum];

/*Startup related*/
extern const char* const SystemInit_Str[LanguageNum];
extern const char* const Capacity_Str[LanguageNum];
extern const char* const NoSD_Str[LanguageNum];
extern const char* const WaitingForEBD_Str[LanguageNum];
extern const char* const EBDConnected_Str[LanguageNum];

/*Language Names*/
extern const char* const LanguageItem_Str[LanguageNum];

/*Time settings related*/
extern const char* const SetYear_Str[LanguageNum];
extern const char* const SetYearUnit_Str[LanguageNum];
extern const char* const SetMonth_Str[LanguageNum];
extern const char* const SetMonthUnit_Str[LanguageNum];
extern const char* const SetDay_Str[LanguageNum];
extern const char* const SetDayUnit_Str[LanguageNum];
extern const char* const SetHour_Str[LanguageNum];
extern const char* const SetHourUnit_Str[LanguageNum];
extern const char* const SetMin_Str[LanguageNum];
extern const char* const SetMinUnit_Str[LanguageNum];
extern const char* const SetSec_Str[LanguageNum];
extern const char* const SetSecUnit_Str[LanguageNum];
extern const char* const TimeSetting_Str[LanguageNum];
extern const char* const SetTimeHint_Str[LanguageNum];

/*Step up test related*/
extern const char* const StepUpTestRunning_Str[LanguageNum];
extern const char* const StepUpTestProtected_Str[LanguageNum];
extern const char* const StepUpTestDone_Str[LanguageNum];
extern const char* const StepUpTestDialgram_Str[LanguageNum];
extern const char* const StepUpTestList_Str[LanguageNum];
extern const char* const StepUpTestDialgramTime_Str[LanguageNum];
extern const char* const StepUpTestDialgramCurrent_Str[LanguageNum];
extern const char* const StepUpTestDialgramVoltage_Str[LanguageNum];
extern const char* const StepUpTestExitBroswer_Str[LanguageNum];
extern const char* const StepUpConfirm_Str[LanguageNum];
extern const char* const StartCurrentGet_Str[LanguageNum];
extern const char* const EndCurrentGet_Str[LanguageNum];
extern const char* const StepCurrentGet_Str[LanguageNum];
extern const char* const TimeIntervalGet_Str[LanguageNum];
extern const char* const ProtVoltageGet_Str[LanguageNum];
extern const char* const RunTime_Str[LanguageNum];
extern const char* const CurrentVoltage_Str[LanguageNum];
extern const char* const CurrentCurrent_Str[LanguageNum];

/*Legacy test related*/
extern const char* const SelectLeagcyTestMode_Str[LanguageNum];
extern const char* const SelectLeagcyTestModeSubString_Str[LanguageNum];
extern const char* const LegacyTestSetPower_Str[LanguageNum];
extern const char* const Summary_Str[LanguageNum];
extern const char* const SummaryCapacity_Str[LanguageNum];
extern const char* const SummaryWork_Str[LanguageNum];
extern const char* const SummaryTime_Str[LanguageNum];
extern const char* const SummaryPlatVolt_Str[LanguageNum];
extern const char* const LegacyTestVoltLow_Str[LanguageNum];
extern const char* const LegacyTestStarted_Str[LanguageNum];
extern const char* const RecordIsRunningHint1_Str[LanguageNum];
extern const char* const RecordIsRunningHint2_Str[LanguageNum];
extern const char* const RecordStopConfirm_Str[LanguageNum];
extern const char* const RecordStarted_Str[LanguageNum];
extern const char* const PleaseWait_Str[LanguageNum];
extern const char* const SaveFailed_Str[LanguageNum];
extern const char* const LegacyTestConfirm_Str[LanguageNum];
extern const char* const LegacyTestSetCurrent_Str[LanguageNum];
extern const char* const NoDiskConfirm_Str[LanguageNum];
extern const char* const FileCreated_Str[LanguageNum];
extern const char* const FileCreateFailed_Str[LanguageNum];
extern const char* const RecordConfirm_Str[LanguageNum];

/*Trigger related*/
extern const char* const QCMTKConfirm_Str[LanguageNum] ;
extern const char* const ReleaseFastCharge_Str[LanguageNum];

/*MassStorage related*/
extern const char* const USBMassStorage_Str[LanguageNum];
extern const char* const MountUSBMassStorageConfirm_Str[LanguageNum];
extern const char* const UnMountUSBMassStorage_Str[LanguageNum];
extern const char* const USBMassStorageStatus_Str[LanguageNum];
extern const char* const USBMassStorageStatusReading_Str[LanguageNum];
extern const char* const USBMassStorageStatusWriting_Str[LanguageNum];
extern const char* const USBMassStorageStatusIdling_Str[LanguageNum];

/*Overheat control settings related*/
extern const char* const SetInternalTemp_Max_Str[LanguageNum];
extern const char* const SetExternalTemp_Max_Str[LanguageNum];
extern const char* const SetTempProtectResumeGap_Str[LanguageNum];

/*Overheat related*/
extern const char* const ExternalOverHeat_Str[LanguageNum];
extern const char* const SystemOverHeat_Str[LanguageNum];
extern const char* const TestPaused_Str[LanguageNum];
extern const char* const Temperature_Unit_Str[LanguageNum];
extern const char* const TestPaused_Str[LanguageNum];
extern const char* const Temperature_Unit_Str[LanguageNum];

/*Settings items*/
extern const char* const SettingsItemMountDisk_Str[LanguageNum];
extern const char* const SettingsItemUnmountDisk_Str[LanguageNum];
extern const char* const SettingsItemClockSettings_Str[LanguageNum];
extern const char* const SettingsItemOverHeatControl_Str[LanguageNum];
extern const char* const SettingsItemLanguage_Str[LanguageNum];
extern const char* const SettingsItemModel_Str[LanguageNum];
extern const char* const SettingsItemSystemInfo_Str[LanguageNum];
extern const char* const SettingsItemFormatInternal_Str[LanguageNum];
extern const char* const SettingsItemFirmwareUpdate_Str[LanguageNum];
extern const char* const SettingsItemSystemScan_Str[LanguageNum];
extern const char* const SettingsItemCalibration_Str[LanguageNum];
extern const char* const SettingsItemIdleClockSettings_Str[LanguageNum];

/*Legacy test mode items*/
extern const char* const SelectLeagcyTestModeButtonCC_Str[LanguageNum];
extern const char* const SelectLeagcyTestModeButtonCP_Str[LanguageNum];


/*Disk format related*/
extern const char* const SettingsItemFormatSD_Str[LanguageNum];
extern const char* const SettingsItemFormatSD_Str[LanguageNum];
extern const char* const SettingsItemNoDisk_Str[LanguageNum];
extern const char* const SettingsItemFormatDisks_Str[LanguageNum];
extern const char* const FormatInternalConfirm_Str[LanguageNum];
extern const char* const FormatSDConfirm_Str[LanguageNum];
extern const char* const Formatting_Str[LanguageNum];
extern const char* const FormatSuccess_Str[LanguageNum];

/*Public*/
extern const char* const ConfirmationConfirm_Str[LanguageNum];
extern const char* const ConfirmationCancel_Str[LanguageNum];
extern const char* const Confirmation_Str[LanguageNum];
extern const char* const Hint_Str[LanguageNum];
extern const char* const Saved_Str[LanguageNum];
extern const char* const Enabled_Str[LanguageNum];
extern const char* const Disabled_Str[LanguageNum] ;

/*Calibration*/
extern const char* const CalibrationItemAmpfilierSelfCalibration_Str[LanguageNum];
extern const char* const AmpfilierSelfCalibrationSuccess_Str[LanguageNum];
extern const char* const AmpfilierSelfCalibrationFailed_Str[LanguageNum] ;
extern const char* const AmpfilierSelfCalibrationRunning_Str[LanguageNum] ;
extern const char* const CalibrationItemVoltageCalibration_Str[LanguageNum];
extern const char* const VoltageCalibrationSuccess_Str[LanguageNum];
extern const char* const CalibrationItemCurrentCalibration_Str[LanguageNum];
extern const char* const CalibrationAbort_Str[LanguageNum];
extern const char* const AbortConfirmation_Str[LanguageNum];

/*Firmware update related*/
extern const char* const FirmwareUpdateConfirm_Str[LanguageNum];

extern const char* const DontSave_Str[LanguageNum];

/*Idle Clock Settings related*/
extern const char* const IdleClockSettingsDisableIdleClock_Str[LanguageNum];
extern const char* const IdleClockSettingsEnableIdleClock_Str[LanguageNum];
extern const char* const IdleClockSettingsIdleClockTime_Str[LanguageNum];
extern const char* const IdleClockSettingsPleaseEnable_Str_Str[LanguageNum];

/*Alarm Settings related*/
extern const char* const EnableBuzzerAlarm[LanguageNum];
extern const char* const DisableBuzzerAlarm[LanguageNum];
extern const char* const EnableLEDAlarm[LanguageNum];
extern const char* const DisableLEDAlarm[LanguageNum];

/*MTK-PE Trigger related*/
extern const char* const IvePlugged_Str[LanguageNum];
extern const char* const PlugInLoad_Str[LanguageNum];
extern const OLED_PositionStruct  IvePluggedInPositions[LanguageNum][1];

/*Over Power Control*/
extern const char* const Continue_Str[LanguageNum];
extern const char* const Stop_Str[LanguageNum];
extern const char* const OverPowerLimit_Str[LanguageNum];
extern const OLED_PositionStruct  ContinueAndStopPositions[LanguageNum][2];
extern const OLED_PositionStruct  StopPositions[LanguageNum][2];

/*Confirmation positions*/
extern const OLED_PositionStruct  ComfirmationPositions[LanguageNum][2];
extern const OLED_PositionStruct  SelectLegacyTestModePositions[LanguageNum][2];

#endif /* __MULTILANGUAGESTRINGS_H */













