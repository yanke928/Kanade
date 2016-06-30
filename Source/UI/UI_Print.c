//File Name     UI_Print.c
//Description : UI Print 

#include <string.h>
#include <stdio.h>

#include "SSD1306.h" 

#include "UI_Utilities.h" 
#include "UI_Print.h" 

u16 FindSplitAddr(u8 xLengthAvailable,const char *string,u16 startAddr,u8 fontWidth)
{
 int nbrOfGraphicalCharMax=xLengthAvailable/fontWidth;
 u8 offset=startAddr;
 while(nbrOfGraphicalCharMax--)
 {
  if((*(string+offset))&0x80)
	{
	 nbrOfGraphicalCharMax--;
	 offset+=3;
	}
	else if(*(string+offset)==0)
	{
	 return(offset);
	}
	else
	{
	 offset++;
	}
	if(nbrOfGraphicalCharMax<0)
	{
	 return(offset-3);
	}
 }
 return(offset);
}

void UI_PrintMultiLineString(u8 x1,u8 y1,u8 x2,u8 y2,const char *string,bool select,u8 fontSize)
{
 char tmpChar;
 char tempString[200];
 u16 offset=0;
 u16 lastOffset=0;
 u8 nbrOfLine=0;
 u8 fontWidth;
 u8 xDisplayArea=x2-x1+1;
 strcpy(tempString,string);
 switch(fontSize)
 {
	 case 8:fontWidth=6;break;
	 case 12:fontWidth=6;break;
	 case 16:fontWidth=8;
 }
 for(;;)
 {
	offset=FindSplitAddr(xDisplayArea,tempString,offset,fontWidth);
	tmpChar=*(tempString+offset);
	*(tempString+offset)=0; 
  OLED_ShowAnyString(x1,y1+nbrOfLine*fontSize,tempString+lastOffset,NotOnSelect,fontSize);
	if(tmpChar==0) break;
  lastOffset=offset;
  *(tempString+offset)=tmpChar;
	nbrOfLine++;
	if(y1+fontSize*nbrOfLine>y2)break;
 }
 if(select)
 {
  OLED_InvertRect(x1,y1,x2,y2);
 }
}


