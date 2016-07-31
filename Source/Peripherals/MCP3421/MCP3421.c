//File Name   MCP3421.c
//Description MCP3421     

#include <stdbool.h>

#include "stm32f10x_gpio.h"

#include "FreeRTOS_Standard_Include.h"

#include "MCP3421.h"

#define I2C_SDA_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_11)
#define I2C_SDA_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_11)

#define I2C_SCL_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_10)
#define I2C_SCL_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_10)

#define I2C_SDA_READ()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)
#define I2C_SCL_READ()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)

#define MCP3421_UPDATE_SERVICE_PRIORITY tskIDLE_PRIORITY+6

#define VOLTAGE_GAIN 11

USBMeterStruct CurrentMeterData;

xSemaphoreHandle USBMeterState_Mutex = NULL;

void I2C_GPIO_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	
 	GPIO_Init(GPIOB, &GPIO_InitStructure); 
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

bool I2C_Start(void)
{
 I2C_SDA_HIGH();
 I2C_SCL_HIGH();
 I2C_delay();
 if(!I2C_SDA_READ())return false;
 I2C_SDA_LOW();
 I2C_delay();
 if(I2C_SDA_READ()) return false;
 I2C_SDA_LOW();
 I2C_delay();
 return true;
}

void I2C_Ack(void)
{ 
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_LOW();
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 I2C_SCL_LOW();
 I2C_delay();
}

void I2C_Stop(void)
{
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_LOW();
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 I2C_SDA_HIGH();
 I2C_delay();
}

void I2C_NoAck(void)
{ 
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_HIGH();
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 I2C_SCL_LOW();
 I2C_delay();
}

bool I2C_WaitAck(void)  
{
 I2C_SCL_LOW();
 I2C_delay();
 I2C_SDA_HIGH();   
 I2C_delay();
 I2C_SCL_HIGH();
 I2C_delay();
 if(I2C_SDA_READ())
 {
   I2C_SCL_LOW();
   return false;
 }
 I2C_SCL_LOW();
 return true;
}

void I2C_SendByte(u8 SendByte)
{
    u8 i=8;
    while(i--)
    {
       I2C_SCL_LOW();
       I2C_delay();
       if(SendByte&0x80)
         I2C_SDA_HIGH();
       else 
       I2C_SDA_LOW();   
       SendByte<<=1;
       I2C_delay();
       I2C_SCL_HIGH();
       I2C_delay();
    }
    I2C_SCL_LOW();
}

u8 I2C_ReceiveByte(void) 
{ 
    u8 i=8;
    u8 ReceiveByte=0;
    I2C_SDA_HIGH(); 
    while(i--)
    {
      ReceiveByte<<=1;      
      I2C_SCL_LOW();
      I2C_delay();
      I2C_SCL_HIGH();
      I2C_delay(); 
      if(I2C_SDA_READ())
      {
       ReceiveByte|=0x01;
      }
    }
    I2C_SCL_LOW();
    return ReceiveByte;
}

bool I2C_WriteByte(u8 SendByte, u16 WriteAddress, u8 DeviceAddress)
{  
    if(!I2C_Start())return false;
	
    I2C_SendByte( DeviceAddress & 0xFE);
    if(!I2C_WaitAck())
    {
		  I2C_Stop(); 
		  return false;
		}
    I2C_SendByte((u8)((WriteAddress>>8) & 0xFF));      
    I2C_WaitAck(); 
    I2C_SendByte((u8)((WriteAddress) & 0xFF));   
    I2C_WaitAck(); 
    I2C_SendByte(SendByte);         
    I2C_WaitAck();   
    I2C_Stop(); 
    I2C_delay();
    return true;
}

u8 I2C_ReadByte( u16 ReadAddress,  u8 DeviceAddress)
{  
    u8 temp;
    if(!I2C_Start())return false;
	
    I2C_SendByte((DeviceAddress & 0xFE));
    if(!I2C_WaitAck())
    {
		  I2C_Stop(); 
		  return false;
		}
    I2C_SendByte((u8)((ReadAddress>>8) & 0xFF));  
    I2C_WaitAck();
    I2C_SendByte((u8)((ReadAddress) & 0xFF));        
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte((DeviceAddress & 0xFE)|0x01);  
    I2C_WaitAck();
   
    temp = I2C_ReceiveByte();
    
    I2C_NoAck();
     
    I2C_Stop();
    return temp;
}

u32 GetResultFromMCP3421(unsigned char address)
{
	unsigned char byte1, byte2, byte3;
	u32 mcp3421result;
	I2C_Start();
	I2C_SendByte(address);
	I2C_WaitAck();
	byte1=I2C_ReceiveByte();
	I2C_Ack();
	byte2=I2C_ReceiveByte();
	I2C_Ack();
	byte3=I2C_ReceiveByte();
	I2C_NoAck();
	I2C_Stop();
	mcp3421result = byte1;
	mcp3421result = mcp3421result << 8;
	mcp3421result = mcp3421result | byte2;
	mcp3421result = mcp3421result << 8;
	mcp3421result = mcp3421result | byte3;
	return mcp3421result;
}

void WriteMCP3421(u8 dat,u8 add)
{
 I2C_Start();
 I2C_SendByte(add);
 I2C_WaitAck();
 I2C_SendByte(dat);
 I2C_WaitAck();
 I2C_Stop();
}
	
void MCP3421_MeterData_Update_Service(void *pvParameters)
{
 u32 t;
 for(;;)
	{
	 t=GetResultFromMCP3421(0xd1);
	 t=t&0x03ffff;
		
	 if (t&0x020000)
	 {
	  t=0;
	 }
	 
	 xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
	 CurrentMeterData.Voltage=(double)(*(int32_t*)(&t))/0x20000*2.048*VOLTAGE_GAIN;
	 xSemaphoreGive(USBMeterState_Mutex);
	 
	 vTaskDelay(100 / portTICK_RATE_MS);
	}
}

bool MCP3421_Init(void)
{
 I2C_GPIO_Configuration();
	
 /*Voltage sensor configuration*/
 WriteMCP3421(0x9c,0xd0); 
	
 USBMeterState_Mutex = xSemaphoreCreateMutex();
	
 CreateTaskWithExceptionControl(MCP3421_MeterData_Update_Service, "MCP3421_Update_Service",
		128,NULL, MCP3421_UPDATE_SERVICE_PRIORITY, NULL);
 return true;
}
