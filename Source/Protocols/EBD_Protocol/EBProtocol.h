#ifndef __EBPROTOCOL_H
#define	__EBPROTOCOL_H

#include "stm32f10x.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//EBD CommandSet,ignore if not needed
#define EBD_STOP_BYTE       0xf8
#define EBD_COMMAND_LENGTH  10
#define EBD_COMMAND_VERIFY_BYTE_ADDR EBD_COMMAND_LENGTH-2
#define EBD_COMMAND_STOP_BYTE_ADDR   EBD_COMMAND_LENGTH-1
#define EBD_COMMAND_END_OF_ADDR      EBD_COMMAND_LENGTH
#define EBD_PACKET_LENGTH  19
#define EBD_PACKET_VERIFY_BYTE_ADDR EBD_PACKET_LENGTH-2

#define EBD_QC_COMMAND_ADDR   0x07
#define EBD_QC2_5V_COMMAND    0x0a
#define EBD_QC2_9V_COMMAND    0x0b
#define EBD_QC2_12V_COMMAND   0x0c
#define EBD_QC2_20V_COMMAND   0x0d
#define EBD_QC3_PLUS_COMMAND  0x0f
#define EBD_QC3_MINUS_COMMAND 0x10
#define EBD_MTK_PLUS_COMMAND  0x15
#define EBD_MTK_MINUS_COMMAND 0x14

#define EBD_LOAD_COMMAND_START_ADDR    0x01
#define EBD_LOAD_COMMAND_ADDRH         0x02
#define EBD_LOAD_COMMAND_ADDRL         0x03
#define EBD_LOAD_COMMAND_RUNNING_ADDR  0x05

#define EBD_LOAD_RUNNING_COMMAND       0x50
#define EBD_LOAD_START_COMMAND         0x01
#define EBD_LOAD_STOP_COMMAND          0x02
#define EBD_LOAD_KEEP_COMMAND          0x07

#define EBD_MAIN_VOLATGE_ADDR          0x04
#define EBD_DP_VOLATGE_ADDR            0x06
#define EBD_DM_VOLATGE_ADDR            0x08

extern xQueueHandle EBDTxDataMsg;

extern unsigned char EBDBackPacket[EBD_PACKET_LENGTH + 1];

extern u8 EBDCommandStringCache[EBD_COMMAND_LENGTH + 1];

extern unsigned char ReceiveAddr;

extern unsigned char ReceiveTimeOut;

extern bool EBDPacketReceivingFlag;

extern bool EBDPacketReceiveEnable;

extern bool BadPacketReceivedFlag;

extern bool EBDPacketReceivedFlag;

extern bool SumUpInProgress;

extern bool EBDExceptionHandleOnegaiFlag;

enum { StopTest = 0, KeepTest = 1, StartTest = 2 };

typedef struct {
	float Voltage;
	float Current;
	float Power;
	float VoltageDP;
	float VoltageDM;
}USBMeterStruct;

typedef struct {
	float Capacity;
	float Work;
	float PlatformVolt;
}SumupStruct;

typedef struct {
	u8 Plus;
	u8 Minus;
}FastChargeAdjustCommandSetStruct;

typedef struct {
	u8 CurrentH;
	u8 CurrentL;
}EBDLoadCommandStruct;

extern USBMeterStruct CurrentMeterData;

extern SumupStruct    CurrentSumUpData;

//see .c for details
void EBDUSB_LinkStart(bool setupHandler);

//see .c for details
void EBDPacketHandler(short lese, short lese2, unsigned char lese3);

//see .c for details
void PacketDecode(void);

//see .c for details
void SumUpDataHandler(void);

//see .c for details
void EBDSendFastChargeCommand(unsigned char command);

//see .c fpr details
void EBDSendLoadCommand(u16 current, u8 mode);

//see .c for details
void EBDWatchingDogSetup(void);

void EBD_Init(void);

#endif /* __EBPROTOCOL_H */