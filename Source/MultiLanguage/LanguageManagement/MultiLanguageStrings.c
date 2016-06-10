#include "MultiLanguageStrings.h"
#include "SSD1306.h"

#pragma diag_suppress 870

const char SystemInit_EN[] = "System Init...";
const char SystemInit_TW[] = "系統初始化...";
const char SystemInit_JC[] = "系统初始化...";
const char SystemInit_JP[] = "システムの初期化...";
const char * SystemInit_Str[] = { SystemInit_EN,SystemInit_TW,SystemInit_JC,SystemInit_JP };

const char Capacity_EN[] = "Capacity:%dMB";
const char Capacity_TW[] = "容量:%dMB";
const char Capacity_JC[] = "容量:%dMB";
const char Capacity_JP[] = "容量:%dMB";
const char * Capacity_Str[] = { Capacity_EN,Capacity_TW,Capacity_JC,Capacity_JP };

const char NoSD_EN[] = "No SDCard";
const char NoSD_TW[] = "沒有記憶體";
const char NoSD_JC[] = "没有SD卡";
const char NoSD_JP[] = "いいえTFカード";
const char * NoSD_Str[] = { NoSD_EN,NoSD_TW,NoSD_JP };

const char WaitingForEBD_EN[] = "Waiting for EBD...";
const char WaitingForEBD_TW[] = "等待設備...";
const char WaitingForEBD_JC[] = "等待设备...";
const char WaitingForEBD_JP[] = "接続します...";
const char * WaitingForEBD_Str[] = { WaitingForEBD_EN,WaitingForEBD_TW,WaitingForEBD_JC,WaitingForEBD_JP };

const char EBDConnected_EN[] = "EBD Connected";
const char EBDConnected_TW[] = "設備已連接";
const char EBDConnected_JC[] = "设备已连接";
const char EBDConnected_JP[] = "接続すでに";
const char * EBDConnected_Str[] = { EBDConnected_EN,EBDConnected_TW,EBDConnected_JC,EBDConnected_JP };

const char Confirmation_EN[] = "Confirmation";
const char Confirmation_TW[] = "確認";
const char Confirmation_JC[] = "确认";
const char Confirmation_JP[] = "確認します";
const char * Confirmation_Str[] = { Confirmation_EN,Confirmation_TW,Confirmation_JC,Confirmation_JP };

const char ConfirmCancel_EN[] = "Cancel%Comfirm";
const char ConfirmCancel_TW[] = " 取消 % 確認 ";
const char ConfirmCancel_JC[] = " 取消 % 确认 " ;
const char ConfirmCancel_JP[] = " キャンセル % はい ";
const char * ConfirmCancel_Str[] = { ConfirmCancel_EN,ConfirmCancel_TW,ConfirmCancel_JC,ConfirmCancel_JP };

const char RecordConfirm_EN[] = "Start Record?";
const char RecordConfirm_TW[] = "開始記錄曲線? ";
const char RecordConfirm_JC[] = "开始记录曲线?" ;
const char RecordConfirm_JP[] = "曲線レコード?";
const char * RecordConfirm_Str[] = { RecordConfirm_EN,RecordConfirm_TW,RecordConfirm_JC,RecordConfirm_JP };

const char QCMTKConfirm_EN[] = "QC/MTK Trigger?";
const char QCMTKConfirm_TW[] = "QC/MTK觸發?";
const char QCMTKConfirm_JC[] = "QC/MTK触发?" ;
const char QCMTKConfirm_JP[] = "QC/MTKトリガー?";
const char * QCMTKConfirm_Str[] = { QCMTKConfirm_EN,QCMTKConfirm_TW,QCMTKConfirm_JC,QCMTKConfirm_JP };

const char Saved_EN[] = "Saved";
const char Saved_TW[] = "已保存";
const char Saved_JC[] = "已保存";
const char Saved_JP[] = "保存";
const char * Saved_Str[] = { Saved_EN,Saved_TW,Saved_JC,Saved_JP };

const char Settings_EN[]="Unmount Disk%Clock Settings%Buzzer Settings%Language%System Scan%System Info";
const char Settings_TW[]="彈出SD卡%時間設置%警報設置%Language%系統掃描%系統資訊";
const char Settings_JC[] = "安全弹出存储卡%时间设置%蜂鸣器设置%Language%系统扫描%系统信息" ;
const char Settings_JP[] ="イジェクトメモリ%時刻の設定%警告の設定%Language%システムスキャン%システム情報";
const char * Settings_Str[] = { Settings_EN,Settings_TW,Settings_JC,Settings_JP };

const char SetYear_EN[]="Set year";
const char SetYear_TW[]="設置年份";
const char SetYear_JC[] = "设置年份" ;
const char SetYear_JP[] ="年を設定";
const char * SetYear_Str[] = { SetYear_EN,SetYear_TW,SetYear_JC,SetYear_JP };

const char SetYearUnit_EN[] ="Year";
const char SetYearUnit_TW[] ="年";
const char SetYearUnit_JC[] ="年" ;
const char SetYearUnit_JP[] ="年";
const char * SetYearUnit_Str[] = { SetYearUnit_EN,SetYearUnit_TW,SetYearUnit_JC,SetYearUnit_JP };

const char SetMonth_EN[]="Set month";
const char SetMonth_TW[]="設置月份";
const char SetMonth_JC[] = "设置月份" ;
const char SetMonth_JP[] ="月の設定";
const char * SetMonth_Str[] = { SetMonth_EN,SetMonth_TW,SetMonth_JC,SetMonth_JP };

const char SetMonthUnit_EN[] ="month";
const char SetMonthUnit_TW[] ="月";
const char SetMonthUnit_JC[] ="月" ;
const char SetMonthUnit_JP[] ="月";
const char * SetMonthUnit_Str[] = { SetMonthUnit_EN,SetMonthUnit_TW,SetMonthUnit_JC,SetMonthUnit_JP };
	
const OLED_PositionStruct ComfirmationPositions[LanguageNum][2] =
{
{20,45,76,45},
{36,45,84,45},
{36,45,84,45},
{8,45,84,45}
};


