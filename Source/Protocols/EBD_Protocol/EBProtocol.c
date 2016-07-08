//File Name     EBProtocol.c
//Description : Protocol for EBTester

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "stdbool.h"

#include "USARTS.h"
#include "EBProtocol.h"
#include "SSD1306.h"
#include "MultiLanguageStrings.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Settings.h"
#include "ExceptionHandle.h"

#include "EBProtocolConfig.h"

#include "Startup.h"

#define EBD_PACKET_RECEIVER_PRIORITY tskIDLE_PRIORITY+6

#define EBD_PACKET_TRANSMITTER_PRIORITY tskIDLE_PRIORITY+6

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

//ReceivingFlag,used to judge if a packet has been fully-received
bool EBDPacketReceivingFlag = false;

bool EBDPacketReceivedFlag = false;

bool EBDPacketReceiveEnable = false;

bool EBDExceptionHandleOnegaiFlag = false;

bool EBDAliveFlag = false;

//BadPacketReceivedFlag,used to indicate if a bad packet(usually
//over-length or with a wrong checksum)received
bool BadPacketReceivedFlag = false;

bool SumUpInProgress = false;

u8 EBDWatchCount = 0;

USBMeterStruct CurrentMeterData;

SumupStruct    CurrentSumUpData;

xQueueHandle EBDTxDataMsg;

xQueueHandle EBDRxDataMsg;

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
  *          Work with EBDPacketReceiver to deal with the receiving error
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

float DecodeVoltage(u8 startAddr)
{
	float voltage;
	voltage = (float)((EBDBackPacket[startAddr] * 256 + EBDBackPacket[startAddr + 1] - 320) -
		((int)(EBDBackPacket[startAddr] * 256 + EBDBackPacket[startAddr + 1]) - 5120) / 256 * 16) / 1000;
	if (EBDBackPacket[startAddr] * 256 + EBDBackPacket[startAddr + 1] < 5120)
	{
		voltage = (float)((EBDBackPacket[startAddr] * 256 + EBDBackPacket[startAddr + 1] - 320) +
			((int)5376 - (EBDBackPacket[startAddr] * 256 + EBDBackPacket[startAddr + 1])) / 256 * 16) / 1000;
	}
	if (voltage < 0.1) voltage = 0;
	return(voltage);
}

/**
  * @brief   Decode the EBD data
  * @retval : Changes in struct CurrentMeterData
  */
void PacketDecode(void)
{
	u8 i;
	i = GenerateVerifyByte(EBDBackPacket, 1, 16);
	if (i != EBDBackPacket[EBD_PACKET_VERIFY_BYTE_ADDR] && i < 240)
	{
		return;
	}

	CurrentMeterData.Voltage = DecodeVoltage(EBD_MAIN_VOLATGE_ADDR);

	if (EBD_Protocol_Config[CurrentSettings->EBD_Model]->DataPinSupport)
	{
		CurrentMeterData.VoltageDP = DecodeVoltage(EBD_DP_VOLATGE_ADDR);
		CurrentMeterData.VoltageDM = DecodeVoltage(EBD_DM_VOLATGE_ADDR);
	}

	CurrentMeterData.Current = (float)((EBDBackPacket[2] * 256 + EBDBackPacket[3]) -
		(EBDBackPacket[2] * 256 + EBDBackPacket[3]) / 256 * 16) /
		EBD_Protocol_Config[CurrentSettings->EBD_Model]->CurrentDivider;

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
	xQueueSend(EBDTxDataMsg, &i, 100 / portTICK_RATE_MS);
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
	xQueueSend(EBDTxDataMsg, &i, 100 / portTICK_RATE_MS);
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

/**
  * @brief   Task which transmit a packet to EBD when a queue triggers

  * @retval : None
  */
void EBDPacketTransmitter(void *pvParameters)
{
	u8 i;
	while (1)
	{
		while (xQueueReceive(EBDTxDataMsg, &i, portMAX_DELAY) != pdPASS);
		TxStart();
		vTaskDelay(1 / portTICK_RATE_MS);
		DMA1_Channel4->CNDTR = EBD_COMMAND_LENGTH;
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		DMA_Cmd(DMA1_Channel4, ENABLE);
		vTaskDelay(100 / portTICK_RATE_MS);
		TxStop();
	}
}

/**
  * @brief
			 Send a message to EBDRxDataMsg to notice other apps
	   that a new EBD packet was fully received
			 When flag "EBDPacketReceivingFlag"keeps false for
			 5 times(about 50ms),delt with a timeout, if the
			 new packet passed the check,ebdNewPacketHandleOnegai
			 will be true(a data refresh onegai triggerred)
  * @retval : None
  */
void EBDPacketReceiver(void *pvParameters)
{
	u8 i;
	u8 ReceiveTimeOut = 0;
	while (1)
	{
		//If a new packet is being(or is)received
		if ((EBDPacketReceivingFlag == false) && (EBDPacketReceivedFlag == true))
		{
			ReceiveTimeOut++;
			//If "EBDPacketReceivingFlag" kept false for 10 times(at least 50ms)
			if (ReceiveTimeOut >= 5)
			{
				ReceiveTimeOut = 0;//Clear Timeout
				EBDPacketReceivedFlag = false;//Clear ReceivedFlag
				if (BadPacketReceivedFlag == true)//If it is a bad packet
				{
					BadPacketReceivedFlag = false;//Clear badFlag
					//return;
				}
				EBDWatchCount = 0;//Clear WatchDog
				ReceiveAddr = 0;//Clear ReceiveAddr for the next packet
				PacketDecode();
				xQueueSend(EBDRxDataMsg, &i, 0);
				EBDAliveFlag = true;
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
		vTaskDelay(10 / portTICK_RATE_MS);
	}
}

void EBD_Sync()
{
	u8 i;
	xQueueReceive(EBDRxDataMsg, &i, portMAX_DELAY);
	xQueueReceive(EBDRxDataMsg, &i, portMAX_DELAY);
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
	u8 i;
	/*Init EBD messages*/
	EBDTxDataMsg = xQueueCreate(1, sizeof(u8));
	EBDRxDataMsg = xQueueCreate(1, sizeof(u8));
	/*Create tasks for transmitter and receiver*/
	xTaskCreate(EBDPacketTransmitter, "EBD Packet Transmitter",
		128, NULL, EBD_PACKET_TRANSMITTER_PRIORITY, NULL);
	xTaskCreate(EBDPacketReceiver, "EBD Packet Receiver",
		128, NULL, EBD_PACKET_RECEIVER_PRIORITY, NULL);
	/*Init usart*/
	taskENTER_CRITICAL();
	USART1_TX_DMA_Init();
	USART1_Init();
	taskEXIT_CRITICAL();
	/*Send connection start string to EBD*/
	vTaskDelay(100 / portTICK_RATE_MS);
	EBDUSB_LinkStart(true);
	EBDAliveFlag = true;
	/*Keep waiting until EBD responses*/
	xQueueSend(InitStatusMsg, WaitingForEBD_Str[CurrentSettings->Language], 0);
	while (xQueueReceive(EBDRxDataMsg, &i, 3000 / portTICK_RATE_MS) != pdPASS)
	{
		EBDUSB_LinkStart(true);
	}
	EBD_Exception_Handler_Init();
	/*Update initStatus*/
	xQueueSend(InitStatusMsg, EBDConnected_Str[CurrentSettings->Language], 0);
	return;
}

