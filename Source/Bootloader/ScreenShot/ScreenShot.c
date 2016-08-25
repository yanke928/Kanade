//File Name   ScreenShot.c
//Description Screen shot

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "SSD1306.h"

#include "W25Q64ff.h"

#include "ScreenShot.h"

#define DWORD u32
#define WORD u16 
#define QWORD uint64_t 

#pragma pack(2)  
typedef struct 
{
 char Identifier[2];
 DWORD FileSize;
 DWORD ReservedDWORD;
 DWORD BitMapOffset;
 DWORD BitMapInfoHeaderSize;
 DWORD Width;
 DWORD Hight;
 WORD Planes;
 WORD BitsPerPixel;
 DWORD Compression;
 DWORD BitMapDataSize;
 DWORD HResolution;
 DWORD VResolution;
 DWORD Colors;
 DWORD Important;
 DWORD Palette;
}BitMap_Header_t;

u32 ScreenShotCount=0;

const BitMap_Header_t OLED12864_BitMap_Header=
{
 {'B','M'},//Identifier
 1086,//FileSize
 0,//ReservedDWORD
 0x3e,//BitMapOffSet
 0x28,//BitMapHeaderSize
 128,//Width
 64,//Height
 1,//Planes
 0x01,//BitsPerPixel
 0,//Compression
 1024,//BitMapDataSize
 0,//HResolution
 0,//VResolution
 0,//Colors
 0,//Important
 0,//Palette
};

static bool WriteOLED12864BitMapHeader(FIL* bmp)
{
 u32 byteWritten;
 FRESULT res;
 u32 palette=0x00ffffff; 
 BitMap_Header_t headerOLED12864;
 memcpy(&headerOLED12864,&OLED12864_BitMap_Header,sizeof(BitMap_Header_t));
 res=f_write(bmp,&headerOLED12864,sizeof(BitMap_Header_t),&byteWritten);
 if(res!=FR_OK||byteWritten!=sizeof(BitMap_Header_t)) return false;
 res=f_write(bmp,&palette,sizeof(palette),&byteWritten);
 if(res!=FR_OK||byteWritten!=sizeof(palette)) return false;
 return true;
}

static bool WriteOLED12864BitMap(FIL* bmp)
{
 u8 currentData;
 u32 byteWritten;
 FRESULT res;
 int x=0;
 int y=63;
 u8 writeCnt=0;
 for(;;)
 {
  if(OLED_ReadPoint(x,y))
  {
   currentData=currentData|(0x80>>writeCnt);
  }
  else 
  {
   currentData=currentData&(~(0x80>>writeCnt));
  }
  writeCnt++;
  if(writeCnt==8)
  {
  res=f_write(bmp,&currentData,sizeof(currentData),&byteWritten);
  if(res!=FR_OK||byteWritten!=sizeof(currentData)) return false;
  writeCnt=0;
  }
  x++;
  if(x==128) 
  {
   x=0;
   y--;
  }
  if(y==-1) break;
 }
 return true;
}

void TakeAScreenShot()
{
 	FRESULT res;
  FIL bmp;
  char tempString[20];
  sprintf(tempString,"1:/%d.bmp",ScreenShotCount);
  ScreenShotCount++;
	res = f_open(&bmp, tempString, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) for(;;);
  if (WriteOLED12864BitMapHeader(&bmp) != true)
  for(;;);
  if (WriteOLED12864BitMap(&bmp) != true)
  for(;;);
  f_close(&bmp);
}

