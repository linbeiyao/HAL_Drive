#ifndef __WS2812B_H
#define __WS2812B_H

#include <main.h>
#include <stm32f1xx_hal.h>
#include <stdlib.h>
#include <tim.h>

#define ONE_PULSE        (59)                           // 1 的脉冲宽度
#define ZERO_PULSE       (29)                           // 0 的脉冲宽度
#define RESET_PULSE      (80)                           // 重置脉冲宽度（至少 40 微秒）
/* 这里需要修改数量与需要控制的灯的数量一致 */
#define LED_NUMS         (16)                            // LED 数量 
#define LED_DATA_LEN     (24)                           // 每个 LED 的数据长度（24 位）
#define WS2812_DATA_LEN  (LED_NUMS * LED_DATA_LEN)     // WS2812 总数据长度（LED 数量乘以每个 LED 的数据长度）


// 定义 RGB 颜色结构体 红 绿 蓝
struct ws2812b_color
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};


// 定义环境场景枚举类型
enum EnvFunc
{
    LightHome,    // 暖色，白色（不交替）
    LightBookBar, // 暖黄色（渐变）（30秒）
    LightMeeting, // 冷白色
    LightRest,    // 暖色
    LightEnter,    // 赤、橙、黄、绿、青、蓝、紫（交替）（21秒）
    LightOff,
};

// 颜色表的枚举下标
enum color_enum
{
    Color_RED = 0,
    Color_ORANGE,
    Color_YELLOW,
    Color_GREEN,
    Color_CYAN,
    Color_BLUE,
    Color_PURPLE,
    Color_WHITE,
    Color_BLACK,
    Color_COLD_WHITE,
    Color_PINK,
    Color_WARM,
    Color_DEEP_RED,
    Color_CORAL,
    Color_GOLD,
    Color_LIME,
    Color_DEEP_GREEN,
    Color_INDIGO,
    Color_HOT_PINK,
    Color_DEEP_PINK,
    Color_LIGHT_CORAL,
    Color_DODGER_BLUE,
    Color_PEACH,
    Color_APRICOT,
    Color_KHAKI,
    Color_LEMON_CHIFFON,
    Color_SEASHELL,
    Color_SANDY_BROWN,
    Color_ANTIQUE_WHITE,
    Color_LIGHT_STEEL_BLUE,
    Color_BROWN_ROSE,
    Color_BEIGE,
    Color_LIGHT_PINK,
    Color_LIGHT_CORAL_2,
    Color_GOLDENROD,
    Color_CHOCOLATE,
    Color_COPPER,
    Color_TOMATO,
    Color_CORAL_PINK,
    Color_ORCHID,
    Color_DEEP_FUCHSIA,
    Color_FORREST_GREEN,
    Color_DARK_PURPLE,
    Color_DARK_BLUE,
    Color_PEACH_PINK,
    Color_LIGHT_CORAL_3,
    Color_APRICOT_2
};


// 全局变量 系统场景标识符
extern uint8_t homeflag;
extern uint8_t bookbarflag;
extern uint8_t meetingflag;
extern uint8_t restflag;
extern uint8_t enterflag;


// 颜色表的枚举下标
extern enum color_enum;                                         // 颜色表的下标枚举变量
extern struct ws2812b_color ws2812b_color_table[];              // 颜色表
uint16_t static RGB_buffur[RESET_PULSE + WS2812_DATA_LEN] = { 0 };


// 当前颜色存储
extern uint8_t current_R[LED_NUMS], current_G[LED_NUMS], current_B[LED_NUMS];
// 目标颜色存储
extern uint8_t target_R[LED_NUMS], target_G[LED_NUMS], target_B[LED_NUMS];

void ws2812_set_RGB(uint8_t R, uint8_t G, uint8_t B, uint16_t num);

void ws2812b_SetEnv(enum color_enum color1, enum color_enum color2, uint8_t single_color);          // 实现环境光颜色设置  单颜色模式 双颜色模式
void ws2812b_SetTarget_ALL(enum color_enum target_color);                                           // 用来设置全部灯珠全部通道的颜色            
uint8_t ws2812_set_color_smooth_all(uint8_t target_R[], uint8_t target_G[], uint8_t target_B[]);    // 平滑颜色设置







#endif 