// ****************************************************************************
/// \file      ws2812b.h
///
/// \brief     WS2812B C HeaderFile
///
/// \details   Driver Module for WS2812B leds.
///
/// \author    Nico Korn
///
/// \version   1.0.0.0
///
/// \date      24032021
/// 
/// \copyright Copyright (c) 2021 Nico Korn
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/// \pre       
///
/// \bug       
///
/// \warning   
///
/// \todo      
///
// ****************************************************************************




// Define to prevent recursive inclusion **************************************
#ifndef __WS2812B_H
#define __WS2812B_H

// Include ********************************************************************
#include "stm32f1xx_hal.h"
#include "main.h"

// 定义 RGB 颜色结构体 红 绿 蓝
struct ws2812b_color
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

// 颜色表的枚举下标
enum color_enum
{
    WS2812B_Color_RED = 0,
    WS2812B_Color_ORANGE,
    WS2812B_Color_YELLOW,
    WS2812B_Color_GREEN,
    WS2812B_Color_CYAN,
    WS2812B_Color_BLUE,
    WS2812B_Color_PURPLE,
    WS2812B_Color_WHITE,
    WS2812B_Color_BLACK,
    WS2812B_Color_COLD_WHITE,
    WS2812B_Color_PINK,
    WS2812B_Color_WARM,
    WS2812B_Color_DEEP_RED,
    WS2812B_Color_CORAL,
    WS2812B_Color_GOLD,
    WS2812B_Color_LIME,
    WS2812B_Color_DEEP_GREEN,
    WS2812B_Color_INDIGO,
    WS2812B_Color_HOT_PINK,
    WS2812B_Color_DEEP_PINK,
    WS2812B_Color_LIGHT_CORAL,
    WS2812B_Color_DODGER_BLUE,
    WS2812B_Color_PEACH,
    WS2812B_Color_APRICOT,
    WS2812B_Color_KHAKI,
    WS2812B_Color_LEMON_CHIFFON,
    WS2812B_Color_SEASHELL,
    WS2812B_Color_SANDY_BROWN,
    WS2812B_Color_ANTIQUE_WHITE,
    WS2812B_Color_LIGHT_STEEL_BLUE,
    WS2812B_Color_BROWN_ROSE,
    WS2812B_Color_BEIGE,
    WS2812B_Color_LIGHT_PINK,
    WS2812B_Color_LIGHT_CORAL_2,
    WS2812B_Color_GOLDENROD,
    WS2812B_Color_CHOCOLATE,
    WS2812B_Color_COPPER,
    WS2812B_Color_TOMATO,
    WS2812B_Color_CORAL_PINK,
    WS2812B_Color_ORCHID,
    WS2812B_Color_DEEP_FUCHSIA,
    WS2812B_Color_FORREST_GREEN,
    WS2812B_Color_DARK_PURPLE,
    WS2812B_Color_DARK_BLUE,
    WS2812B_Color_PEACH_PINK,
    WS2812B_Color_LIGHT_CORAL_3,
    WS2812B_Color_APRICOT_2,
    WS2812B_Color_MAX
};
 extern struct ws2812b_color ws2812b_color_table[];

// Exported defines ***********************************************************
// 定义ws2812b矩阵的大小
#define COL                    ( 8u )     // LED 像素数量
#define ROW                    ( 1u )     // LED 灯条数量


// Exported types *************************************************************
typedef enum
{
   WS2812B_OK       = 0x00U,
   WS2812B_ERROR    = 0x01U,
   WS2812B_BUSY     = 0x02U,
   WS2812B_TIMEOUT  = 0x03U,
   WS2812B_READY    = 0x04U,
   WS2812B_RESET    = 0x05U
} WS2812B_StatusTypeDef;

typedef enum  {
    WS2812B_Animation_RunningWaterLamps         = 0x00U,
	WS2812B_Animation_Init				        = 0x01U,
    // 闪烁
} WS2812B_Animation;


// 串行行的命名
typedef enum{
    BigLight_L      = 0,
    BigLight_R      = 1,
    DrivingLight    = 2,
    FogLight        = 3,
}WS2812B_LightName;

/**
 * 使用方式
 * 1. main函数中使用 init 函数进行初始化
 * 2. 然后可以调用 WS2812B_clearBuffer 函数清空缓冲区
 * 3. 接下来就可以使用 WS2812B_setPixel 函数来设置像素颜色
 * 4. 最后调用 WS2812B_sendBuffer 函数将缓冲区中的数据通过 DMA 生成 PWM 波形驱动 WS2812B LED
 * 
 */



// Exported functions *********************************************************
WS2812B_StatusTypeDef   WS2812B_init            ( void );
void                    WS2812B_sendBuffer      ( void );
void                    WS2812B_clearBuffer     ( void );
void                    WS2812B_setclearBuffer  (uint8_t row);
void                    WS2812B_setPixel        ( uint8_t row, uint16_t col, uint8_t red, uint8_t green, uint8_t blue );


// 动画、效果函数 ***************************************************************
void                    WS2812B_setAnimation   ( uint8_t animation);


#endif // __WS2812B_H

