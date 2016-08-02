//File Name   MCP3421.c
//Description MCP3421     

#include <stdbool.h>

#include "stm32f10x_gpio.h"

#include "FreeRTOS_Standard_Include.h"

#include "MCP3421.h"

#define I2C1_SDA_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_11)
#define I2C1_SDA_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_11)

#define I2C2_SDA_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_3)
#define I2C2_SDA_LOW()  GPIO_ResetBits(GPIOA, GPIO_Pin_3)

#define I2C_SCL_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_10)
#define I2C_SCL_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_10)

#define I2C1_SDA_READ()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)
#define I2C2_SDA_READ()  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)
#define I2C_SCL_READ()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)

#define MCP3421_UPDATE_SERVICE_PRIORITY tskIDLE_PRIORITY+6

#define VOLTAGE_GAIN 11
#define CURRENT_SENSOR_RESISTANCE 15

USBMeterStruct CurrentMeterData;

xSemaphoreHandle USBMeterState_Mutex = NULL;

xQueueHandle MCP3421_Current_Updated_Msg;

enum{I2C_1,I2C_2};

void I2C_GPIO_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	
 	GPIO_Init(GPIOB, &GPIO_InitStructure); 
}

void I2C_SDA_HIGH(u8 num)
{
 switch(num)
 {
  case I2C_1:I2C1_SDA_HIGH();break;
  case I2C_2:I2C2_SDA_HIGH();break;
 }
}

void I2C_SDA_LOW(u8 num)
{
 switch(num)
 {
  case I2C_1:I2C1_SDA_LOW();break;
  case I2C_2:I2C2_SDA_LOW();break;
 }
}

u8 I2C_SDA_READ(u8 num)
{ 
 u8 status;
 switch(num)
 {
  case I2C_1:status=I2C1_SDA_READ();break;
  case I2C_2:status=I2C2_SDA_READ();break;
 }
 return status;
}

void Delayxus(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=10; 
      while(i--) ;    
   }
}
#define I2C_delay() Delayxus(1)

bool I2C_Start(u8 num)
{
 I2C_SDA_HIGH(num);
 I2C_SCL_HIGH();
 I2C_delay(); 
 if(!I2C_SDA_READ(num))return false;
 I2C_SDA_LOW(num);
 I2C_delay(); 
 if(I2C_SDA_READ(num))return false;
 I2C_SDA_LOW(num);
 I2C_delay();
 return true;
}

void I2C_Ack(u8 num)
{ 
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_LOW(num);
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 I2C_SCL_LOW();
 I2C_delay();
}

void I2C_Stop(u8 num)
{
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_LOW(num);
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 I2C_SDA_HIGH(num);
 I2C_delay();
}

void I2C_NoAck(u8 num)
{ 
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_HIGH(num);
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 I2C_SCL_LOW();
 I2C_delay();
}

bool I2C_WaitAck(u8 num)  
{
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_HIGH(num);
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 if(I2C_SDA_READ(num))
 {
   I2C_SCL_LOW();
   return false;
 }
 I2C_SCL_LOW();
 return true;
}

void I2C_SendByte(u8 SendByte,u8 num)
{
    u8 i=8;
    while(i--)
    {
       I2C_SCL_LOW();
       I2C_delay();
       if(SendByte&0x80)
         I2C_SDA_HIGH(num);
       else 
       I2C_SDA_LOW(num);   
       SendByte<<=1;
       I2C_delay();
       I2C_SCL_HIGH();
       I2C_delay();
    }
    I2C_SCL_LOW();
}

u8 I2C_ReceiveByte(u8 num) 
{ 
    u8 i=8;
    u8 ReceiveByte=0;
    I2C_SDA_HIGH(num); 
    while(i--)
    {
      ReceiveByte<<=1;      
      I2C_SCL_LOW();
      I2C_delay();
      I2C_SCL_HIGH();
      I2C_delay(); 
      if(I2C_SDA_READ(num))
      {
       ReceiveByte|=0x01;
      }
    }
    I2C_SCL_LOW();
    return ReceiveByte;
}

bool I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress,u8 num)
{  
    if(!I2C_Start(num))return false;
	
    I2C_SendByte( DeviceAddress & 0xFE,num);
    if(!I2C_WaitAck(num))
    {
		  I2C_Stop(num); 
		  return false;
		}
    I2C_SendByte((u8)((WriteAddress>>8) & 0xFF),num);      
    I2C_WaitAck(num); 
    I2C_SendByte((u8)((WriteAddress) & 0xFF),num);   
    I2C_WaitAck(num); 
    I2C_SendByte(SendByte,num);         
    I2C_WaitAck(num);   
    I2C_Stop(num); 
    I2C_delay();
    return true;
}

u8 I2C_ReadByte( u16 ReadAddress,  u8 DeviceAddress,u8 num)
{  
    u8 temp;
    if(!I2C_Start(num))return false;
	
    I2C_SendByte((DeviceAddress & 0xFE),num);
    if(!I2C_WaitAck(num))
    {
		  I2C_Stop(num); 
		  return false;
		}
    I2C_SendByte((u8)((ReadAddress>>8) & 0xFF),num);  
    I2C_WaitAck(num);
    I2C_SendByte((u8)((ReadAddress) & 0xFF),num);        
    I2C_WaitAck(num);
    I2C_Start(num);
    I2C_SendByte((DeviceAddress & 0xFE)|0x01,num);  
    I2C_WaitAck(num);
   
    temp = I2C_ReceiveByte(num);
    
    I2C_NoAck(num);
     
    I2C_Stop(num);
    return temp;
}

u32 GetResultFromMCP3421(unsigned char address,u8 num)
{
	unsigned char byte1, byte2, byte3;
	u32 mcp3421result;
	I2C_Start(num);
	I2C_SendByte(address,num);
	I2C_WaitAck(num);
	byte1=I2C_ReceiveByte(num);
	I2C_Ack(num);
	byte2=I2C_ReceiveByte(num);
	I2C_Ack(num);
	byte3=I2C_ReceiveByte(num);
	I2C_NoAck(num);
	I2C_Stop(num);
	mcp3421result = byte1;
	mcp3421result = mcp3421result << 8;
	mcp3421result = mcp3421result | byte2;
	mcp3421result = mcp3421result << 8;
	mcp3421result = mcp3421result | byte3;
	return mcp3421result;
}

void WriteMCP3421(u8 dat,u8 add,u8 num)
{
 I2C_Start(num);
 I2C_SendByte(add,num);
 I2C_WaitAck(num);
 I2C_SendByte(dat,num);
 I2C_WaitAck(num);
 I2C_Stop(num);
}
	
void MCP3421_MeterData_Update_Service(void *pvParameters)
{
 u32 voltRAW,curtRAW; 
 u32 lstCurrent;
 u8 m;
 for(;;)
	{
	 voltRAW=GetResultFromMCP3421(0xd1,I2C_2);
	 voltRAW=voltRAW&0x03ffff;
		
	 if (voltRAW&0x020000)
	 {
	  voltRAW=0;
	 }

	 curtRAW=GetResultFromMCP3421(0xd1,I2C_1);
	 curtRAW=curtRAW&0x03ffff;
		
	 if (curtRAW&0x020000)
	 {
	  curtRAW=0;
	 }
	 
	 xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
	 CurrentMeterData.Current=(double)(*(int32_t*)(&curtRAW))/(0x20000/2048*8)/CURRENT_SENSOR_RESISTANCE;
	 CurrentMeterData.Voltage=(double)(*(int32_t*)(&voltRAW))/0x20000*2.048*VOLTAGE_GAIN;
   CurrentMeterData.Power=CurrentMeterData.Current* CurrentMeterData.Voltage;
	 xSemaphoreGive(USBMeterState_Mutex);

   if(curtRAW!=lstCurrent)
   {
     xQueueSend(MCP3421_Current_Updated_Msg,&m, 0);
     lstCurrent=curtRAW;
   }

	 vTaskDelay(100 / portTICK_RATE_MS);
	}
}

bool MCP3421_Init(void)
{
 I2C_GPIO_Configuration();

 /*Current sensor configuration*/
 WriteMCP3421(0x9f,0xd0,I2C_1); 
	
 /*Voltage sensor configuration*/
 WriteMCP3421(0x9c,0xd0,I2C_2); 
	
 USBMeterState_Mutex = xSemaphoreCreateMutex();
	
 MCP3421_Current_Updated_Msg=xQueueCreate(1,sizeof(u8));
 CreateTaskWithExceptionControl(MCP3421_MeterData_Update_Service, "MCP3421_Update_Service",
		128,NULL, MCP3421_UPDATE_SERVICE_PRIORITY, NULL);
 return true;
}
