//File Name   SerialControl.c
//Description Serial control app

#include <string.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "MCP3421.h"
#include "VirtualSerial.h"

#include "SerialControl.h"

#define VIRTUAL_SERIAL_UPLOAD_SERVICE_PRIORITY tskIDLE_PRIORITY+7

u8 EBD_Report_Packet[]=
{
 0xfa,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x28,0x00,0x50,0x00,0x00,0x1a,0x66,0xf8
};

void Virtual_Serial_Upload_Service(void *pvParameters)
{
 char tempString[50];
 for(;;)
 {
  sprintf(tempString,"%.4f,%.4f\n",FilteredMeterData.Voltage,FilteredMeterData.Current);
  testMohaPacket();
  vTaskDelay(500/portTICK_RATE_MS);
 }
}

void Virtual_Serial_Upload_Service_Init()
{
	CreateTaskWithExceptionControl(Virtual_Serial_Upload_Service, "Virtual_Serial_Upload Service",
		160, NULL, VIRTUAL_SERIAL_UPLOAD_SERVICE_PRIORITY, NULL);
}

//void SerialControlMode()
//{
// u8 i;
// u8 tmp[]={0xf8};
// while(xQueueReceive(Virtual_Serial_Packet_Received_Msg,&i,10/portTICK_RATE_MS)!=pdPASS);
// for(;;)
//  {
//   for(i=0;i<19;i++)
//   {
//    VirtualSerial_SendData(EBD_Report_Packet+i,1);
//    vTaskDelay(1/portTICK_RATE_MS);
//   }
//   vTaskDelay(1000/portTICK_RATE_MS);
//  }
//} 
