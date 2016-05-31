#ifndef __SSD1306_H
#define __SSD1306_H			  	

#include "misc.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"  

#define OLED_REFRESH_OPTIMIZE_EN 1

#define SSD1306 0

#define SH1106 1

#define OLED_MODEL SH1106

typedef struct
{
	u8 x;
	u8 y;
}OLED_PositionStruct;


#define OLED_MODE 0

#define OLED_CS_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_12)//CS
#define OLED_CS_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_12)


#define OLED_RS_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_1)//A0
#define OLED_RS_Set() GPIO_SetBits(GPIOB,GPIO_Pin_1)

#define DRAW 1
#define UNDRAW 0

#define OLED_CMD  0	
#define OLED_DATA 1	

extern volatile bool UpdateOLEDJustNow;

extern volatile bool GRAM_Changing;

extern volatile bool GRAM_Changed;

extern volatile bool GRAM_No_Change_Timeout;

extern xSemaphoreHandle OLEDRelatedMutex;

//see .c for details
void OLED_WR_Byte(unsigned char  dat, unsigned char  cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);

//see .c for details
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(unsigned char  x, unsigned char  y, unsigned char  t);
void OLED_Fill(unsigned char  x1, unsigned char  y1, unsigned char  x2, unsigned char  y2, unsigned char  dot);
void OLED_ShowCHinese(unsigned char  x, unsigned char  y, unsigned char  no, unsigned char drawOrUnDraw);
void OLED_ShowChar(unsigned char  x, unsigned char  y, unsigned char  chr, unsigned char  size, unsigned char  mode);
void OLED_ShowNum(unsigned char  x, unsigned char  y, unsigned int num, unsigned char  len, unsigned char  size);
void OLED_ShowString(unsigned char  x, unsigned char  y, const unsigned char  *p);
void LCD_Set_Pos(unsigned char  x, unsigned char  y);

void OLED_ShowSelectionString(unsigned char  x, unsigned char  y, const unsigned char  *p, bool OnSelection, unsigned char size);
void OLED_DrawVerticalLine(unsigned char  x, unsigned char  y1, unsigned char y2, unsigned char  t);
void OLED_DrawHorizonalLine(unsigned char  y, unsigned char  x1, unsigned char x2, unsigned char  t);
void OLED_DrawRect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, u8 drawOrUnDraw);
void OLED_InvertRect(u8 x1, u8 y1, u8 x2, u8 y2);
void OLED_InvertPoint(u8 x, u8 y);
void OLED_FillRect(u8 x1, u8 y1, u8 x2, u8 y2, u8 t);
void OLED_DrawAnyLine(u8 x1, u8 y1, u8 x2, u8 y2, bool drawOrUndraw);
void OLED_Unlimited_Refresh(void);
void OLED_BackupScreen(void);
void OLED_RestoreScreen(void);

#endif  




