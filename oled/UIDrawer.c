#include "UIDrawer.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "weather_example.h"

// 外部变量声明
extern Weather_t g_weather;
extern unsigned int soil_moisture_value;
extern unsigned int voltage_value;
extern unsigned int current_value;
extern unsigned int power_value;
extern unsigned char is_pump_on;
extern unsigned char is_mode_auto;
extern float rain_probability;
extern uint8_t is_test;
extern char update_time[32];

extern uint8_t is_init_ok;

// 添加一个标志位表示是否正在获取数据
static uint8_t is_fetching_weather = 0;

// UI绘制函数指针数组
static UIDrawFunc_t s_drawFuncs[SCREEN_MAX] = {
    UI_DrawInit,             // SCREEN_INIT
    UI_DrawLoadingNetData,  // SCREEN_LOADING_NETDATA
    UI_DrawMain,            // SCREEN_MAIN
    UI_DrawStatus,          // SCREEN_STATUS  
    UI_DrawData,            // SCREEN_DATA
    UI_DrawEnv,             // SCREEN_ENV
    UI_DrawWeather,         // SCREEN_WEATHER
    UI_DrawTest            // SCREEN_TEST
};

// 获取UI绘制函数指针
UIDrawFunc_t UI_GetDrawFunction(UIScreen_t screen)
{
    if (screen < SCREEN_MAX)
    {
        return s_drawFuncs[screen];
    }
    return NULL;
}

// 设置获取天气状态的函数
void UI_SetWeatherFetching(uint8_t fetching)
{
    is_fetching_weather = fetching;
    UIManager_Update();  // 立即更新界面
}

// ========== 初始化界面 ==========
void UI_DrawInit(void)
{
    static uint8_t dots = 0;
    char loading[32] = {0};
    
    OLED_NewFrame();
    
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统初始化", &font16x16, OLED_COLOR_NORMAL);
    

    
    OLED_PrintString((128 - (16 * strlen(loading))) / 2, 24, loading, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示提示
    OLED_PrintString((128 - (16 * 4)) / 2, 48, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 主界面 ==========
void UI_DrawMain(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    
    if (!is_init_ok) {
        // 显示初始化界面
        OLED_PrintString((128 - (16 * 4)) / 2, 16, "系统初始化", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    } else {
        // 显示标题
        OLED_PrintString((128 - (16 * 5)) / 2, 0, "土壤湿度系统", &font16x16, OLED_COLOR_NORMAL);
        
        // 显示土壤湿度（反向计算：越湿润值越大）
        sprintf(buffer, "湿度: %d%%", 100 - (int)(soil_moisture_value * 100 / 4095));
        OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示水泵状态
        sprintf(buffer, "水泵: %s", is_pump_on ? "开启" : "关闭");
        OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示模式
        sprintf(buffer, "模式: %s", is_mode_auto ? "自动" : "手动");
        OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    }
    
    OLED_ShowFrame();
}

// ========== 状态界面 ==========
void UI_DrawStatus(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统状态", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示电压 (voltage_value单位为厘伏，显示为V)
    sprintf(buffer, "电压: %.2fV", voltage_value / 100.0f);
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示电流 (current_value单位为毫安，显示为mA)
    sprintf(buffer, "电流: %dmA", current_value);
    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示功率 (power_value单位为毫瓦，显示为mW)
    sprintf(buffer, "功率: %dmW", power_value);
    OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 数据界面 ==========
void UI_DrawData(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统信息", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示阈值设置
    sprintf(buffer, "湿度阈值: %d%%", 30);
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示版本信息
    sprintf(buffer, "版本: V1.0");
    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 环境信息界面 ==========
void UI_DrawEnv(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    
    if(is_fetching_weather) {
        // 显示加载界面
        OLED_PrintString((128 - (16 * 4)) / 2, 0, "正在获取", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 16, "天气数据", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
        
        // 显示动态加载动画
        static uint8_t dots = 0;
        char loading[32] = "Loading";
        for(uint8_t i = 0; i < (dots % 3 + 1); i++) {
            strcat(loading, ".");
        }
        dots++;
        OLED_PrintString((128 - (16 * strlen(loading))) / 2, 48, loading, &font16x16, OLED_COLOR_NORMAL);
    } else {
        // 显示正常的环境信息界面
        WeatherDay_t *day = &g_weather.days[g_weather.current_day];
        
        // 显示日期和页码
        sprintf(buffer, "%s (%d/3)", day->date, g_weather.current_day + 1);
        OLED_PrintString(0, 0, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示温度范围
        sprintf(buffer, "温度:%.1f-%.1f℃", day->min_temp, day->max_temp);
        OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示天气和降雨概率（使用float类型）
        sprintf(buffer, "%s 雨:%.1f%%", day->text_day, rain_probability);
        OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示风向和湿度
        sprintf(buffer, "%s %dkm/h %d%%", day->wind_direction, (int)day->wind_speed, day->humidity);
        OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    }
    
    OLED_ShowFrame();
}

// ========== 天气详情界面 ==========
void UI_DrawWeather(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    
    if(is_fetching_weather) {
        // 显示加载界面
        OLED_PrintString((128 - (16 * 4)) / 2, 0, "正在获取", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 16, "天气数据", &font16x16, OLED_COLOR_NORMAL);
        OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    } else {
        WeatherDay_t *day = &g_weather.days[g_weather.current_day];
        
        // 显示日期和页码
        sprintf(buffer, "详情 %s (%d/3)", day->date, g_weather.current_day + 1);
        OLED_PrintString(0, 0, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示白天和夜间天气
        sprintf(buffer, "日:%s", day->text_day);
        OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        sprintf(buffer, "夜:%s", day->text_night);
        OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
        
        // 显示更新时间
        char hour[3] = {0}, min[3] = {0};
        if(strlen(g_weather.update_time) >= 16) {
            strncpy(hour, g_weather.update_time + 11, 2);
            strncpy(min, g_weather.update_time + 14, 2);
            sprintf(buffer, "更新:%s:%s", hour, min);
            OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
        }
    }
    
    OLED_ShowFrame();
} 

void UI_DrawLoadingNetData(void)
{
    OLED_NewFrame();
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "正在获取", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString((128 - (16 * 4)) / 2, 16, "天气数据", &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString((128 - (16 * 4)) / 2, 32, "请稍候...", &font16x16, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}

// 测试界面绘制函数
void UI_DrawTest(void)
{
    char buf[32];
    extern uint8_t current_item;  // 声明为外部变量
    is_test = 1;
    
    OLED_NewFrame();
    
    // 绘制标题
    OLED_PrintString(0, 0, "测试模式", &font16x16, OLED_COLOR_NORMAL);
    OLED_DrawLine(0, 16, 127, 16, OLED_COLOR_NORMAL);
    
    // 绘制土壤湿度（使用反白显示选中项）
    sprintf(buf, "土壤湿度: %d%%", 
            100 - (int)(soil_moisture_value * 100 / 4095));
    OLED_PrintString(0, 20, buf, &font16x16, 
                    current_item == 0 ? OLED_COLOR_REVERSED : OLED_COLOR_NORMAL);
    
    // 绘制降雨概率（使用反白显示选中项，支持float类型）
    sprintf(buf, "降雨概率: %.1f%%", rain_probability);
    OLED_PrintString(0, 36, buf, &font16x16,
                    current_item == 1 ? OLED_COLOR_REVERSED : OLED_COLOR_NORMAL);
    
    // 绘制操作提示
    OLED_PrintString(0, 52, "K2:选择 K3:+ K4:-", &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}



