//File Name   moha_SerialProtoocl.c
//Description moha serial stream protocol

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "stm32f10x.h"

#include "VirtualSerial.h"

#include "moha_SerialProtocol.h"

typedef struct
{
	u16 PacketType;
	u16 NumOfParameters;
}moha_SerialProtocol_Packet_Descriptor_t;

typedef struct
{
	u16* ParameterTypes;
	u16* ParameterLengths;
	void** Parameters;
}moha_SerialProtocol_Packet_t;

#define CurrentAddr backPacket+addr

u32 moha_GeneratePacket(const moha_SerialProtocol_Packet_Descriptor_t* descriptor, const moha_SerialProtocol_Packet_t* packet,
	u8* backPacket)
{
	u32 addr = 0;
	u32 i;
  memcpy(CurrentAddr, &(descriptor->PacketType),sizeof(descriptor->PacketType));
  addr = addr + 2;
  memcpy(CurrentAddr, &(descriptor->NumOfParameters),sizeof(descriptor->NumOfParameters));
  addr = addr + 2;
	for (i = 0; i < descriptor->NumOfParameters; i++)
	{
    memcpy(CurrentAddr, &(packet->ParameterTypes[i]),sizeof(u16));
		addr = addr + 2;
		memcpy(CurrentAddr, &(packet->ParameterLengths[i]),sizeof(u16));
		addr = addr + 2;
		memcpy(CurrentAddr, packet->Parameters[i], packet->ParameterLengths[i]);
		addr = addr + packet->ParameterLengths[i];
	}
	return addr;
}

u32 testMohaPacket()
{
 u32 length;
 u8 generatedPacket[200];
 char parameter0[]="param0";
 char parameter1[]="param1";
 u16 parameterTypes[2]={0xb6b6,0x9f9f};
 u16 ParameterLengths[2]={7,7};

 moha_SerialProtocol_Packet_Descriptor_t test_Descriptor={0xa5a5,0x02};
 moha_SerialProtocol_Packet_t test_packet;

 void* parameters[2];

 parameters[0]=parameter0;
 parameters[1]=parameter1;

 test_packet.ParameterTypes=parameterTypes;
 test_packet.ParameterLengths=ParameterLengths;
 test_packet.Parameters=parameters;

 length=moha_GeneratePacket(&test_Descriptor,&test_packet,generatedPacket);

// sprintf((char*)generatedPacket,"%d",length);
// ShowSmallDialogue((char*)generatedPacket,1000,true);
 VirtualSerial_SendData(generatedPacket,length);
 return 0;

}
