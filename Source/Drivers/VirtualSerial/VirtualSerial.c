//File Name   VirtualSerial.c
//Description VirtualSerial driver layer

#include <stdbool.h>

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_conf.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"

#include "UI_Utilities.h"

#include "VirtualSerial.h"

extern uint8_t  USB_Tx_State; 

void VirtualSerial_SendData(u8 *data,u16 length)
{
    if(linecoding.datatype == 7)
    {
        while(length)
        {
            USART_Rx_Buffer[USART_Rx_ptr_in] = *data & 0x7F;
            data++;
            length--;
            USART_Rx_ptr_in++;
            if(USART_Rx_ptr_in == USART_RX_DATA_SIZE) USART_Rx_ptr_in = 0;
        }
    }
    else if(linecoding.datatype == 8)
    {
        while(length)
        {
            USART_Rx_Buffer[USART_Rx_ptr_in] = *data;
            data++;
            length--;
            USART_Rx_ptr_in++;
            if(USART_Rx_ptr_in == USART_RX_DATA_SIZE) USART_Rx_ptr_in = 0;
        }
    }
}

u16 VirtualSerial_RecvData(u8 *pdata,u16 limit)
{
	u16 length;

	if(USART_Tx_ptr_in == USART_Tx_ptr_out) return 0;

	if(USART_Tx_ptr_in > USART_Tx_ptr_out)
		length = USART_Tx_ptr_in - USART_Tx_ptr_out;
  	else
  	 	length = USART_Tx_ptr_in + USART_TX_DATA_SIZE - USART_Tx_ptr_out;

	if(length < limit)
		length = limit;
    else
        limit = length;//use limit to save retval
    
	while(length)
	{
		*pdata = USART_Tx_Buffer[USART_Tx_ptr_out];
		USART_Tx_ptr_out++;
		if(USART_Tx_ptr_out >= USART_TX_DATA_SIZE) 
            USART_Tx_ptr_out = 0;
		pdata++;
		length--;
	}

	return limit;
}

void VirtualSerial_SendString(char* string)
{
 u16 length=GetStringLengthInBytes(string);
 VirtualSerial_SendData((u8*)string,length);
}
