

#include "gc9a01.h"


#ifdef SET_SPI_DISPLAY

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "dispcolor.h"


#define CS_HIGH()     LCD_CS_OFF()
#define CS_LOW()      LCD_CS_ON()
#define RESET_HIGH()  LCD_RST_OFF()
#define RESET_LOW()   LCD_RST_ON()
#define DC_HIGH()     LCD_DATA()
#define DC_LOW()      LCD_CMD()

#define Cmd_SLPIN       0x10
#define Cmd_SLPOUT      0x11
#define Cmd_INVOFF      0x20
#define Cmd_INVON       0x21
#define Cmd_DISPOFF     0x28
#define Cmd_DISPON      0x29
#define Cmd_CASET       0x2A
#define Cmd_RASET       0x2B
#define Cmd_RAMWR       0x2C
#define Cmd_TEON      	0x35     // Tearing effect line ON
#define Cmd_MADCTL      0x36     // Memory data access control
#define Cmd_COLMOD      0x3A     // Pixel format set

#define Cmd_DisplayFunctionControl    0xB6

#define Cmd_PWCTR1       0xC1    // Power control 1
#define Cmd_PWCTR2       0xC3    // Power control 2
#define Cmd_PWCTR3       0xC4    // Power control 3
#define Cmd_PWCTR4       0xC9    // Power control 4
#define Cmd_PWCTR7       0xA7    // Power control 7

#define Cmd_FRAMERATE       0xE8
#define Cmd_InnerReg1Enable 0xFE
#define Cmd_InnerReg2Enable 0xEF

#define Cmd_GAMMA1       0xF0    // Set gamma 1
#define Cmd_GAMMA2       0xF1    // Set gamma 2
#define Cmd_GAMMA3       0xF2    // Set gamma 3
#define Cmd_GAMMA4       0xF3    // Set gamma 4

//==============================================================================

#define ColorMode_RGB_16bit  0x50
#define ColorMode_RGB_18bit  0x60
#define ColorMode_MCU_12bit  0x03
#define ColorMode_MCU_16bit  0x05
#define ColorMode_MCU_18bit  0x06

#define MADCTL_MY        0x80
#define MADCTL_MX        0x40
#define MADCTL_MV        0x20
#define MADCTL_ML        0x10
#define MADCTL_BGR       0x08
#define MADCTL_MH        0x04

#define GC9A01A_Width	240
#define GC9A01A_Height 	240

//--------------------------------------------------------------------------------------

uint8_t GC9A01A_X_Start = 0, GC9A01A_Y_Start = 0;
const uint32_t min_wait_lcd = 100;
const uint32_t max_wait_lcd = 1000;

#if (GC9A01_MODE == GC9A01_BUFFER_MODE)
	static uint16_t ScreenBuff[GC9A01A_Height * GC9A01A_Width];
#endif

//--------------------------------------------------------------------------------------
void SPI_send(uint8_t *pBuff, uint16_t Len)
{
	lcdRdy = 0;
	if (HAL_SPI_Transmit(lcdPort, pBuff, Len, min_wait_lcd) != HAL_OK) devError |= devLCD;
	lcdRdy = 1;
}
//--------------------------------------------------------------------------------------
void SPI_send_dma(uint8_t *pBuff, uint16_t Len)
{
	lcdRdy = 0;
	if (HAL_SPI_Transmit_DMA(lcdPort, pBuff, Len) != HAL_OK) devError |= devLCD;
	else {
		uint32_t sch = max_wait_lcd;
		while (!lcdRdy && --sch) HAL_Delay(1);
		if (!sch) devError |= devLCD;
	}
}
//--------------------------------------------------------------------------------------
void SPI_recv(uint8_t *pBuff, uint16_t Len)
{
	lcdRdy = 0;
	if (HAL_SPI_Receive(lcdPort, pBuff, Len, min_wait_lcd) != HAL_OK) devError |= devLCD;
	lcdRdy = 1;
}
//--------------------------------------------------------------------------------------
uint16_t SPI_sendrecv(void *pData)
{
uint8_t result = 0;

	lcdRdy = 0;
	if (HAL_SPI_TransmitReceive(lcdPort, pData, &result, 1, min_wait_lcd) != HAL_OK) devError |= devLCD;
	lcdRdy = 1;

	return result;
}
//--------------------------------------------------------------------------------------
void SPI_SendRecv_dma(uint8_t *pTxBuff, uint8_t *pRxBuff, uint16_t Len)
{
	if ( ((!pTxBuff) && (!pRxBuff)) || (!Len) ) return;

	lcdRdy = 0;
	if (HAL_SPI_TransmitReceive_DMA(lcdPort, pTxBuff, pRxBuff, Len) != HAL_OK) devError |= devLCD;
	else {
		uint32_t sch = max_wait_lcd;
		while (!lcdRdy && --sch) HAL_Delay(1);
		if (!sch) devError |= devLCD;
	}
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
#if (GC9A01_MODE == GC9A01_BUFFER_MODE)
static void SwapBytes(uint16_t *color)
{
    uint8_t temp = *color >> 8;
    *color = (*color << 8) | temp;
}
#endif
//--------------------------------------------------------------------------------------
void SendCmd(uint8_t Cmd)
{
    DC_LOW(); CS_LOW();

    SPI_send(&Cmd, 1);

    CS_HIGH();
}
//--------------------------------------------------------------------------------------
void SendData(uint8_t Data)
{
    DC_HIGH(); CS_LOW();

    SPI_send(&Data, 1);

    CS_HIGH();
}
//--------------------------------------------------------------------------------------
void SendDatas(uint8_t *Data, uint16_t len)
{
    DC_HIGH(); CS_LOW();

    SPI_send(Data, len);

    CS_HIGH();
}
//--------------------------------------------------------------------------------------
void GC9A01A_HardReset(void)
{
#if (GC9A01A_RESET_Used)
	RESET_LOW();
	HAL_Delay(10);
	RESET_HIGH();
	HAL_Delay(150);
#endif
}
//--------------------------------------------------------------------------------------
void GC9A01A_SleepMode(uint8_t Mode)
{
    if (Mode) SendCmd(Cmd_SLPIN);
         else SendCmd(Cmd_SLPOUT);

    HAL_Delay(500);
}
//--------------------------------------------------------------------------------------
void GC9A01A_InversionMode(uint8_t Mode)
{
	if (Mode) SendCmd(Cmd_INVON);
	     else SendCmd(Cmd_INVOFF);
}
//--------------------------------------------------------------------------------------
void GC9A01A_DisplayPower(uint8_t On)
{
	if (On) SendCmd(Cmd_DISPON);
	   else SendCmd(Cmd_DISPOFF);
}
//--------------------------------------------------------------------------------------
static void ColumnSet(uint16_t ColumnStart, uint16_t ColumnEnd)
{
	if (ColumnStart > ColumnEnd) return;
	if (ColumnEnd > GC9A01A_Width) return;

	ColumnStart += GC9A01A_X_Start;
	ColumnEnd += GC9A01A_X_Start;

	SendCmd(Cmd_CASET);
//	SendData(ColumnStart >> 8);
//	SendData(ColumnStart & 0xFF);
//	SendData(ColumnEnd >> 8);
//	SendData(ColumnEnd & 0xFF);
	uint8_t dat[] = { ColumnStart >> 8, ColumnStart & 0xFF, ColumnEnd >> 8, ColumnEnd & 0xFF };
	SendDatas(dat, sizeof(dat));
}
//--------------------------------------------------------------------------------------
static void RowSet(uint16_t RowStart, uint16_t RowEnd)
{
	if (RowStart > RowEnd) return;
	if (RowEnd > GC9A01A_Height) return;

	RowStart += GC9A01A_Y_Start;
	RowEnd += GC9A01A_Y_Start;

	SendCmd(Cmd_RASET);
//	SendData(RowStart >> 8);
//	SendData(RowStart & 0xFF);
//	SendData(RowEnd >> 8);
//	SendData(RowEnd & 0xFF);
	uint8_t dat[] = { RowStart >> 8, RowStart & 0xFF, RowEnd >> 8, RowEnd & 0xFF };
	SendDatas(dat, sizeof(dat));
}
//--------------------------------------------------------------------------------------
void GC9A01A_SetWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	ColumnSet(x0, x1);
	RowSet(y0, y1);

	SendCmd(Cmd_RAMWR);
}
//--------------------------------------------------------------------------------------
static void ColorModeSet(uint8_t ColorMode)
{
	SendCmd(Cmd_COLMOD);
	SendData(ColorMode & 0x77);
}
//--------------------------------------------------------------------------------------
static void MemAccessModeSet(uint8_t Rotation, uint8_t VertMirror, uint8_t HorizMirror, uint8_t IsBGR)
{
uint8_t Value;

	Rotation &= 7;

	SendCmd(Cmd_MADCTL);

	switch (Rotation) {
		case 0:
			Value = 0;
		break;
		case 1:
			Value = MADCTL_MX;
		break;
		case 2:
			Value = MADCTL_MY;
		break;
		case 3:
			Value = MADCTL_MX | MADCTL_MY;
		break;
		case 4:
			Value = MADCTL_MV;
		break;
		case 5:
			Value = MADCTL_MV | MADCTL_MX;
		break;
		case 6:
			Value = MADCTL_MV | MADCTL_MY;
		break;
		case 7:
			Value = MADCTL_MV | MADCTL_MX | MADCTL_MY;
		break;
	}

	if (VertMirror) Value = MADCTL_ML;
	if (HorizMirror) Value = MADCTL_MH;

	if (IsBGR) Value |= MADCTL_BGR;

	SendData(Value);
}
//--------------------------------------------------------------------------------------
/*
void GC9A01A_SetBL(uint8_t Value)
{
	return;

	//if (Value > 100) Value = 100;
	//tmr11_PWM_set(Value);
}
*/
//--------------------------------------------------------------------------------------
#if (GC9A01_MODE == GC9A01_DIRECT_MODE)

void GC9A01A_RamWrite(uint16_t *pBuff, uint16_t Len)
{
	while (Len--) {
		SendData(*pBuff >> 8);
		SendData(*pBuff & 0xFF);
	}
}
//--------------------------------------------------------------------------------------
void GC9A01A_DrawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) ||(x >= GC9A01A_Width) || (y < 0) || (y >= GC9A01A_Height)) return;

	GC9A01A_SetWindow(x, y, x, y);
	GC9A01A_RamWrite(&color, 1);
}
//--------------------------------------------------------------------------------------
void GC9A01A_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	if ((x >= GC9A01A_Width) || (y >= GC9A01A_Height)) return;

	if ((x + w) > GC9A01A_Width) w = GC9A01A_Width - x;

	if ((y + h) > GC9A01A_Height) h = GC9A01A_Height - y;

	GC9A01A_SetWindow(x, y, x + w - 1, y + h - 1);

	for (uint32_t i = 0; i < (h * w); i++) GC9A01A_RamWrite(&color, 1);
}
//--------------------------------------------------------------------------------------
void GC9A01A_Update() {}
//--------------------------------------------------------------------------------------
#else
//--------------------------------------------------------------------------------------
void GC9A01A_DrawPixel(int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= GC9A01A_Width) || (y < 0) || (y >= GC9A01A_Height))
		return;

	SwapBytes(&color);

	ScreenBuff[y * GC9A01A_Width + x] = color;
}

uint16_t GC9A01A_GetPixel(int16_t x, int16_t y) {
	if ((x < 0) || (x >= GC9A01A_Width) || (y < 0) || (y >= GC9A01A_Height))
		return 0;

	uint16_t color = ScreenBuff[y * GC9A01A_Width + x];
	SwapBytes(&color);
	return color;
}

void GC9A01A_FillRect(int16_t x, int16_t y, int16_t w, int16_t h,
		uint16_t color) {
	if ((w <= 0) || (h <= 0) || (x >= GC9A01A_Width) || (y >= GC9A01A_Height))
		return;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}

	if ((w <= 0) || (h <= 0))
		return;

	if ((x + w) > GC9A01A_Width)
		w = GC9A01A_Width - x;
	if ((y + h) > GC9A01A_Height)
		h = GC9A01A_Height - y;

	SwapBytes(&color);

	for (uint16_t row = 0; row < h; row++) {
		for (uint16_t col = 0; col < w; col++)
			//GC9A01A_DrawPixel(col, row, color);
			ScreenBuff[(y + row) * GC9A01A_Width + x + col] = color;
	}
}

void SendPart2() {
	uint16_t len = GC9A01A_Width * GC9A01A_Height;
	SPI_send_dma(GC9A01A_SPI_periph, 0, (uint8_t*) &ScreenBuff[len / 2], len,
			0);
}

void GC9A01A_Update() {
	GC9A01A_SetWindow(0, 0, GC9A01A_Width - 1, GC9A01A_Height - 1);

	DC_HIGH(); CS_LOW();

	uint16_t len = GC9A01A_Width * GC9A01A_Height;
	SPI_send_dma(GC9A01A_SPI_periph, 0, (uint8_t*) ScreenBuff, len, SendPart2);
}

#endif

//==============================================================================

uint16_t GC9A01A_GetWidth() { return GC9A01A_Width; }

uint16_t GC9A01A_GetHeight() { return GC9A01A_Height; }

//==============================================================================

void GC9A01A_Init()
{
	GC9A01A_X_Start = 0;
	GC9A01A_Y_Start = 0;

	GC9A01A_HardReset();

	SendCmd(Cmd_InnerReg2Enable);
	SendCmd(0xEB);        // ?
	SendData(0x14);

	SendCmd(Cmd_InnerReg1Enable);
	SendCmd(Cmd_InnerReg2Enable);

	SendCmd(0xEB);        // ?
	SendData(0x14);       // ?

	SendCmd(0x84);        // ?
	SendData(0x40);

	SendCmd(0x85);        // ?
	SendData(0xFF);

	SendCmd(0x86);        // ?
	SendData(0xFF);

	SendCmd(0x87);        // ?
	SendData(0xFF);

	SendCmd(0x88);        // ?
	SendData(0x0A);

	SendCmd(0x89);        // ?
	SendData(0x21);

	SendCmd(0x8A);        // ?
	SendData(0x00);

	SendCmd(0x8B);        // ?
	SendData(0x80);

	SendCmd(0x8C);        // ?
	SendData(0x01);

	SendCmd(0x8D);        // ?
	SendData(0x01);

	SendCmd(0x8E);        // ?
	SendData(0xFF);

	SendCmd(0x8F);        // ?
	SendData(0xFF);

	SendCmd(Cmd_DisplayFunctionControl);
	SendData(0x00);
	SendData(0x20); // Scan direction S360 -> S1

	MemAccessModeSet(0, 0, 0, 1);
	ColorModeSet(ColorMode_MCU_16bit);

	SendCmd(0x90);        // ?
	SendData(0x08);
	SendData(0x08);
	SendData(0x08);
	SendData(0x08);

	SendCmd(0xBD);        // ?
	SendData(0x06);

	SendCmd(0xBC);        // ?
	SendData(0x00);

	SendCmd(0xFF);        // ?
	SendData(0x60);
	SendData(0x01);
	SendData(0x04);

	SendCmd(Cmd_PWCTR2);  // Power control 2
	SendData(0x13);       // 5.18 V
	SendCmd(Cmd_PWCTR3);  // Power control 3
	SendData(0x13);       // VREG2A = -3.82 V, VREG2B = 0.68 V
	SendCmd(Cmd_PWCTR4);  // Power control 4
	SendData(0x22);       // VREG2A = 5.88 V, VREG2B = -2.88 V

	SendCmd(0xBE);        // ?
	SendData(0x11);

	SendCmd(0xE1);        // ?
	SendData(0x10);
	SendData(0x0E);

	SendCmd(0xDF);        // ?
	SendData(0x21);
	SendData(0x0c);
	SendData(0x02);

	SendCmd(Cmd_GAMMA1);
	SendData(0x45);
	SendData(0x09);
	SendData(0x08);
	SendData(0x08);
	SendData(0x26);
	SendData(0x2A);

	SendCmd(Cmd_GAMMA2);
	SendData(0x43);
	SendData(0x70);
	SendData(0x72);
	SendData(0x36);
	SendData(0x37);
	SendData(0x6F);

	SendCmd(Cmd_GAMMA3);
	SendData(0x45);
	SendData(0x09);
	SendData(0x08);
	SendData(0x08);
	SendData(0x26);
	SendData(0x2A);

	SendCmd(Cmd_GAMMA4);
	SendData(0x43);
	SendData(0x70);
	SendData(0x72);
	SendData(0x36);
	SendData(0x37);
	SendData(0x6F);

	SendCmd(0xED);        // ?
	SendData(0x1B);
	SendData(0x0B);

	SendCmd(0xAE);        // ?
	SendData(0x77);

	SendCmd(0xCD);        // ?
	SendData(0x63);

	SendCmd(0x70);        // ?
	SendData(0x07);
	SendData(0x07);
	SendData(0x04);
	SendData(0x0E);
	SendData(0x0F);
	SendData(0x09);
	SendData(0x07);
	SendData(0x08);
	SendData(0x03);

	SendCmd(Cmd_FRAMERATE);       // Frame rate
	SendData(0x34);         // 4 dot inversion

	SendCmd(0x62);        // ?
	SendData(0x18);
	SendData(0x0D);
	SendData(0x71);
	SendData(0xED);
	SendData(0x70);
	SendData(0x70);
	SendData(0x18);
	SendData(0x0F);
	SendData(0x71);
	SendData(0xEF);
	SendData(0x70);
	SendData(0x70);

	SendCmd(0x63);        // ?
	SendData(0x18);
	SendData(0x11);
	SendData(0x71);
	SendData(0xF1);
	SendData(0x70);
	SendData(0x70);
	SendData(0x18);
	SendData(0x13);
	SendData(0x71);
	SendData(0xF3);
	SendData(0x70);
	SendData(0x70);

	SendCmd(0x64);        // ?
	SendData(0x28);
	SendData(0x29);
	SendData(0xF1);
	SendData(0x01);
	SendData(0xF1);
	SendData(0x00);
	SendData(0x07);

	SendCmd(0x66);        // ?
	SendData(0x3C);
	SendData(0x00);
	SendData(0xCD);
	SendData(0x67);
	SendData(0x45);
	SendData(0x45);
	SendData(0x10);
	SendData(0x00);
	SendData(0x00);
	SendData(0x00);

	SendCmd(0x67);        // ?
	SendData(0x00);
	SendData(0x3C);
	SendData(0x00);
	SendData(0x00);
	SendData(0x00);
	SendData(0x01);
	SendData(0x54);
	SendData(0x10);
	SendData(0x32);
	SendData(0x98);

	SendCmd(0x74);        // ?
	SendData(0x10);
	SendData(0x85);
	SendData(0x80);
	SendData(0x00);
	SendData(0x00);
	SendData(0x4E);
	SendData(0x00);

	SendCmd(0x98);        // ?
	SendData(0x3e);
	SendData(0x07);

	SendCmd(Cmd_TEON); // Tearing effect line on

	GC9A01A_InversionMode(1);
	GC9A01A_SleepMode(0);

	HAL_Delay(120);
	GC9A01A_DisplayPower(1);
	HAL_Delay(20);

	GC9A01A_FillRect(0, 0, GC9A01A_Width, GC9A01A_Height, BLACK);
	//GC9A01A_Update();
	HAL_Delay(20);

	//GC9A01A_SetBL(100);
}
//==============================================================================

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------


#endif

