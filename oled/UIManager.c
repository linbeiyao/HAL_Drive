#include "UIManager.h"
#include "UIDrawer.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "font.h"
#include "main.h"

/*
 * 界面管理层思路：
 * 1. 用一个枚举表示当前界面 (currentScreen)。
 * 2. 每个界面都有一个专门的"绘制函数" (或称"刷新函数")。
 * 3. UIManager_Update() 调用当前界面的绘制函数。
 * 4. 通过 UIManager_SwitchScreen() 改变 currentScreen。
 */

#define  OLED_WIDTH 128
#define  OLED_HEIGHT 64

// 界面层级结构
static UIScreenHierarchy_t screenHierarchy = {
    .parentScreen = SCREEN_MAIN,
    .subScreenIndex = 0,
    .subScreenCount = 0
};

// ------------------【1】定义当前界面 ------------------

static UIScreen_t currentScreen = SCREEN_MAIN;

// 外部变量声明
extern unsigned int soil_moisture_value;
extern unsigned int voltage_value;
extern unsigned int current_value;
extern unsigned int power_value;
extern unsigned char is_pump_on;
extern unsigned char is_mode_auto;

// ------------------【新增弹窗和对话框相关变量】------------------
// Toast提示相关变量
static char toast_message[64] = {0};
static ToastType_t toast_type = TOAST_INFO;
static uint32_t toast_show_time = 0;
static uint32_t toast_timeout = 0;
static uint8_t has_toast = 0;

// 对话框相关变量
static char dialog_title[32] = {0};
static char dialog_message[64] = {0};
static DialogType_t dialog_type = DIALOG_INFO;
static DialogCallback_t dialog_callback = NULL;
static uint8_t has_dialog = 0;
static uint8_t dialog_selected_button = 0; // 0: 确认, 1: 取消

// 界面过渡动画相关变量
static uint8_t is_animating = 0;
static UIScreen_t from_screen = SCREEN_MAIN;
static UIScreen_t to_screen = SCREEN_MAIN;
static uint8_t animation_progress = 0;
static uint8_t animation_type = 0; // 0: 滑动, 1: 淡入淡出

// ------------------【2】对外接口实现 ------------------
void UIManager_Init(void)
{
    // 初始化时，可做一些界面相关变量的清零
    // 例如：设置默认界面
    currentScreen = SCREEN_MAIN;
    
    // 初始化界面层级结构
    screenHierarchy.parentScreen = SCREEN_MAIN;
    screenHierarchy.subScreenIndex = 0;
    screenHierarchy.subScreenCount = 0;

    // 初始化弹窗和对话框相关变量
    has_toast = 0;
    has_dialog = 0;
    
    // 也可以直接先画一次
    UIManager_Update();
}

void UIManager_Update(void)
{
    static uint32_t last_update_time = 0;
    static bool display_busy = false;
    const uint32_t MIN_UPDATE_INTERVAL = 50;  // 最小更新间隔50ms
    
    // 如果显示正忙或时间间隔太短，直接返回
    if(display_busy || (HAL_GetTick() - last_update_time < MIN_UPDATE_INTERVAL)) {
        return;
    }
    
    display_busy = true;
    
    // 检查Toast是否需要自动关闭
    if (has_toast && toast_timeout > 0) {
        if (HAL_GetTick() - toast_show_time >= toast_timeout) {
            has_toast = 0;
        }
    }
    
    // 如果正在执行动画
    if (is_animating) {
        // 绘制动画帧
        UI_DrawAnimation(from_screen, to_screen, animation_progress, animation_type);
        
        // 更新动画进度
        animation_progress += 10; // 每次增加10%的进度
        
        // 动画完成
        if (animation_progress >= 100) {
            is_animating = 0;
            currentScreen = to_screen;
        }
    } 
    // 正常绘制当前界面
    else {
        // 调用当前界面的绘制函数
        if (currentScreen < SCREEN_MAX)
        {
            UIDrawFunc_t drawFunc = UI_GetDrawFunction(currentScreen);
            if (drawFunc != NULL)
            {
                drawFunc();
            }
            
            // 在基础界面上叠加显示Toast提示或对话框
            if (has_toast) {
                UI_DrawToast(toast_message, toast_type);
            }
            
            if (has_dialog) {
                UI_DrawDialog(dialog_title, dialog_message, dialog_type, dialog_selected_button);
            }
        }
    }
    
    last_update_time = HAL_GetTick();
    display_busy = false;
}

void UIManager_SwitchScreen(UIScreen_t screen)
{
    if (screen >= SCREEN_MAX)
        return;

    // 如果当前有对话框，则不切换界面
    if (has_dialog)
        return;
        
    // 是否使用动画效果
    if (1) {
        // 设置动画参数
        from_screen = currentScreen;
        to_screen = screen;
        animation_progress = 0;
        animation_type = 0; // 使用滑动效果
        is_animating = 1;
    } else {
        // 直接切换
        currentScreen = screen;
    }

    // 立即刷新
    UIManager_Update();
}

/**
 * @brief 获取当前界面
 * @return 当前界面的枚举值
 */
UIScreen_t UIManager_GetCurrentScreen(void)
{
    return currentScreen;
}

// 界面切换 触发切换到下一个界面或子页面
void UIManager_TriggerNextScreen(void)
{
    // 如果当前有对话框，则不切换界面
    if (has_dialog) {
        // 切换选中的按钮
        dialog_selected_button = !dialog_selected_button;
        UIManager_Update();
        return;
    }
    
    // 如果当前有Toast，关闭它
    if (has_toast) {
        has_toast = 0;
        UIManager_Update();
        return;
    }

    // 如果有子页面，切换到下一个子页面
    if (screenHierarchy.subScreenCount > 0) {
        screenHierarchy.subScreenIndex = (screenHierarchy.subScreenIndex + 1) % screenHierarchy.subScreenCount;
    } else {
        // 否则切换到下一个主界面，但不超过SCREEN_SWITCH_LIMIT
        currentScreen = (currentScreen + 1);
        if (currentScreen > SCREEN_SWITCH_LIMIT) {
            currentScreen = SCREEN_MAIN;  // 超过限制则回到主界面
        }
    }
    UIManager_Update();
}

// 界面切换 触发切换到上一个界面或子页面
void UIManager_TriggerPreviousScreen(void)
{
    // 如果当前有对话框，则不切换界面
    if (has_dialog) {
        // 切换选中的按钮
        dialog_selected_button = !dialog_selected_button;
        UIManager_Update();
        return;
    }
    
    // 如果当前有Toast，关闭它
    if (has_toast) {
        has_toast = 0;
        UIManager_Update();
        return;
    }

    // 如果有子页面，切换到上一个子页面
    if (screenHierarchy.subScreenCount > 0) {
        if (screenHierarchy.subScreenIndex > 0) {
            screenHierarchy.subScreenIndex--;
        } else {
            screenHierarchy.subScreenIndex = screenHierarchy.subScreenCount - 1;
        }
    } else {
        // 否则切换到上一个主界面，但不超过SCREEN_SWITCH_LIMIT
        if (currentScreen > SCREEN_MAIN) {
            currentScreen--;
        } else {
            currentScreen = SCREEN_SWITCH_LIMIT;  // 回到最后一个允许的界面
        }
    }
    UIManager_Update();
}

// 切换到指定的子页面
void UIManager_SwitchToSubScreen(UIScreen_t parentScreen, uint8_t subScreenIndex)
{
    // 保存当前屏幕为父屏幕
    screenHierarchy.parentScreen = currentScreen;
    
    // 设置子屏幕索引和总数
    screenHierarchy.subScreenIndex = subScreenIndex;
    
    // 切换到指定屏幕
    UIManager_SwitchScreen(parentScreen);
    
    UIManager_Update();
}

// 返回到父界面
void UIManager_ReturnToParentScreen(void)
{
    // 如果当前有对话框，则关闭对话框
    if (has_dialog) {
        has_dialog = 0;
        UIManager_Update();
        return;
    }
    
    // 如果当前有Toast，关闭它
    if (has_toast) {
        has_toast = 0;
        UIManager_Update();
        return;
    }

    // 切换到父界面
    currentScreen = screenHierarchy.parentScreen;
    screenHierarchy.subScreenIndex = 0;
    screenHierarchy.subScreenCount = 0;
    UIManager_Update();
}

// 获取当前界面的层级信息
UIScreenHierarchy_t UIManager_GetScreenHierarchy(void)
{
    return screenHierarchy;
}

// ------------------【3】新增弹窗和对话框相关功能实现 ------------------

/**
 * @brief 显示Toast提示
 */
void UIManager_ShowToast(const char* message, ToastType_t type, uint32_t timeout_ms)
{
    // 保存Toast信息
    strncpy(toast_message, message, sizeof(toast_message) - 1);
    toast_type = type;
    toast_show_time = HAL_GetTick();
    toast_timeout = timeout_ms;
    has_toast = 1;
    
    // 关闭对话框(如果有)
    has_dialog = 0;
    
    // 立即刷新显示
    UIManager_Update();
}

/**
 * @brief 显示模态对话框
 */
void UIManager_ShowDialog(const char* title, const char* message, DialogType_t type, DialogCallback_t callback)
{
    // 保存对话框信息
    strncpy(dialog_title, title, sizeof(dialog_title) - 1);
    strncpy(dialog_message, message, sizeof(dialog_message) - 1);
    dialog_type = type;
    dialog_callback = callback;
    dialog_selected_button = 0; // 默认选中确认按钮
    has_dialog = 1;
    
    // 关闭Toast(如果有)
    has_toast = 0;
    
    // 立即刷新显示
    UIManager_Update();
}

/**
 * @brief 关闭当前提示或对话框
 */
void UIManager_CloseOverlay(void)
{
    has_toast = 0;
    has_dialog = 0;
    UIManager_Update();
}

/**
 * @brief 对话框用户操作 - 确认
 */
void UIManager_DialogConfirm(void)
{
    if (has_dialog && dialog_callback) {
        dialog_callback(1); // 1表示确认
    }
    has_dialog = 0;
    UIManager_Update();
}

/**
 * @brief 对话框用户操作 - 取消
 */
void UIManager_DialogCancel(void)
{
    if (has_dialog && dialog_callback) {
        dialog_callback(0); // 0表示取消
    }
    has_dialog = 0;
    UIManager_Update();
}

/**
 * @brief 检查当前是否有提示或对话框显示
 */
uint8_t UIManager_HasOverlay(void)
{
    return has_toast || has_dialog;
}

// ------------------【4】各界面绘制函数实现 ------------------

// ========== 主界面 ==========
static void UI_DrawMain(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 5)) / 2, 0, "土壤湿度系统", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示土壤湿度
    sprintf(buffer, "湿度: %d%%", (int)(soil_moisture_value * 100 / 4095));
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示水泵状态
    sprintf(buffer, "水泵: %s", is_pump_on ? "开启" : "关闭");
    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示模式
    sprintf(buffer, "模式: %s", is_mode_auto ? "自动" : "手动");
    OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 状态界面 ==========
static void UI_DrawStatus(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "系统状态", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示电压
    sprintf(buffer, "电压: %.2fV", voltage_value / 1000.0f);
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示电流
    sprintf(buffer, "电流: %.2fmA", current_value / 1000.0f);
    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    // 显示功率
    sprintf(buffer, "功率: %.2fmW", power_value / 1000.0f);
    OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}

// ========== 数据界面 ==========
static void UI_DrawData(void)
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
static void UI_DrawEnv(void)
{
    char buffer[32];
    
    OLED_NewFrame();
    // 显示标题
    OLED_PrintString((128 - (16 * 4)) / 2, 0, "环境信息", &font16x16, OLED_COLOR_NORMAL);
    
    // 显示土壤湿度详细信息
    sprintf(buffer, "土壤湿度: %d%%", (int)(soil_moisture_value * 100 / 4095));
    OLED_PrintString(0, 16, buffer, &font16x16, OLED_COLOR_NORMAL);
    
//    // 显示今日降雨概率
//    sprintf(buffer, "今日降雨: %d%%", rain_probability_today);
//    OLED_PrintString(0, 32, buffer, &font16x16, OLED_COLOR_NORMAL);
//    
//    // 显示明日降雨概率
//    sprintf(buffer, "明日降雨: %d%%", rain_probability_tomorrow);
//    OLED_PrintString(0, 48, buffer, &font16x16, OLED_COLOR_NORMAL);
    
    OLED_ShowFrame();
}
