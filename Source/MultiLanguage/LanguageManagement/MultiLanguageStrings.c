#include "MultiLanguageStrings.h"
#include "SSD1306.h"

#pragma diag_suppress 870

const char SystemInit_EN[] = "System Init...";
const char SystemInit_TW[] = "系統初始化...";
const char SystemInit_CN[] = "系统初始化...";
const char SystemInit_JP[] = "システムの初期化...";
const char * SystemInit_Str[] = { SystemInit_EN,SystemInit_TW,SystemInit_CN,SystemInit_JP };

const char Capacity_EN[] = "Capacity:%dMB";
const char Capacity_TW[] = "容量:%dMB";
const char Capacity_CN[] = "容量:%dMB";
const char Capacity_JP[] = "容量:%dMB";
const char * Capacity_Str[] = { Capacity_EN,Capacity_TW,Capacity_CN,Capacity_JP };

const char NoSD_EN[] = "No SDCard";
const char NoSD_TW[] = "沒有記憶體";
const char NoSD_CN[] = "没有SD卡";
const char NoSD_JP[] = "いいえTFカード";
const char * NoSD_Str[] = { NoSD_EN,NoSD_TW,NoSD_CN,NoSD_JP };

const char WaitingForEBD_EN[] = "Waiting for EBD...";
const char WaitingForEBD_TW[] = "等待設備...";
const char WaitingForEBD_CN[] = "等待设备...";
const char WaitingForEBD_JP[] = "接続します...";
const char * WaitingForEBD_Str[] = { WaitingForEBD_EN,WaitingForEBD_TW,WaitingForEBD_CN,WaitingForEBD_JP };

const char EBDConnected_EN[] = "EBD Connected";
const char EBDConnected_TW[] = "設備已連接";
const char EBDConnected_CN[] = "设备已连接";
const char EBDConnected_JP[] = "接続すでに";
const char * EBDConnected_Str[] = { EBDConnected_EN,EBDConnected_TW,EBDConnected_CN,EBDConnected_JP };

const char Confirmation_EN[] = "Confirmation";
const char Confirmation_TW[] = "確認";
const char Confirmation_CN[] = "确认";
const char Confirmation_JP[] = "確認します";
const char * Confirmation_Str[] = { Confirmation_EN,Confirmation_TW,Confirmation_CN,Confirmation_JP };

const char ConfirmCancel_EN[] = "Cancel%Comfirm";
const char ConfirmCancel_TW[] = " 取消 % 確認 ";
const char ConfirmCancel_CN[] = " 取消 % 确认 " ;
const char ConfirmCancel_JP[] = " キャンセル % はい ";
const char * ConfirmCancel_Str[] = { ConfirmCancel_EN,ConfirmCancel_TW,ConfirmCancel_CN,ConfirmCancel_JP };

const char RecordConfirm_EN[] = "Start Record?";
const char RecordConfirm_TW[] = "開始記錄曲線? ";
const char RecordConfirm_CN[] = "开始记录曲线?" ;
const char RecordConfirm_JP[] = "曲線レコード?";
const char * RecordConfirm_Str[] = { RecordConfirm_EN,RecordConfirm_TW,RecordConfirm_CN,RecordConfirm_JP };

const char QCMTKConfirm_EN[] = "QC/MTK Trigger?";
const char QCMTKConfirm_TW[] = "QC/MTK觸發?";
const char QCMTKConfirm_CN[] = "QC/MTK触发?" ;
const char QCMTKConfirm_JP[] = "QC/MTKトリガー?";
const char * QCMTKConfirm_Str[] = { QCMTKConfirm_EN,QCMTKConfirm_TW,QCMTKConfirm_CN,QCMTKConfirm_JP };

const char StepUpConfirm_EN[] ="Step-up Test?";
const char StepUpConfirm_TW[] ="開始步進測試?";
const char StepUpConfirm_CN[] ="开始步进测试?" ;
const char StepUpConfirm_JP[] ="StepUpテスト?";
const char * StepUpConfirm_Str[] = { StepUpConfirm_EN,StepUpConfirm_TW,StepUpConfirm_CN,StepUpConfirm_JP };

const char Saved_EN[] = "Saved";
const char Saved_TW[] = "已保存";
const char Saved_CN[] = "已保存";
const char Saved_JP[] = "保存";
const char * Saved_Str[] = { Saved_EN,Saved_TW,Saved_CN,Saved_JP };

const char Settings_EN[]="Unmount Disk%Clock Settings%Buzzer Settings%Language%System Scan%System Info";
const char Settings_TW[]="彈出SD卡%時間設置%警報設置%Language%系統掃描%系統資訊";
const char Settings_CN[] = "安全弹出存储卡%时间设置%蜂鸣器设置%Language%系统扫描%系统信息" ;
const char Settings_JP[] ="イジェクトメモリ%時刻の設定%警告の設定%Language%システムスキャン%システム情報";
const char * Settings_Str[] = { Settings_EN,Settings_TW,Settings_CN,Settings_JP };

const char SetYear_EN[]="Set year";
const char SetYear_TW[]="設置年份";
const char SetYear_CN[] = "设置年份" ;
const char SetYear_JP[] ="年の設定";
const char * SetYear_Str[] = { SetYear_EN,SetYear_TW,SetYear_CN,SetYear_JP };

const char SetYearUnit_EN[] ="";
const char SetYearUnit_TW[] ="年";
const char SetYearUnit_CN[] ="年" ;
const char SetYearUnit_JP[] ="年";
const char * SetYearUnit_Str[] = { SetYearUnit_EN,SetYearUnit_TW,SetYearUnit_CN,SetYearUnit_JP };

const char SetMonth_EN[]="Set month";
const char SetMonth_TW[]="設置月份";
const char SetMonth_CN[] = "设置月份" ;
const char SetMonth_JP[] ="月の設定";
const char * SetMonth_Str[] = { SetMonth_EN,SetMonth_TW,SetMonth_CN,SetMonth_JP };

const char SetMonthUnit_EN[] ="";
const char SetMonthUnit_TW[] ="月";
const char SetMonthUnit_CN[] ="月" ;
const char SetMonthUnit_JP[] ="月";
const char * SetMonthUnit_Str[] = { SetMonthUnit_EN,SetMonthUnit_TW,SetMonthUnit_CN,SetMonthUnit_JP };

const char SetDay_EN[]="Set day";
const char SetDay_TW[]="設置日期";
const char SetDay_CN[] = "设置日期" ;
const char SetDay_JP[] ="天の設定";
const char * SetDay_Str[] = { SetDay_EN,SetDay_TW,SetDay_CN,SetDay_JP };

const char SetDayUnit_EN[] ="";
const char SetDayUnit_TW[] ="日";
const char SetDayUnit_CN[] ="日" ;
const char SetDayUnit_JP[] ="日";
const char * SetDayUnit_Str[] = { SetDayUnit_EN,SetDayUnit_TW,SetDayUnit_CN,SetDayUnit_JP };

const char SetHour_EN[]="Set hour";
const char SetHour_TW[]="設置小時";
const char SetHour_CN[] = "设置小时" ;
const char SetHour_JP[] ="時間を設定します";
const char * SetHour_Str[] = { SetHour_EN,SetHour_TW,SetHour_CN,SetHour_JP };

const char SetHourUnit_EN[] ="";
const char SetHourUnit_TW[] ="時";
const char SetHourUnit_CN[] ="时" ;
const char SetHourUnit_JP[] ="時間";
const char * SetHourUnit_Str[] = { SetHourUnit_EN,SetHourUnit_TW,SetHourUnit_CN,SetHourUnit_JP };

const char SetMin_EN[]="Set min";
const char SetMin_TW[]="設置分";
const char SetMin_CN[] = "设置分" ;
const char SetMin_JP[] ="分を設定します";
const char * SetMin_Str[] = { SetMin_EN,SetMin_TW,SetMin_CN,SetMin_JP };

const char SetMinUnit_EN[] ="";
const char SetMinUnit_TW[] ="分";
const char SetMinUnit_CN[] ="分" ;
const char SetMinUnit_JP[] ="分";
const char * SetMinUnit_Str[] = { SetMinUnit_EN,SetMinUnit_TW,SetMinUnit_CN,SetMinUnit_JP };

const char SetSec_EN[]="Set sec";
const char SetSec_TW[]="設置秒";
const char SetSec_CN[] = "设置秒" ;
const char SetSec_JP[] ="秒を設定します";
const char * SetSec_Str[] = { SetSec_EN,SetSec_TW,SetSec_CN,SetSec_JP };

const char SetSecUnit_EN[] ="";
const char SetSecUnit_TW[] ="分";
const char SetSecUnit_CN[] ="分" ;
const char SetSecUnit_JP[] ="秒";
const char * SetSecUnit_Str[] = { SetSecUnit_EN,SetSecUnit_TW,SetSecUnit_CN,SetSecUnit_JP };
	
const char TimeSetting_EN[] ="Applied";
const char TimeSetting_TW[] ="已應用";
const char TimeSetting_CN[] ="已应用" ;
const char TimeSetting_JP[] ="適用されました";
const char * TimeSetting_Str[] = { TimeSetting_EN,TimeSetting_TW,TimeSetting_CN,TimeSetting_JP };

const char StartCurrentGet_EN[] ="I.Start";
const char StartCurrentGet_TW[] ="起始電流";
const char StartCurrentGet_CN[] ="起始电流" ;
const char StartCurrentGet_JP[] ="初期電流";
const char * StartCurrentGet_Str[] = { StartCurrentGet_EN,StartCurrentGet_TW,StartCurrentGet_CN,StartCurrentGet_JP };

const char EndCurrentGet_EN[] ="I.End";
const char EndCurrentGet_TW[] ="終止電流";
const char EndCurrentGet_CN[] ="终止电流" ;
const char EndCurrentGet_JP[] ="終了電流";
const char * EndCurrentGet_Str[] = { EndCurrentGet_EN,EndCurrentGet_TW,EndCurrentGet_CN,EndCurrentGet_JP };

const char StepCurrentGet_EN[] ="I.Step";
const char StepCurrentGet_TW[] ="步距";
const char StepCurrentGet_CN[] ="步距" ;
const char StepCurrentGet_JP[] ="ステップ";
const char * StepCurrentGet_Str[] = { StepCurrentGet_EN,StepCurrentGet_TW,StepCurrentGet_CN,StepCurrentGet_JP };

const char TimeIntervalGet_EN[] ="T.Interval";
const char TimeIntervalGet_TW[] ="時間間隔";
const char TimeIntervalGet_CN[] ="时间间隔" ;
const char TimeIntervalGet_JP[] ="インターバル";
const char * TimeIntervalGet_Str[] = { TimeIntervalGet_EN,TimeIntervalGet_TW,TimeIntervalGet_CN,TimeIntervalGet_JP };

const char ProtVoltageGet_EN[] ="V.Protect";
const char ProtVoltageGet_TW[] ="保護電壓";
const char ProtVoltageGet_CN[] ="保护电压" ;
const char ProtVoltageGet_JP[] ="電圧防護";
const char * ProtVoltageGet_Str[] = { ProtVoltageGet_EN,ProtVoltageGet_TW,ProtVoltageGet_CN,ProtVoltageGet_JP };

const char RunTime_EN[] ="RunTime:";
const char RunTime_TW[] ="當前時間";
const char RunTime_CN[] ="当前时间" ;
const char RunTime_JP[] ="現在時刻";
const char * RunTime_Str[] = { RunTime_EN,RunTime_TW,RunTime_CN,RunTime_JP };

const char CurrentVoltage_EN[] ="Voltage:";
const char CurrentVoltage_TW[] ="當前電壓";
const char CurrentVoltage_CN[] ="当前电压" ;
const char CurrentVoltage_JP[] ="現在電圧";
const char * CurrentVoltage_Str[] = { CurrentVoltage_EN,CurrentVoltage_TW,CurrentVoltage_CN,CurrentVoltage_JP };

const char CurrentCurrent_EN[] ="Current:";
const char CurrentCurrent_TW[] ="當前電流";
const char CurrentCurrent_CN[] ="当前电流" ;
const char CurrentCurrent_JP[] ="現在電流";
const char * CurrentCurrent_Str[] = { CurrentCurrent_EN,CurrentCurrent_TW,CurrentCurrent_CN,CurrentCurrent_JP };

const char StepUpTestRunning_EN[] ="Test Running...";
const char StepUpTestRunning_TW[] ="正在測試...";
const char StepUpTestRunning_CN[] ="正在测试..." ;
const char StepUpTestRunning_JP[] ="テスト運転中...";
const char * StepUpTestRunning_Str[] = { StepUpTestRunning_EN,StepUpTestRunning_TW,StepUpTestRunning_CN,StepUpTestRunning_JP };

const char StepUpTestProtected_EN[] ="Protected";
const char StepUpTestProtected_TW[] ="已保護";
const char StepUpTestProtected_CN[] ="已保护" ;
const char StepUpTestProtected_JP[] ="保護されました";
const char * StepUpTestProtected_Str[] = { StepUpTestProtected_EN,StepUpTestProtected_TW,StepUpTestProtected_CN,StepUpTestProtected_JP };

const char StepUpTestDone_EN[] ="Test Done";
const char StepUpTestDone_TW[] ="已完成測試";
const char StepUpTestDone_CN[] ="已完成测试" ;
const char StepUpTestDone_JP[] ="完成しました";
const char * StepUpTestDone_Str[] = { StepUpTestDone_EN,StepUpTestDone_TW,StepUpTestDone_CN,StepUpTestDone_JP };

const char StepUpTestDialgram_EN[] ="Dialgram";
const char StepUpTestDialgram_TW[] ="圖像";
const char StepUpTestDialgram_CN[] ="图像" ;
const char StepUpTestDialgram_JP[] ="ダイアグラム";
const char * StepUpTestDialgram_Str[] = { StepUpTestDialgram_EN,StepUpTestDialgram_TW,StepUpTestDialgram_CN,StepUpTestDialgram_JP };

const char StepUpTestList_EN[] ="ListView";
const char StepUpTestList_TW[] ="表格";
const char StepUpTestList_CN[] ="表格" ;
const char StepUpTestList_JP[] ="テーブル";
const char * StepUpTestList_Str[] = { StepUpTestList_EN,StepUpTestList_TW,StepUpTestList_CN,StepUpTestList_JP };

const char StepUpTestDialgramTime_EN[] ="Time";
const char StepUpTestDialgramTime_TW[] ="時間";
const char StepUpTestDialgramTime_CN[] ="时间" ;
const char StepUpTestDialgramTime_JP[] ="時間";
const char * StepUpTestDialgramTime_Str[] = { StepUpTestDialgramTime_EN,StepUpTestDialgramTime_TW,StepUpTestDialgramTime_CN,StepUpTestDialgramTime_JP };

const char StepUpTestDialgramCurrent_EN[] ="Current";
const char StepUpTestDialgramCurrent_TW[] ="電流";
const char StepUpTestDialgramCurrent_CN[] ="电流" ;
const char StepUpTestDialgramCurrent_JP[] ="電流";
const char * StepUpTestDialgramCurrent_Str[] = { StepUpTestDialgramCurrent_EN,StepUpTestDialgramCurrent_TW,StepUpTestDialgramCurrent_CN,StepUpTestDialgramCurrent_JP };

const char StepUpTestDialgramVoltage_EN[] ="Voltage";
const char StepUpTestDialgramVoltage_TW[] ="電壓";
const char StepUpTestDialgramVoltage_CN[] ="电压" ;
const char StepUpTestDialgramVoltage_JP[] ="電圧";
const char * StepUpTestDialgramVoltage_Str[] = { StepUpTestDialgramVoltage_EN,StepUpTestDialgramVoltage_TW,StepUpTestDialgramVoltage_CN,StepUpTestDialgramVoltage_JP };

const char StepUpTestExitBroswer_EN[] ="Exit Broswer?";
const char StepUpTestExitBroswer_TW[] ="退出瀏覽?";
const char StepUpTestExitBroswer_CN[] ="退出浏览?" ;
const char StepUpTestExitBroswer_JP[] ="終了ブラウザ?";
const char * StepUpTestExitBroswer_Str[] = { StepUpTestExitBroswer_EN,StepUpTestExitBroswer_TW,StepUpTestExitBroswer_CN,StepUpTestExitBroswer_JP };

const OLED_PositionStruct ComfirmationPositions[LanguageNum][2] =
{
{20,45,76,45},
{36,45,84,45},
{36,45,84,45},
{8,45,84,45}
};


