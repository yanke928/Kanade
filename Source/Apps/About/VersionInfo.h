#ifndef __VERSIONINFO_H
#define	__VERSIONINFO_H

#include "stm32f10x.h"
#include "stdbool.h"
#include "Music.h"

#pragma diag_suppress 870

const char ProductName_EN[]="Kanade";
const char ProductName_TW[]="Kanade";
const char ProductName_CN[]="Kanade";
const char ProductName_JP[]="Kanade";
const char* const ProductName_Str[] = { ProductName_EN,ProductName_TW,ProductName_CN,ProductName_JP };

const char ProductVersion_EN[]="V1.1";
const char ProductVersion_TW[]="V1.1";
const char ProductVersion_CN[]="V1.1";
const char ProductVersion_JP[]="V1.1";
const char* const ProductVersion_Str[] = { ProductVersion_EN,ProductVersion_TW,ProductVersion_CN,ProductVersion_JP };

const char ProductDesigner_EN[]="Design: yanke928";
const char ProductDesigner_TW[]="yanke928設計";
const char ProductDesigner_CN[]="yanke928设计";
const char ProductDesigner_JP[]="yanke928設計";
const char* const ProductDesigner_Str[] = { ProductDesigner_EN,ProductDesigner_TW,ProductDesigner_CN,ProductDesigner_JP };

const char ProductVendor_EN[]="Vendor: yanke928";
const char ProductVendor_TW[]="yanke928供應";
const char ProductVendor_CN[]="yanke928供应";
const char ProductVendor_JP[]="yanke928提供";
const char* const ProductVendor_Str[] = { ProductVendor_EN,ProductVendor_TW,ProductVendor_CN,ProductVendor_JP };

const SingleToneStruct* const ProductSound=Tori_No_Uta;

#endif /* __VERSIONINFO_H */
