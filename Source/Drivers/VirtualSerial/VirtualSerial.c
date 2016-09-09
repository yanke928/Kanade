//File Name   VirtualSerial.c
//Description VirtualSerial driver layer

#include <stdbool.h>
#include <string.h>

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_conf.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"

#include "FreeRTOS_Standard_Include.h"

#include "UI_Utilities.h"

#include "VirtualSerial.h"

#define VIRTUAL_SERIAL_PACKET_SERVICE_PRIORITY tskIDLE_PRIORITY+7
#define VIRTUAL_SERIAL_PACKET_TIMEOUT_10_MS 3
#define VIRTUAL_SERIAL_MAX_PACKET_SIZE 128

u8 ReceivedPacket[VIRTUAL_SERIAL_MAX_PACKET_SIZE];

xQueueHandle Virtual_Serial_Packet_Received_Msg;

extern uint8_t  USB_Tx_State;

/**
  * @brief  Send data to virtual serial

	  @params data,length
  */
void VirtualSerial_SendData(u8 *data, u16 length)
{
	if (linecoding.datatype == 7)
	{
		while (length)
		{
			USART_Rx_Buffer[USART_Rx_ptr_in] = *data & 0x7F;
			data++;
			length--;
			USART_Rx_ptr_in++;
			if (USART_Rx_ptr_in == USART_RX_DATA_SIZE) USART_Rx_ptr_in = 0;
		}
	}
	else if (linecoding.datatype == 8)
	{
		while (length)
		{
			USART_Rx_Buffer[USART_Rx_ptr_in] = *data;
			data++;
			length--;
			USART_Rx_ptr_in++;
			if (USART_Rx_ptr_in == USART_RX_DATA_SIZE) USART_Rx_ptr_in = 0;
		}
	}
}

/**
  * @brief  Receive data from virtual serial

	  @params buffer for rtval,limit

	@rtval Length received
  */
u16 VirtualSerial_RecvData(u8 *pdata, u16 limit)
{
	u16 length;

	if (USART_Tx_ptr_in == USART_Tx_ptr_out) return 0;

	if (USART_Tx_ptr_in > USART_Tx_ptr_out)
		length = USART_Tx_ptr_in - USART_Tx_ptr_out;
	else
		length = USART_Tx_ptr_in + USART_TX_DATA_SIZE - USART_Tx_ptr_out;

	if (length < limit)
		length = limit;
	else
		limit = length;

	while (length)
	{
		*pdata = USART_Tx_Buffer[USART_Tx_ptr_out];
		USART_Tx_ptr_out++;
		if (USART_Tx_ptr_out >= USART_TX_DATA_SIZE)
			USART_Tx_ptr_out = 0;
		pdata++;
		length--;
	}

	return limit;
}

/**
  * @brief  Read a byte from virtual serial
  */
bool VirtualSerial_ReadByte(u8* data)
{
 if (USART_Tx_ptr_in == USART_Tx_ptr_out) return false;
 *data=USART_Tx_Buffer[USART_Tx_ptr_out];
 USART_Tx_ptr_out++;
 return true;
}

/**
  * @brief  Send a string to virtual serial
  */
void VirtualSerial_SendString(char* string)
{
	u16 length = GetStringLengthInBytes(string);
	VirtualSerial_SendData((u8*)string, length);
}

/**
  * @brief  This service handles the packet from virtual serial
  */
void VirtualSerial_Packet_Service(void *pvParameters)
{
	u16 offset = 0;
	u16 timeoutCnt = 0;
	bool maxReached = false;
	u8 i;

	for (;;)
	{
		if (VirtualSerial_ReadByte(ReceivedPacket))
		{
			offset++;
      memset(ReceivedPacket+1,0,VIRTUAL_SERIAL_MAX_PACKET_SIZE-1);
			for (;;)
			{
				while (VirtualSerial_ReadByte(ReceivedPacket + offset))
				{
					offset++;
					timeoutCnt = 0;
					if (offset == VIRTUAL_SERIAL_MAX_PACKET_SIZE)
					{
						maxReached = true;
						break;
					}
				}
				if (maxReached) break;
				vTaskDelay(10 / portTICK_RATE_MS);
				timeoutCnt++;
				if (timeoutCnt == VIRTUAL_SERIAL_PACKET_TIMEOUT_10_MS) break;
			}
			maxReached = false;
			timeoutCnt = 0;
			offset = 0;
			xQueueSend(Virtual_Serial_Packet_Received_Msg, &i, 0);
		}
		vTaskDelay(10 / portTICK_RATE_MS);
	}
}

/**
  * @brief  Init VirtualSerial hardware
  */
void VirtualSerial_Hardware_Init()
{
 USB_Init();
}

/**
  * @brief  Init VirtualSerial_Packet_Service
  */
void VirtualSerial_Packet_Service_Init()
{
	Virtual_Serial_Packet_Received_Msg = xQueueCreate(1, sizeof(u8));
	CreateTaskWithExceptionControl(VirtualSerial_Packet_Service, "Virtual_Serial Service",
		128, NULL, VIRTUAL_SERIAL_PACKET_SERVICE_PRIORITY, NULL);
}

void VirtualSerial_Init()
{
 VirtualSerial_Hardware_Init();
 VirtualSerial_Packet_Service_Init();
}
