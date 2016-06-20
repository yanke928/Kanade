#ifndef __VERSIONINFO_H
#define	__VERSIONINFO_H

#include "stm32f10x.h"
#include "stdbool.h"
#include "Music.h"

#pragma diag_suppress 870

const char ProductName_EN[]="Kanade";
const char ProductName_TW[]="Kanade";
const char ProductName_CN[]="Kanade";
const char ProductName_JP[]="かなで";
const char* const ProductName_Str[] = { ProductName_EN,ProductName_TW,ProductName_CN,ProductName_JP };

const char ProductVersion_EN[]="V1.1";
const char ProductVersion_TW[]="V1.1";
const char ProductVersion_CN[]="V1.1";
const char ProductVersion_JP[]="V1.1";
const char* const ProductVersion_Str[] = { ProductVersion_EN,ProductVersion_TW,ProductVersion_CN,ProductVersion_JP };

const SingleToneStruct* const ProductSound=Tori_No_Uta;

#endif /* __VERSIONINFO_H */
