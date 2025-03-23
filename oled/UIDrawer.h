#ifndef __UI_DRAWER_H
#define __UI_DRAWER_H

#include <stdint.h>
#include "oled.h"
#include "font.h"
#include "UIManager.h"

#ifdef __cplusplus
extern "C" {
#endif

// UI绘制函数类型定义
typedef void (*UIDrawFunc_t)(void);

// UI绘制函数声明
void UI_DrawMain(void);
void UI_DrawStatus(void);
void UI_DrawData(void);
void UI_DrawEnv(void);
void UI_DrawWeather(void);  // 新增天气详情界面
void UI_DrawInit(void);  // 添加初始化界面绘制函数
void UI_DrawLoadingNetData(void);  // 添加加载界面绘制函数
void UI_DrawTest(void);  // 测试界面绘制函数

// 获取UI绘制函数
UIDrawFunc_t UI_GetDrawFunction(UIScreen_t screen);

// 设置天气数据获取状态
void UI_SetWeatherFetching(uint8_t fetching);

// 新增弹窗、对话框和动画绘制功能
void UI_DrawToast(const char* message, ToastType_t type);
void UI_DrawDialog(const char* title, const char* message, DialogType_t type, uint8_t selected_button);
void UI_DrawAnimation(UIScreen_t from_screen, UIScreen_t to_screen, uint8_t progress, uint8_t type);

// 子界面功能相关
void UI_SetSubScreenCount(uint8_t count);
uint8_t UI_GetSubScreenCount(void);

#ifdef __cplusplus
}
#endif

#endif // __UI_DRAWER_H 