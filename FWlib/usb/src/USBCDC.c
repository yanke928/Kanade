//USBCDC for STM32


#include <stdio.h>

#include "USBCDC.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_conf.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "startup.h"

extern LINE_CODING linecoding;

void USBCDC_Init(void)
{
    Set_GPIO();
    Set_USBClock(); 
    USB_Interrupts_Config();
    USB_Init();
}

void USBCDC_connect(void)
{
    PowerOn(); 
} 

void USBCDC_disconnect(void)
{
    PowerOff();
}

bool USBCDC_ready(void)
{
    if(bDeviceState == CONFIGURED) return true;
    else return false;
}

/*synchronize between USB host and device*/
void USBCDC_sync(void)
{
	if(bDeviceState != CONFIGURED) return;
	//When USB is disconnected, ignore the sync operation
	while(USART_Rx_ptr_in != USART_Rx_ptr_out);
	//wait for transmit complete
}

/*USB CDC virtual COM port send data */
void USBCDC_senddata(uint8 *data,uint16 length)
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

void USBCDC_putc(u8 chr)
{
 USART_Rx_Buffer[USART_Rx_ptr_in] = chr;
 USART_Rx_ptr_in++;
 if(USART_Rx_ptr_in == USART_RX_DATA_SIZE) USART_Rx_ptr_in = 0;
}

u8 USBCDC_getc(u8* chr)
{
 if(USART_Tx_ptr_in == USART_Tx_ptr_out) return 0;
 *chr = USART_Tx_Buffer[USART_Tx_ptr_out];
 USART_Tx_ptr_out++;
 return 1;
}

/*USB CDC virtual COM port receive data */
uint16 USBCDC_recvdata(uint8 *pdata,uint16 limit)
{
	uint16 length;

	if(USART_Tx_ptr_in == USART_Tx_ptr_out) return 0;
	//无数据则不接收

	if(USART_Tx_ptr_in > USART_Tx_ptr_out)
		length = USART_Tx_ptr_in - USART_Tx_ptr_out;
  	else
  	 	length = USART_Tx_ptr_in + USART_TX_DATA_SIZE - USART_Tx_ptr_out;
	//计算数据长度

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
	//逐个字节写入

	return limit;
}

void USBCDC_SendString(char string[])
{
 u8 length=strlen(string);
 USBCDC_senddata((u8*)string,length);
}



