#include "MultiLanguageStrings.h"
#include "SSD1306.h"

const char SystemInit_EN[] = "System Init...";
const char SystemInit_TW[] = "╰参﹍て...";
const char SystemInit_JC[] = "系统初始化...";
const char SystemInit_JP[] = "僔僗僥儉偺弶婜壔...";
const char * SystemInit_Str[] = { SystemInit_EN,SystemInit_TW,SystemInit_JC,SystemInit_JP };

const char Capacity_EN[] = "Capacity:%dMB";
const char Capacity_TW[] = "甧秖:%dMB";
const char Capacity_JC[] = "容量:%dMB";
const char Capacity_JP[] = "梕検:%dMB";
const char * Capacity_Str[] = { Capacity_EN,Capacity_TW,Capacity_JC,Capacity_JP };

const char NoSD_EN[] = "No SDCard";
const char NoSD_TW[] = "⊿Τ癘拘砰";
const char NoSD_JC[] = "没有SD卡";
const char NoSD_JP[] = "偄偄偊TF僇乕僪";
const char * NoSD_Str[] = { NoSD_EN,NoSD_TW,NoSD_JP };

const char WaitingForEBD_EN[] = "Waiting for EBD...";
const char WaitingForEBD_TW[] = "单砞称...";
const char WaitingForEBD_JC[] = "等待设备...";
const char WaitingForEBD_JP[] = "愙懕偟傑偡...";
const char * WaitingForEBD_Str[] = { WaitingForEBD_EN,WaitingForEBD_TW,WaitingForEBD_JC,WaitingForEBD_JP };

const char EBDConnected_EN[] = "EBD Connected";
const char EBDConnected_TW[] = "砞称硈钡";
const char EBDConnected_JC[] = "设备已连接";
const char EBDConnected_JP[] = "愙懕偡偱偵";
const char * EBDConnected_Str[] = { EBDConnected_EN,EBDConnected_TW,EBDConnected_JC,EBDConnected_JP };

const char Confirmation_EN[] = "Confirmation";
const char Confirmation_TW[] = "絋粄";
const char Confirmation_JC[] = "确认";
const char Confirmation_JP[] = "妋擣偟傑偡";
const char * Confirmation_Str[] = { Confirmation_EN,Confirmation_TW,Confirmation_JC,Confirmation_JP };

const char ConfirmCancel_EN[] = "Cancel%Comfirm";
const char ConfirmCancel_TW[] = "  % 絋粄 ";
const char ConfirmCancel_JC[] = " 取消 % 确认 " ;
const char ConfirmCancel_JP[] = " 僉儍儞僙儖 % 偼偄 ";
const char * ConfirmCancel_Str[] = { ConfirmCancel_EN,ConfirmCancel_TW,ConfirmCancel_JC,ConfirmCancel_JP };

const char RecordConfirm_EN[] = "Start Record?";
const char RecordConfirm_TW[] = "秨﹍癘魁Ρ絬? ";
const char RecordConfirm_JC[] = "开始记录曲线?" ;
const char RecordConfirm_JP[] = "嬋慄儗僐乕僪?";
const char * RecordConfirm_Str[] = { RecordConfirm_EN,RecordConfirm_TW,RecordConfirm_JC,RecordConfirm_JP };

const char QCMTKConfirm_EN[] = "QC/MTK Trigger?";
const char QCMTKConfirm_TW[] = "QC/MTK牟祇? ";
const char QCMTKConfirm_JC[] = "QC/MTK触发?" ;
const char QCMTKConfirm_JP[] = "QC/MTK僩儕僈乕?";
const char * QCMTKConfirm_Str[] = { QCMTKConfirm_EN,QCMTKConfirm_TW,QCMTKConfirm_JC,QCMTKConfirm_JP };

const OLED_PositionStruct ComfirmationPositions[LanguageNum][2] =
{
{20,45,76,45},
{36,45,84,45},
{36,45,84,45},
{12,45,84,45}
};




