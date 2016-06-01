//File Name   £ºEBProtocol.c
//Description : Protocol for EBTester

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "stdbool.h"

#include "USARTS.h"
#include "EBProtocol.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Startup.h"

#define EBD_PACKET_RECEIVER_PRIORITY tskIDLE_PRIORITY+5

//EBD command string example
unsigned char EBDUSBStandardString[8] = { 0xfa,0x05,0x00,0x00,0x00,0x00,0x00,0x00 };

unsigned char DummyResponse[19] = { 0xfa,0x00,0x00,0x00,0x16,0x38,
0x00,0x00,0x00,0x00,0x0b,0x3c,0x00,0x50,0x00,0x00,0x1a,0x53,0xf8 };

//Cache for the will-transmitted string
u8 EBDCommandStringCache[EBD_COMMAND_LENGTH + 1];

//Packet received buffer
u8 EBDBackPacket[EBD_PACKET_LENGTH + 1];

//Relative addr in EBDBackPacket,used to receive packet byte by byte
unsigned char ReceiveAddr = 0;

//Receive timeout count,used to judge if a packet has been fully-received
unsigned char ReceiveTimeOut = 0;

//ReceivingFlag,used to judge if a packet has been fully-received
bool EBDPacketReceivingFlag = false;

//An onegaiFlag indicates if a new packet has been received
volatile bool EBDNewPacketHandleOnegai = false;

bool IgnoreNextEBDErr = false;

bool EBDPacketReceivedFlag = false;

bool EBDPacketReceiveEnable = false;

bool EBDExceptionHandleOnegaiFlag = false;

//BadPacketReceivedFlag,used to indicate if a bad packet(usually
//over-length or with a wrong checksum)received
bool BadPacketReceivedFlag = false;

bool SumUpInProgress = false;

u8 EBDWatchCount = 0;

USBMeterStruct CurrentMeterData;

SumupStruct    CurrentSumUpData;

xQueueHandle EBDTxDataMsg;

/**
  * @brief   To keep the communications between EBD-USB and PC,
	the Tx Pin of the MCU cannot be push mode all the time,this function
	is used to start a data transfer from MCU(ATTENTION!!It's MCU) to
  EBD-USB
  MUST CALL TxStop() AFTER USE!!
  * @retval : None
  */
void TxStart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief   To keep the communications between EBD-USB and PC,
	the Tx Pin of the MCU cannot be push mode all the time,this function
	is used to stop a data transfer from MCU(ATTENTION!!It's MCU) to
  EBD-USB
  MUST CALL THIS FUNCTION AFTER USE!!
  * @retval : None
  */
void TxStop(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//Delay_Us(2000);//Wait for data to be transferred
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief   EB Protocol rules a verification byte after the data bytes
  This function is used to generate this byte
  * @parameter: Oringinal string
	 function will generate the verify byte of the segment from startAddr
	 to stopAddr
  * @retval : VerifyByte
  */
u8 GenerateVerifyByte(u8 string[], u8 startAddr, u8 stopAddr)
{
	u8 i;
	u8 verifyByte = 0;
	for (i = startAddr; i < stopAddr + 1; i++)
	{
		verifyByte ^= string[i];
	}
	return(verifyByte);
}

/**
  * @brief   Start communicating with EBD-USB
  * @retval : None
  */
void EBDUSB_LinkStart(bool setupHandler)
{
	u8 i;
	for (i = 0; i < 8; i++)
	{
		EBDCommandStringCache[i] = EBDUSBStandardString[i];
	}
	i = GenerateVerifyByte(EBDCommandStringCache, 1, 7);
	EBDCommandStringCache[EBD_COMMAND_VERIFY_BYTE_ADDR] = i;
	EBDCommandStringCache[EBD_COMMAND_STOP_BYTE_ADDR] = EBD_STOP_BYTE;
	EBDCommandStringCache[EBD_COMMAND_END_OF_ADDR] = 0;
	xQueueSend(EBDTxDataMsg, &i, 100 / portTICK_RATE_MS);
	if (setupHandler == true)
	{
		;
	}
	EBDPacketReceiveEnable = true;
}


/**
  * @brief   This is the interrupt processing function for USART1
  *          This function receive the EBD-USB packet from USART1
  *          Work with EBDPacketHandler to deal with the receiving error
  * @retval : VerifyByte
  */
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		EBDPacketReceivingFlag = true;
		EBDPacketReceivedFlag = true;
		if (ReceiveAddr > EBD_PACKET_LENGTH - 1)
		{
			BadPacketReceivedFlag = true;
			ReceiveAddr = 0;
		}
		EBDBackPacket[ReceiveAddr] = USART_ReceiveData(USART1);
		ReceiveAddr++;
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

/**
  * @brief   Called by SystemBeats
			 Used to change ebdNewPacketHandleOnegai for other
			 process which needs to update new data in the new
			 packet
			 When flag "EBDPacketReceivingFlag"keeps false for
			 10 times(about 50ms),delt with a timeout, if the
			 new packet passed the check,ebdNewPacketHandleOnegai
			 will be true(a data refresh onegai triggerred)
  * @retval : VerifyByte
  */

void EBDPacketHandler(short lese, short lese2, unsigned char lese3)
{
	if (lese)  lese = 0;
	if (lese2) lese2 = 0;
	if (lese3) lese3 = 0;//Throw the rubbish
	if ((EBDPacketReceivingFlag == false) && (EBDPacketReceivedFlag == true))
		//If a new packet is being(or is)received
	{
		ReceiveTimeOut++;
		if (ReceiveTimeOut >= 10)
			//If "EBDPacketReceivingFlag" kept false for 15 times(at least 50ms)
		{
			ReceiveTimeOut = 0;//Clear Timeout
			EBDPacketReceivedFlag = false;//Clear ReceivedFlag
			if (BadPacketReceivedFlag == true)//If it is a bad packet
			{
				EBDNewPacketHandleOnegai = false;//Do not onegai
				BadPacketReceivedFlag = false;//Clear badFlag
				return;
			}
			EBDWatchCount = 0;//Clear WatchDog
			ReceiveAddr = 0;//Clear ReceiveAddr for the next packet
			EBDNewPacketHandleOnegai = true;//Trigger a newPacketHandleOnegai
		}
	}
	if (EBDPacketReceivingFlag == true)
		//If  "EBDPacketReceivingFlag" is reset to true by USART1_IRQHandler()
		//,means the receiving process is still active
	{
		ReceiveTimeOut = 0;//Clear TimeOut
	}
	EBDPacketReceivingFlag = false;
	//Make "EBDPacketReceivingFlag" false to detect if USART1_IRQHandler()
	//is active
}

/**
  * @brief   Decode the voltage data
  * @retval : Changes in struct CurrentMeterData
  */
void PacketDecode(void)
{
	u8 i;
	i = GenerateVerifyByte(EBDBackPacket, 1, 16);
	if (i !=
		EBDBackPacket[EBD_PACKET_VERIFY_BYTE_ADDR] && i < 240 &&
		IgnoreNextEBDErr == false)
	{
		return;
	}
	IgnoreNextEBDErr = true;

	CurrentMeterData.Voltage = (float)((EBDBackPacket[4] * 256 + EBDBackPacket[5] - 320) -
		((int)(EBDBackPacket[4] * 256 + EBDBackPacket[5]) - 5120) / 256 * 16) / 1000;
	if (EBDBackPacket[4] * 256 + EBDBackPacket[5] < 5120)
	{
		CurrentMeterData.Voltage = (float)((EBDBackPacket[4] * 256 + EBDBackPacket[5] - 320) +
			((int)5376 - (EBDBackPacket[4] * 256 + EBDBackPacket[5])) / 256 * 16) / 1000;
	}
	if (CurrentMeterData.Voltage < 0.1) CurrentMeterData.Voltage = 0;

	CurrentMeterData.VoltageDP = (float)((EBDBackPacket[6] * 256 + EBDBackPacket[7] - 320) -
		((int)(EBDBackPacket[6] * 256 + EBDBackPacket[7]) - 5120) / 256 * 16) / 1000;
	if (EBDBackPacket[4] * 256 + EBDBackPacket[5] < 5120)
	{
		CurrentMeterData.VoltageDP = (float)((EBDBackPacket[6] * 256 + EBDBackPacket[7] - 320) +
			((int)5376 - (EBDBackPacket[6] * 256 + EBDBackPacket[7])) / 256 * 16) / 1000;
	}
	if (CurrentMeterData.VoltageDP < 0) CurrentMeterData.VoltageDP = 0;

	CurrentMeterData.VoltageDM = (float)((EBDBackPacket[8] * 256 + EBDBackPacket[9] - 320) -
		((int)(EBDBackPacket[8] * 256 + EBDBackPacket[9]) - 5120) / 256 * 16) / 1000;
	if (EBDBackPacket[8] * 256 + EBDBackPacket[9] < 5120)
	{
		CurrentMeterData.VoltageDM = (float)((EBDBackPacket[8] * 256 + EBDBackPacket[9] - 320) +
			((int)5376 - (EBDBackPacket[8] * 256 + EBDBackPacket[9])) / 256 * 16) / 1000;
	}
	if (CurrentMeterData.VoltageDM < 0) CurrentMeterData.VoltageDM = 0;

	CurrentMeterData.Current = (float)((EBDBackPacket[2] * 256 + EBDBackPacket[3]) -
		(EBDBackPacket[2] * 256 + EBDBackPacket[3]) / 256 * 16) / 10000;

	CurrentMeterData.Power = CurrentMeterData.Voltage*CurrentMeterData.Current;
}

/**
  * @brief   Called by RTC
			 Sum up data every 1s
  * @retval : None
  */
void SumUpDataHandler(void)
{
	CurrentSumUpData.Capacity += (CurrentMeterData.Current / 3600);
	CurrentSumUpData.Work += (CurrentMeterData.Power / 3600);
	CurrentSumUpData.PlatformVolt = CurrentMeterData.Power / CurrentMeterData.Current;
}

/**
  * @brief   Send FastCharge Command
  * @retval : None
  */
void EBDSendFastChargeCommand(unsigned char command)
{
	u8 i;
	TxStart();
	EBDCommandStringCache[0] = 0xfa;
	EBDCommandStringCache[1] = 0x07;
	for (i = 2; i < EBD_QC_COMMAND_ADDR; i++)
	{
		EBDCommandStringCache[i] = 0x00;
	}
	EBDCommandStringCache[EBD_QC_COMMAND_ADDR] = command;
	i = GenerateVerifyByte(EBDCommandStringCache, 1, 7);
	EBDCommandStringCache[EBD_COMMAND_VERIFY_BYTE_ADDR] = i;
	EBDCommandStringCache[EBD_COMMAND_STOP_BYTE_ADDR] = EBD_STOP_BYTE;
	EBDCommandStringCache[EBD_COMMAND_END_OF_ADDR] = 0;
	TxStart();
	for (i = 0; i < EBD_COMMAND_LENGTH; i++)
	{
		UART1SendByte(EBDCommandStringCache[i]);
	}
	TxStop();
}

/**
  * @brief   Generate EBD current command
  * @retval : A struct which includes currentH and currentL
  */
EBDLoadCommandStruct EBDGenerateLoadCommand(u16 current)
{
	u16 m;
	EBDLoadCommandStruct currentCommand;
	m = current / 240 * 16 + current;
	currentCommand.CurrentH = m / 256;
	currentCommand.CurrentL = m % 256;
	return(currentCommand);
}

/**
  * @brief   Send load command
  * @retval : None
  */
void EBDSendLoadCommand(u16 current, u8 mode)
{
	u8 i;
	EBDLoadCommandStruct loadCommand;
	/*Get load command*/
	loadCommand = EBDGenerateLoadCommand(current);
	for (i = 0; i < 8; i++)
	{
		EBDCommandStringCache[i] = EBDUSBStandardString[i];
	}
	/*Set command according to mode and current*/
	switch (mode)
	{
	case StartTest:
		EBDCommandStringCache[EBD_LOAD_COMMAND_START_ADDR] =
			EBD_LOAD_START_COMMAND;
		EBDCommandStringCache[EBD_LOAD_COMMAND_RUNNING_ADDR] =
			EBD_LOAD_RUNNING_COMMAND;
		EBDCommandStringCache[EBD_LOAD_COMMAND_ADDRH] =
			loadCommand.CurrentH;
		EBDCommandStringCache[EBD_LOAD_COMMAND_ADDRL] =
			loadCommand.CurrentL; break;

	case StopTest:
		EBDCommandStringCache[EBD_LOAD_COMMAND_START_ADDR] =
			EBD_LOAD_STOP_COMMAND; break;

	case KeepTest:
		EBDCommandStringCache[EBD_LOAD_COMMAND_START_ADDR] =
			EBD_LOAD_KEEP_COMMAND;
		EBDCommandStringCache[EBD_LOAD_COMMAND_RUNNING_ADDR] =
			EBD_LOAD_RUNNING_COMMAND;
		EBDCommandStringCache[EBD_LOAD_COMMAND_ADDRH] =
			loadCommand.CurrentH;
		EBDCommandStringCache[EBD_LOAD_COMMAND_ADDRL] =
			loadCommand.CurrentL;
	}
	/*Generate verify byte*/
	EBDCommandStringCache[EBD_COMMAND_VERIFY_BYTE_ADDR] =
		GenerateVerifyByte(EBDCommandStringCache, 1, 7);
	/*Set stop byte and end string*/
	EBDCommandStringCache[EBD_COMMAND_STOP_BYTE_ADDR] =
		EBD_STOP_BYTE;
	EBDCommandStringCache[EBD_COMMAND_END_OF_ADDR] = 0;
	/*Send Command*/
	TxStart();
	for (i = 0; i < EBD_COMMAND_LENGTH; i++)
	{
		UART1SendByte(EBDCommandStringCache[i]);
	}
	TxStop();
}

/**
  * @brief   Called by SystemBeats
Handle the watchDog for EBD

  * @param : Rubbish

  * @retval : None
  */
void EBDWatchingDogHandler(short lese, short lese2, unsigned char lese3)
{
	EBDWatchCount++;
	/*If EBD keeps not responding for at least 3s*/
	if (EBDWatchCount == 3)
	{
		EBDWatchCount = 1;
		/*Make an EBDExceptionHandleOnegai*/
		EBDExceptionHandleOnegaiFlag = true;
	}
	/*For every while in which EBD responds,clear the exception flag*/
	if (EBDWatchCount == 0)
	{
		EBDExceptionHandleOnegaiFlag = false;
	}
}

void EBDPacketTransmitter(void *pvParameters)
{
	u8 i;
	while (1)
	{
		while (xQueueReceive(EBDTxDataMsg, &i, portMAX_DELAY) != pdPASS);
		TxStart();
		vTaskDelay(1 / portTICK_RATE_MS);
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		DMA_Cmd(DMA1_Channel4, ENABLE);
		vTaskDelay(1 / portTICK_RATE_MS);
	}
}

/**
  * @brief   Setup watchingDog of EBD

  * @retval : None
  */
void EBDWatchingDogSetup(void)
{
	;
}

void EBD_Init(void)
{
	xQueueSend(InitStatusMsg, "Waiting for EBD...", 100 / portTICK_RATE_MS);
	EBDTxDataMsg = xQueueCreate(1, sizeof(u8));
	xTaskCreate(EBDPacketTransmitter, "EBD Packet Transmitter",
		128, NULL, EBD_PACKET_RECEIVER_PRIORITY, NULL);
	taskENTER_CRITICAL();
	USART1_TX_DMA_Init();
	USART1_Init();
	taskEXIT_CRITICAL();
	EBDUSB_LinkStart(true);
}

