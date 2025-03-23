#ifndef __OLED_H__
#define __OLED_H__

#include "font.h"
#include "UIManager.h"
#include "main.h"
#include "string.h"
#include <stdbool.h>








#define SSD1306_I2C_ADDR  0x3C << 1  // I2C 地址，注意 HAL 需要左移 1 位





typedef enum {
  OLED_COLOR_NORMAL = 0, // 正常模式 黑底白字
  OLED_COLOR_REVERSED    // 反色模式 白底黑字
} OLED_ColorMode;

extern I2C_HandleTypeDef I2C_Handle;



// OLED 初始化函数
void SSD1306_Init(I2C_HandleTypeDef *hi2c);  // 初始化 OLED 显示屏

// 控制 OLED 显示状态
void OLED_DisPlay_On(void);  // 打开 OLED 显示
void OLED_DisPlay_Off(void);  // 关闭 OLED 显示

// 帧操作
void OLED_NewFrame(void);  // 开始新的帧
void OLED_ShowFrame(void);  // 显示当前帧

// 像素操作
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color);  // 设置指定位置的像素颜色
void SSD1306_WriteCommand(uint8_t command);  // 向 OLED 发送命令

// 绘图函数
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color);  // 绘制直线
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);  // 绘制矩形
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);  // 绘制填充矩形
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);  // 绘制三角形
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);  // 绘制填充三角形
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);  // 绘制圆形
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);  // 绘制填充圆形
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color);  // 绘制椭圆
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color);  // 绘制图像

// 文本显示函数
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color);  // 显示 ASCII 字符
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color);  // 显示 ASCII 字符串
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color);  // 显示字符串



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////         UI 函数         ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////









#endif // __OLED_H__



