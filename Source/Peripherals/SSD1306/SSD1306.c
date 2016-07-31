//File Name   SSD1306.c
//Description SSD1306 driver and basic GUI functions

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "stm32f10x_spi.h"

#include "FreeRTOS_Standard_Include.h"

#include "Public_FontTab.h"
#include "SSD1306Font.h"  

#include "Settings.h"

#include "SSD1306.h"

#define OLED_REFRESH_PRIORITY tskIDLE_PRIORITY+2

#define English 0


xSemaphoreHandle OLEDRelatedMutex = NULL;

//#include "delay.h"

//GRAM of OLED
//Save with the format as.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   
unsigned char  OLED_GRAM[128][8];

unsigned char  OLED_GRAM_Backup[128][8];

volatile bool GRAM_Changing = false;

volatile bool GRAM_Changed = false;

volatile bool GRAM_No_Change_Timeout = false;

volatile bool UpdateOLEDJustNow = false;

volatile bool UpdateOLEDJustNow_Backup = false;

/**
  * @brief  Read/Write single byte to/from SPI2

	  @retval Received data
  */
unsigned char SPI2_ReadWriteByte(unsigned char TxData)
{
	unsigned int retry = 0;
	while ((SPI2->SR & 1 << 1) == 0)
	{
		retry++;
		if (retry > 2000)return 0;
	}
	SPI2->DR = TxData;
	retry = 0;
	while ((SPI2->SR & 1 << 0) == 0)
	{
		retry++;
		if (retry > 2000)return 0;
	}
	return SPI2->DR;
}


/**
  * @brief  Upload GRAM to OLED immediately

	  @retval None
  */
void OLED_Refresh_Gram(void)
{
	unsigned char  i, n;
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD);
#if OLED_MODEL
		OLED_WR_Byte(0x02, OLED_CMD);
#else
		OLED_WR_Byte(0x00, OLED_CMD);
#endif
		OLED_WR_Byte(0x10, OLED_CMD);
		for (n = 0; n < 128; n++)OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
	}
}

/**
  * @brief  Write a byte to OLED

	@params dat: 8-bit data being transferred

			cmd: Command or data select

	  @retval None
  */
void OLED_WR_Byte(unsigned char  dat, unsigned char  cmd)
{
	if (cmd)
		OLED_RS_Set();
	else
		OLED_RS_Clr();
	OLED_CS_Clr();

	SPI2_ReadWriteByte(dat);

	OLED_CS_Set();
	OLED_RS_Set();
}

/**
  * @brief  Turn on display

	  @retval None
  */
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //Set DCDC Command
	OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}

/**
  * @brief  Turn off display

	  @retval None
  */
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC
	OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}

/**
  * @brief  Clear whole screen

	  @retval None
  */
void OLED_Clear(void)
{
	unsigned char  i, n;
	for (i = 0; i < 8; i++)for (n = 0; n < 128; n++)OLED_GRAM[n][i] = 0X00;
#if OLED_REFRESH_OPTIMIZE_EN
	if (UpdateOLEDJustNow)
		OLED_Refresh_Gram();//Upload GRAM to screen
#else
	OLED_Refresh_Gram();
#endif
}

/**
  * @brief  Set position

	@param  x(from 0 to 127): X position

			y(from 0 to 63) : Y position

	  @retval None
  */
void LCD_Set_Pos(unsigned char  x, unsigned char  y)
{
#if OLED_MODEL
	x = x + 2;
#endif
	OLED_WR_Byte(0xb0 + (7 - y / 8), OLED_CMD);
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
	OLED_WR_Byte((x & 0x0f), OLED_CMD);
}

/**
  * @brief  Draw/unDraw a single point

	@param  x(from 0 to 127): X position

			y(from 0 to 63) : Y position

			t:Draw(1) or unDraw(0)

	  @retval None
  */
void OLED_DrawPoint(u8  x, u8  y, u8  t)
{
	u8  pos, bx, temp = 0;
#if OLED_REFRESH_OPTIMIZE_EN
	if (UpdateOLEDJustNow == true)
		LCD_Set_Pos(x, y);
#else
	LCD_Set_Pos(x, y);
#endif
	if (x > 127 || y > 63)return;//Out of range
	pos = 7 - y / 8;
	bx = y % 8;
	temp = 1 << (7 - bx);

	if (t)OLED_GRAM[x][pos] |= temp;
	else OLED_GRAM[x][pos] &= ~temp;
#if OLED_REFRESH_OPTIMIZE_EN	
	if (UpdateOLEDJustNow == true)
		OLED_WR_Byte(OLED_GRAM[x][pos], OLED_DATA);
	else
	{
		GRAM_Changing = true;
		GRAM_Changed = true;
	}
#else
	OLED_WR_Byte(OLED_GRAM[x][pos], OLED_DATA);
#endif
}

/**
  * @brief  Invert a pixel

	@param  x(from 0 to 127): X position

			y(from 0 to 63) : Y position

	  @retval None
  */
void OLED_InvertPoint(u8 x, u8 y)
{
	unsigned char  pos, bx, temp = 0;
#if OLED_REFRESH_OPTIMIZE_EN
	if (UpdateOLEDJustNow == true)
		LCD_Set_Pos(x, y);
#else
	LCD_Set_Pos(x, y);
#endif
	if (x > 127 || y > 63)return;//Out of range
	pos = 7 - y / 8;
	bx = y % 8;
	temp = 1 << (7 - bx);
	if ((OLED_GRAM[x][pos] & temp) > 0)
	{
		OLED_GRAM[x][pos] &= ~temp;
	}
	else OLED_GRAM[x][pos] |= temp;
#if OLED_REFRESH_OPTIMIZE_EN	
	if (UpdateOLEDJustNow == true)
		OLED_WR_Byte(OLED_GRAM[x][pos], OLED_DATA);
	else
	{
		GRAM_Changing = true;
		GRAM_Changed = true;
	}
#else
	OLED_WR_Byte(OLED_GRAM[x][pos], OLED_DATA);
#endif
}

/**
  * @brief  Draw a vertical line

	@param  x(from 0 to 127): Public X position

			y1(from 0 to 63): Start of line

			y2(from 0 to 63): End of line

			t:Draw(1) or unDraw(0)

			y1,y2 satisfies y1<y2

	  @retval None
  */
void OLED_DrawVerticalLine(unsigned char  x, unsigned char  y1, unsigned char y2, unsigned char  t)
{
	for (; y1 < y2 + 1; y1++)
	{
		OLED_DrawPoint(x, y1, t);
	}
}

/**
  * @brief  Draw a horizonal line

	@param  y(from 0 to 127): Public Y position

			x1(from 0 to 63): Start of line

			x2(from 0 to 63): End of line

			t:Draw(1) or unDraw(0)

			x1,x2 satisfies x1<x2

	  @retval None
  */
void OLED_DrawHorizonalLine(unsigned char  y, unsigned char  x1, unsigned char x2, unsigned char  t)
{
	for (; x1 < x2 + 1; x1++)
	{
		OLED_DrawPoint(x1, y, t);
	}
}

/**
  * @brief  Draw a rectangle

	@param  x1,y1:Position1

			x2,y2:Position2

			drawOrUnDraw:Draw(1) or unDraw(1)

			x1,y1,x2,y2 satisfies x1<x2 y1<y2

	  @retval None
  */
void OLED_DrawRect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, u8 drawOrUnDraw)
{
	OLED_DrawHorizonalLine(y1, x1, x2, drawOrUnDraw);
	OLED_DrawHorizonalLine(y2, x1, x2, drawOrUnDraw);
	OLED_DrawVerticalLine(x1, y1, y2, drawOrUnDraw);
	OLED_DrawVerticalLine(x2, y1, y2, drawOrUnDraw);
}

/**
  * @brief  Show a char withgiven size

	@param  x,y:Position

			chr:Char

			size:Char size in y 16,12,8 are selectable

			mode:Draw(1) or unDraw(0)

	  @retval None
  */
void OLED_ShowChar(unsigned char  x, unsigned char  y, unsigned char  chr, unsigned char  size, unsigned char  mode)
{
	unsigned char  temp, t, t1, m;
	unsigned char  y0 = y;
	if (chr == '\n' || chr == '\r') return;
	chr = chr - ' ';
	if (size == 8)
	{
		m = 5;
		for (t = 0; t < 5; t++)
		{
			temp = oled_asc2_0705[chr][t];
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x01)OLED_DrawPoint(x, y, mode);
				else OLED_DrawPoint(x, y, !mode);
				temp >>= 1;
				y++;
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					break;
				}
			}
		}
	}
	else
	{
		m = size;
		for (t = 0; t < m; t++)
		{
			if (size == 12)temp = oled_asc2_1206[chr][t]; //1206
			else temp = oled_asc2_1608[chr][t];//1608
			for (t1 = 0; t1 < 8; t1++)
			{
				if (temp & 0x80)OLED_DrawPoint(x, y, mode);
				else OLED_DrawPoint(x, y, !mode);
				temp <<= 1;
				y++;
				if ((y - y0) == size)
				{
					y = y0;
					x++;
					break;
				}
			}
		}
	}
}

/**
  * @brief  Show a Chinese char or a 16*16 icon

	@param  x,y:Position

			no:Icon No

			drawOrUnDraw:Draw(1) or unDraw(0)

	  @retval None
  */
void OLED_ShowIcon(unsigned char  x, unsigned char  y, unsigned char  no, unsigned char drawOrUnDraw)
{
	unsigned char  temp, t, t1;
	unsigned char  y0 = y;
	for (t = 0; t < 32; t++)
	{

		temp = Icons[no][t];
		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)OLED_DrawPoint(x, y, drawOrUnDraw);
			else OLED_DrawPoint(x, y, !drawOrUnDraw);
			temp <<= 1;
			y++;
			if ((y - y0) == 16)
			{
				y = y0;
				x++;
				break;
			}
		}
	}
}

/**
  * @brief  Power function

	  @retval Result
  */
unsigned int oled_pow(unsigned char  m, unsigned char  n)
{
	unsigned int result = 1;
	while (n--)result *= m;
	return result;
}


/**
  * @brief  Show string in 1608

	@param  x,y:Position

			*p:Target string

	  @retval None
  */
void OLED_ShowString(unsigned char  x, unsigned char  y, const char  *p)
{
#define MAX_CHAR_POSX 127
#define MAX_CHAR_POSY 63         
	while (*p != '\0')
	{
		if (x > MAX_CHAR_POSX) { x = 0; y += 16; }
		if (y > MAX_CHAR_POSY) { y = x = 0; OLED_Clear(); }
		OLED_ShowChar(x, y, *p, 16, 1);
		x += 8;
		p++;
	}
}

void OLED_ShowNotASCChar(unsigned char  x, unsigned char  y, char *chr, unsigned char  size, unsigned char  mode)
{
	u16 addr;
	unsigned char  temp, t, t1, m;
	unsigned char  y0 = y;
	addr = GetUTF8IndexInFontTab((s8 *)chr, size);
	if (size == 12) m = 24;
	else if (size == 16) m = 32;
	else m = 0;
	temp = 255;
	for (t = 0; t < m; t++)
	{
		if (size == 12)
			temp = UTF8FontTab12[addr].Msk[t];
		else if (size == 16) temp = UTF8FontTab16[addr].Msk[t];

		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)OLED_DrawPoint(x, y, mode);
			else OLED_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
			if ((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}
	}
}

/**
  * @brief  Show string with selection options

	@param  x,y:Position

			*p:Target string

			OnSelection:Invert(OnSelect) or not(NotOnSelect)

			size:Size of the string(in Y)

	  @retval None
  */
void OLED_ShowAnyString(unsigned char  x, unsigned char  y, const char  *p, bool OnSelection, unsigned char size)
{
#define MAX_CHAR_POSX 127
#define MAX_CHAR_POSY 63  

	while (*p != '\0')
	{
		if (x > MAX_CHAR_POSX) { x = 0; y += 16; }
		if (y > MAX_CHAR_POSY) { y = x = 0; OLED_Clear(); }
		if (*p > 127)
		{
			OLED_ShowNotASCChar(x, y, (char *)p, size, (!OnSelection));
			if (size == 16)
				x += 16;
			else
				x += 12;
			p += 3;
		}
		else
		{
			OLED_ShowChar(x, y, *p, size, (!OnSelection));
			if (size == 16)
				x += 8;
			else
				x += 6;
			p++;
		}
	}
}

/**
  * @brief  Invert a rectangular range

	@param  x1,y1:Position1

			x2,y2:Position2

			x1,y1,x2,y2 satisfies x1<x2,y1<y2

	  @retval None
  */
void OLED_InvertRect(u8 x1, u8 y1, u8 x2, u8 y2)
{
	u8 x, y;
	for (x = x1; x < x2 + 1; x++)
	{
		for (y = y1; y < y2; y++)
		{
			OLED_InvertPoint(x, y);
		}
	}
}

/**
  * @brief  Fill a rectangular range

	@param  x1,y1:Position1

			x2,y2:Position2

			x1,y1,x2,y2 satisfies x1<x2,y1<y2

	  @retval None
  */
void OLED_FillRect(u8 x1, u8 y1, u8 x2, u8 y2, u8 t)
{
	u8 x, y;
	for (x = x1; x < x2 + 1; x++)
	{
		for (y = y1; y < y2; y++)
		{
			OLED_DrawPoint(x, y, t);
		}
	}
}

/**
  * @brief  Draw a line with any position set

	@param  x1,y1:Position1

			x2,y2:Position2

			x1,y1,x2,y2 satisfies x1<x2,y1<y2

	  @retval None
  */
void OLED_DrawAnyLine(u8 x1, u8 y1, u8 x2, u8 y2, bool drawOrUndraw)
{
	u8 x, y;

	if (x1 == x2)
	{
		if (y1 > y2)
			OLED_DrawVerticalLine(x1, y2, y1, drawOrUndraw);
		else
			OLED_DrawVerticalLine(x1, y1, y2, drawOrUndraw);
	}

	else if (y1 == y2)
	{
		OLED_DrawHorizonalLine(y1, x1, x2, drawOrUndraw);
	}

	else if (y2 >= y1)
	{
		if (x2 - x1 <= y2 - y1)
			for (y = y1; y < y2 + 1; y++)
			{
				x = (x2 - x1)*((float)(y - y1) / (float)(y2 - y1)) + x1;
				OLED_DrawPoint(x, y, drawOrUndraw);
			}
		else
		{
			for (x = x1; x < x2 + 1; x++)
			{
				y = (y2 - y1)*((float)(x - x1) / (float)(x2 - x1)) + y1;
				OLED_DrawPoint(x, y, drawOrUndraw);
			}
		}
	}
	else
	{
		if (x2 - x1 <= y1 - y2)
			for (y = y2; y < y1 + 1; y++)
			{
				x = x1 + (x2 - x1)*((float)(y1 - y) / (float)(y1 - y2));
				OLED_DrawPoint(x, y, drawOrUndraw);
			}
		else
		{
			for (x = x1; x < x2 + 1; x++)
			{
				y = y1 - (y1 - y2)*((float)(x - x1) / (float)(x2 - x1));
				OLED_DrawPoint(x, y, drawOrUndraw);
			}
		}
	}
}

/**
  * @brief  Init SPI of OLED

	  @retval None
  */
void SPI2_Init(void)

{
	SPI_InitTypeDef  SPI_InitStructure;

	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI2 and GPIOB clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI2的时钟

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能IO口时钟

	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* Configure SPI2 pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure PB.12 as Output push-pull, used as Flash Chip select */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_12);//片选信号为低

	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	 //设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	//SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; //时钟悬空高
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;//数据捕获于第二个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	//内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//波特率预分频值为2
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7; //SPI_CRCPolynomial定义了用于CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI2  */
	SPI_Cmd(SPI2, ENABLE);
	SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_IT_RXNE);

}

#if OLED_REFRESH_OPTIMIZE_EN
/**
  * @brief  This function handles the update of OLED

	@hint   When OLED_GRAM keeps not edited for 10ms
			This function will upload GRAM to OLED

	  @retval None
  */
void OLED_Refresh_Handler(void *pvParameters)
{
	while (1)
	{
		if (GRAM_Changed == true)
		{
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			OLED_Refresh_Gram();
			xSemaphoreGive(OLEDRelatedMutex);
		}
		vTaskDelay(10 / portTICK_RATE_MS);
	}
}
#endif

/**
  * @brief  Make OLED refresh mode into UpdateJustNow

	  @retval None
  */
void SetUpdateOLEDJustNow()
{
	UpdateOLEDJustNow_Backup = UpdateOLEDJustNow;
	UpdateOLEDJustNow = true;
}

/**
  * @brief  Restore OLED refresh mode

	  @retval None
  */
void ResetUpdateOLEDJustNow()
{
	UpdateOLEDJustNow = UpdateOLEDJustNow_Backup;
}

/**
  * @brief  Backup GRAM into OLED_GRAM_BKP

	  @retval None
  */
void OLED_BackupScreen()
{
	memcpy(OLED_GRAM_Backup, OLED_GRAM, sizeof(OLED_GRAM));
}

/**
  * @brief  Restore GRAM

	  @retval None
  */
void OLED_RestoreScreen()
{
	memcpy(OLED_GRAM, OLED_GRAM_Backup, sizeof(OLED_GRAM));
}

/**
  * @brief  Init OLED

	  @retval None
  */
void OLED_Init(void)
{


	GPIO_InitTypeDef  GPIO_InitStructure;

	//SendLogString("SSD1306 Initializing...\n");
	SPI2_Init();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);

	OLED_WR_Byte(0xAE, OLED_CMD); //关闭显示
	OLED_WR_Byte(0xD5, OLED_CMD); //设置时钟分频因子,震荡频率
	OLED_WR_Byte(0x80, OLED_CMD);   //[3:0],分频因子;[7:4],震荡频率
	OLED_WR_Byte(0xA8, OLED_CMD); //设置驱动路数
	OLED_WR_Byte(0X3f, OLED_CMD); //默认0X3F(1/64) 
	OLED_WR_Byte(0xD3, OLED_CMD); //设置显示偏移
	OLED_WR_Byte(0X00, OLED_CMD); //默认为0

	OLED_WR_Byte(0x40, OLED_CMD); //设置显示开始行 [5:0],行数.

	OLED_WR_Byte(0x8D, OLED_CMD); //电荷泵设置
	OLED_WR_Byte(0x14, OLED_CMD); //bit2，开启/关闭
	OLED_WR_Byte(0x20, OLED_CMD); //设置内存地址模式
	OLED_WR_Byte(0x02, OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
	OLED_WR_Byte(0xA1, OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0, OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	OLED_WR_Byte(0xDA, OLED_CMD); //设置COM硬件引脚配置
	OLED_WR_Byte(0x12, OLED_CMD); //[5:4]配置

	OLED_WR_Byte(0x81, OLED_CMD); //对比度设置
	OLED_WR_Byte(0xFF, OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
	OLED_WR_Byte(0xD9, OLED_CMD); //设置预充电周期
	OLED_WR_Byte(0xf1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB, OLED_CMD); //设置VCOMH 电压倍率
	OLED_WR_Byte(0x30, OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4, OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6, OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
	OLED_WR_Byte(0xAF, OLED_CMD); //开启显示	 

	UpdateOLEDJustNow = true;
	OLED_Clear();
	UpdateOLEDJustNow = false;

	OLEDRelatedMutex = xSemaphoreCreateMutex();

	xTaskCreate(OLED_Refresh_Handler, "OLED Refresh Handler",
		configMINIMAL_STACK_SIZE, NULL, OLED_REFRESH_PRIORITY, NULL);
}

void Draw_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char bmp[])
{
	unsigned int ii = 0;
	unsigned char x, y;
	unsigned char c;
	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y <= y1; y++)
	{
		LCD_Set_Pos(x0, y * 8);
		for (x = x0; x < x1; x++)
		{
			c = bmp[ii++];
			c = (c & 0x55) << 1 | (c & 0xAA) >> 1;
			c = (c & 0x33) << 2 | (c & 0xCC) >> 2;
			c = (c & 0x0F) << 4 | (c & 0xF0) >> 4;
			OLED_WR_Byte(c, OLED_DATA);
		}
	}
}

void PointDrawing_Test()
{
	u16 fps = 0;
	char tempString[20];
	u8 x = 0, y = 0;
	u8 t;
	u8 getTickCountCnt = 0;
	TickType_t lstTickCount = xTaskGetTickCount();
	for (;;)
	{
		SetUpdateOLEDJustNow();
		for (;;)
		{
			x++;
			if (x > 127)
			{
				x = 0; y++;
			}
			if (y > 63)
			{
				y = 0; fps++;
				t = !t;
			}
			OLED_DrawPoint(x, y, t);

			getTickCountCnt++;
			if (getTickCountCnt == 100)
			{
				getTickCountCnt = 0;
				if (lstTickCount + 1000 <= xTaskGetTickCount()) break;
			}
		}
		sprintf(tempString, "%dfps", fps);
		SetUpdateOLEDJustNow();
		OLED_ShowAnyString(0, 0, tempString, NotOnSelect, 16);
		ResetUpdateOLEDJustNow();
		fps = 0;
		vTaskDelay(500);
		lstTickCount = xTaskGetTickCount();
	}
}

























