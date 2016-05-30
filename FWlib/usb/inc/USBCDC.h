//USBCDC for STM32
#ifndef _USBCDC_H__
#define _USBCDC_H__

#include "platform.h"
#include <stdbool.h>

void USBCDC_Init(void); //初始化USBCDC虚拟串口 

bool USBCDC_ready(void);//获取串口状态,TRUE表示可以通讯 

void USBCDC_connect(void);//打开USB连接
void USBCDC_disconnect(void);//断开USB连接

void USBCDC_senddata(uint8 *pdata,uint16 length);//发数据
uint16 USBCDC_recvdata(uint8 *pdata, uint16 limit);//收数据,返回实际接收到的数据长度
//limit是pdata区的容量
void USBCDC_sync(void);//等待传送完成 

void __USBCDC_ISR(void);//USB中断发生时调用
//在usb_istr.c中实现

void USBCDC_SendString(char string[]);

#endif
