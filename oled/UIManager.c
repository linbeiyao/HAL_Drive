#include "UIManager.h"
#include <string.h>
#include <stdio.h>
#include "font.h"

/*
 * 界面管理层思路：
 * 1. 用一个枚举表示当前界面 (currentScreen)。
 * 2. 每个界面都有一个专门的"绘制函数" (或称"刷新函数")。
 * 3. UIManager_Update() 调用当前界面的绘制函数。
 * 4. 通过 UIManager_SwitchScreen() 改变 currentScreen。
 */

// ------------------【1】定义当前界面 ------------------

static UIScreen_t currentScreen = SCREEN_MAIN;

// ------------------【2】声明各界面绘制函数 ------------------
// 你可以根据自己需要，写更多界面，如：SCREEN_STATUS, SCREEN_DATA, SCREEN_ENV
static void UI_DrawMain(void);
static void UI_DrawStatus(void);
static void UI_DrawData(void);
static void UI_DrawEnv(void);

// 如果界面较多，可以用函数指针数组来简化
typedef void (*UIDrawFunc_t)(void);
static UIDrawFunc_t s_drawFuncs[SCREEN_MAX] = {
    UI_DrawMain,              // SCREEN_MAIN
    UI_DrawStatus,           // SCREEN_STATUS  
    UI_DrawData,            // SCREEN_DATA
    UI_DrawEnv,             // SCREEN_ENV
    
};

// ------------------【3】对外接口实现 ------------------
void UIManager_Init(void)
{
    // 初始化时，可做一些界面相关变量的清零
    // 例如：设置默认界面
    currentScreen = SCREEN_MAIN;

    // 也可以直接先画一次
    UIManager_Update();
}

void UIManager_Update(void)
{
    // 调用当前界面的绘制函数
    if (currentScreen < SCREEN_MAX)
    {
        s_drawFuncs[currentScreen]();
    }
}

void UIManager_SwitchScreen(UIScreen_t screen)
{
    if (screen >= SCREEN_MAX)
        return;

    // 修改当前界面
    currentScreen = screen;

    // 立即刷新或等到下次UIManager_Update时再刷新
    UIManager_Update();
}

// ------------------【4】各界面绘制函数实现 ------------------

// ========== 示例：主界面 ==========
static void UI_DrawMain(void)
{
	
	
	
	
	
	
}

// ========== 示例：状态界面 ==========
static void UI_DrawStatus(void)
{
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString(8, 0, "温湿度检测系统", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 4)) / 2, 16, "欢迎使用", &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ========== 示例：数据界面 ==========
static void UI_DrawData(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(8, 0, "智能水果售货机", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 5)) / 2, 16, "请放置水果", &font16x16, OLED_COLOR_NORMAL);

    // 显示风扇和LED的状态
    char fanStatusStr[32];
    char ledStatusStr[32];

    OLED_PrintString(0, 32, fanStatusStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, ledStatusStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

// ========== 示例：环境界面 ==========
static void UI_DrawEnv(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString(8, 0, "智能水果售货机", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 5)) / 2, 16, "请放置水果", &font16x16, OLED_COLOR_NORMAL);

    // 显示温湿度信息
    char tempStr[32];
    char humStr[32];

    OLED_PrintString(0, 32, tempStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, humStr, &font16x16, OLED_COLOR_NORMAL);

    OLED_ShowFrame();
}

