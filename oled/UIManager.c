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

static void UI_ModeManual(void);
static void UI_ModeAuto(void);

// 如果界面较多，可以用函数指针数组来简化
typedef void (*UIDrawFunc_t)(void);
static UIDrawFunc_t s_drawFuncs[SCREEN_MAX] = {
    UI_DrawMain,              // SCREEN_MAIN
    UI_DrawStatus,           // SCREEN_STATUS  
    UI_DrawData,            // SCREEN_DATA
    
    UI_ModeManual,           // SCREEN_MODE_MANUAL
    UI_ModeAuto,            // SCREEN_MODE_AUTO
    
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
    // UIManager_Update();
}

// ------------------【4】各界面绘制函数实现 ------------------

// ========== 示例：主界面 ==========
static void UI_DrawMain(void)
{
	  OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "土壤湿度检测", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString((128 - (16 * 4)) / 2, 16, "欢迎使用", &font16x16, OLED_COLOR_NORMAL);


    OLED_ShowFrame();
	
}

// ========== 示例：状态界面 ==========
static void UI_DrawStatus(void)
{
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "土壤湿度检测", &font16x16, OLED_COLOR_NORMAL);

    // 显示当前模式
    OLED_PrintString(0, 16, "当前模式：", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 32, "水泵状态：", &font16x16, OLED_COLOR_NORMAL);



    if(System_GetMode() == MODE_AUTO){
        OLED_PrintString(128 - (16 * 4), 16, "自动", &font16x16, OLED_COLOR_NORMAL);
    }
    else{
        OLED_PrintString(128 - (16 * 4), 16, "手动", &font16x16, OLED_COLOR_NORMAL);
    }

    if(System_GetPumpStatus() == 1){
        OLED_PrintString(128 - (16 * 4), 32, "开启", &font16x16, OLED_COLOR_NORMAL);
    }
    else{
        OLED_PrintString(128 - (16 * 4), 32, "关闭", &font16x16, OLED_COLOR_NORMAL);
    }

    
    OLED_ShowFrame();
}

// ========== 示例：数据界面 ==========
static void UI_DrawData(void)
{
    OLED_NewFrame();

    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "土壤湿度检测", &font16x16, OLED_COLOR_NORMAL);

    // 显示提示信息
    OLED_PrintString(0, 16, "湿度:", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 32, "电压:", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(64, 16, "电流:", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(64, 32, "功率:", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 48, "降雨概率:", &font16x16, OLED_COLOR_NORMAL);


    // 显示湿度
    char humidityStr[32];
    char voltageStr[32];
    char currentStr[32];
    char powerStr[32];
    char rainStr[32];

    sprintf(humidityStr, "%u%%", System_GetSoilMoisture());
    sprintf(voltageStr, "%.2fV", System_GetVoltage());
    sprintf(currentStr, "%.2fA", System_GetCurrent());
    sprintf(powerStr, "%.2fW", System_GetPower());
    sprintf(rainStr, "%u%%   %u%%", System_GetTodayRainProbability(), System_GetTomorrowRainProbability());


    OLED_PrintString(128 - (16 * 2) + 8, 32, voltageStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(128 - (16 * 2) + 8, 16, currentStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(128 - (16 * 2) + 8, 32, powerStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(128 - (16 * 2) + 8, 48, rainStr, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(128 - (16 * 4) + 8, 64, humidityStr, &font16x16, OLED_COLOR_NORMAL);



    OLED_ShowFrame();
}

// 手动模式提示界面
static void UI_ModeManual(void)
{
    OLED_NewFrame();
    OLED_PrintString((128 - (12 * 12)) / 2, (64 - 24) / 2, "Manual Mode", &font24x12, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}



// 自动模式提示界面
static void UI_ModeAuto(void)
{
    OLED_NewFrame();
    OLED_PrintString((128 - (12 * 10)) / 2, (64 - 24) / 2, "Auto Mode", &font24x12, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}

