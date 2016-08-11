//File Name     DataPin2Protocol.c
//Description : Data pin voltage to protocol

#include <stdio.h>

#include "MCP3421.h"

#include "DataPin2Protocol.h"

#define DATAPIN2PROTOCOL_PRIORITY tskIDLE_PRIORITY+3

#define _ABS(x) ((x) > 0 ? (x) : -(x))

#define NOT_NECCESSARY 10

const ProtocolDescriptor ProtocolTab[]=
{
"QC-9V",{3.1,3.5},{0.3,1},NOT_NECCESSARY,{8,10},
"QC-12V",{0.3,1},{0.3,1},NOT_NECCESSARY,{11,13},
"QC-20V",{3.1,3.5},{3.1,3.5},NOT_NECCESSARY,{19,21},
"QC-5V",{0.4,1},{-0.1,0.1},NOT_NECCESSARY,{4,6},
"Apple0.5",{1.85,2.15},{1.85,2.15},NOT_NECCESSARY,{4.5,5.5},
"Apple-1A",{1.85,2.15},{2.55,2.85},NOT_NECCESSARY,{4.5,5.5},
"Apple-2A",{2.55,2.85},{1.85,2.15},NOT_NECCESSARY,{4.5,5.5},
"Apple2.4",{2.55,2.85},{2.55,2.85},NOT_NECCESSARY,{4.5,5.5},
"USB-DCP",{0,3.3},{0,3.3},0.015,{4.5,5.5},
"USB-DCP",{2.6,2.8},{2.6,2.8},0.03,{4.5,5.5},
"USB-DCP",{1.1,1.3},{1.1,1.3},0.2,{4.5,5.5},
"MTK-7V",{0,3.3},{0,3.3},NOT_NECCESSARY,{6.5,7.5},
};

u8 PossibleProtocolTab[10];

u8 PossibleProtocolNum;

void Protocol_Recognize_Service(void *pvParameters)
{
 u8 i;
 u8 p;
 for(;;)
  { 
   p=0;
   for(i=0;i<sizeof(ProtocolTab)/sizeof(ProtocolDescriptor);i++)
     {
      if(ProtocolTab[i].DPMinAndMax[0]<CurrentMeterData.VoltageDP&&
         ProtocolTab[i].DPMinAndMax[1]>CurrentMeterData.VoltageDP&&
         ProtocolTab[i].DMMinAndMax[0]<CurrentMeterData.VoltageDM&&
         ProtocolTab[i].DMMinAndMax[1]>CurrentMeterData.VoltageDM&&
         ProtocolTab[i].MainVoltMinAndMax[0]<CurrentMeterData.Voltage&&
         ProtocolTab[i].MainVoltMinAndMax[1]>CurrentMeterData.Voltage&&
         _ABS(CurrentMeterData.VoltageDM-CurrentMeterData.VoltageDP)<ProtocolTab[i].MaxDeltaV)
       {
        PossibleProtocolTab[p]=i;
        p++;
       }
     }
   PossibleProtocolNum=p;
   vTaskDelay(200/portTICK_RATE_MS);
  }
}

void DataPin2Protocol_Service_Init()
{
	CreateTaskWithExceptionControl(Protocol_Recognize_Service, "DataPin2Protocol_Service",
		96, NULL, DATAPIN2PROTOCOL_PRIORITY, NULL);
}
