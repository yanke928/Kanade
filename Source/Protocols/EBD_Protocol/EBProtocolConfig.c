//File Name     EBProtocolConfig.c
//Description : EB protocol configurations

#include "stdbool.h"

#include "EBProtocolConfig.h"

const EBD_Protocol_Config_Struct EBD_USB_V1 =
{
 1000,
 false,
 1,
 3000,
 24000,
 1
};

const char EBD_USB_V1_Descriptor[] = "EBD-USB V1";

const EBD_Protocol_Config_Struct EBD_USB_V2 =
{
 1000,
 false,
 2,
 3000,
 24000,
 1
};

const char EBD_USB_V2_Descriptor[] = "EBD-USB V2";

const EBD_Protocol_Config_Struct EBD_USB_Plus =
{
 10000,
 true,
 2,
 5000,
 50000,
 1
};

const char EBD_USB_Plus_Descriptor[] = "EBD-USB Plus";

const EBD_Protocol_Config_Struct EBD_USB_Plus_Plus=
{
 5000,
 true,
 2,
 5000,
 90000,
 0.5
};

const char EBD_USB_Plus_Plus_Descriptor[] = "EBD-USB Plus+";

const EBD_Protocol_Config_Struct* const EBD_Protocol_Config[EBD_MODEL_NUM] =
{ &EBD_USB_V1,&EBD_USB_V2,&EBD_USB_Plus,&EBD_USB_Plus_Plus};

const char* const EBD_Protocol_Descriptors[EBD_MODEL_NUM] =
{ EBD_USB_V1_Descriptor,EBD_USB_V2_Descriptor,EBD_USB_Plus_Descriptor,EBD_USB_Plus_Plus_Descriptor};

