// https://raw.githubusercontent.com/russhughes/gc9a01_mpy/main/src/gc9a01.h

#ifndef __GC9A01_H__
#define __GC9A01_H__

#include "main.h"

#ifdef SET_SPI_DISPLAY
//

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF


//
#define GC9A01_DIRECT_MODE 1//0
#define GC9A01_BUFFER_MODE 0//1

#define GC9A01_MODE GC9A01_DIRECT_MODE//GC9A01_BUFFER_MODE


#define GC9A01A_SPI_periph       SPI2
#define GC9A01A_RESET_Used       1
#define GC9A01A_CS_Used          1
#define GC9A01A_DC_Used          1
#define GC9A01A_BLK_PWM_Used     1

#if (GC9A01A_RESET_Used)
  #define GC9A01A_RESET_Port     LCD_RST_GPIO_Port//GPIOA
  #define GC9A01A_RESET_Pin      LCD_RST_Pin//GPIO_PIN_3
#endif

#if (GC9A01A_CS_Used)
  #define GC9A01A_CS_Port        LCD_CS_GPIO_Port//GPIOB
  #define GC9A01A_CS_Pin         LCD_CS_Pin//GPIO_PIN_10
#endif

#if (GC9A01A_DC_Used)
  #define GC9A01A_DC_Port        LCD_DC_GPIO_Port//GPIOA
  #define GC9A01A_DC_Pin         LCD_DC_Pin//GPIO_PIN_2
#endif

#if (GC9A01A_BLK_PWM_Used)

#else
  #define GC9A01A_BLK_Port       GPIOA
  #define GC9A01A_BLK_Pin        GPIO_PIN_1
#endif

uint16_t GC9A01A_GetWidth();
uint16_t GC9A01A_GetHeight();

void GC9A01A_Init();
void GC9A01A_Update();
void GC9A01A_SleepMode(uint8_t Mode);
void GC9A01A_DisplayPower(uint8_t On);
uint16_t GC9A01A_GetPixel(int16_t x, int16_t y);
//void GC9A01A_SetBL(uint8_t Value);
void GC9A01A_DrawPixel(int16_t x, int16_t y, uint16_t color);
void GC9A01A_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
//void GC9A01A_DrawPartYX(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff);
//void GC9A01A_DrawPartXY(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pBuff);

//
#endif


#endif  /*  __GC9A01_H__ */
