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

// 当前界面状态
static UIScreen_t s_currentScreen = SCREEN_INIT;
static UIScreen_t s_previousScreen = SCREEN_INIT;
static uint8_t s_subScreenIndex = 0;
static uint8_t s_subScreenCount = 0;

// 界面配置数组
static UIScreenConfig_t s_screenConfigs[SCREEN_MAX] = {0};

// ------------------【1】定义当前界面 ------------------

static UIScreen_t currentScreen = SCREEN_MAIN;


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
    s_currentScreen = SCREEN_INIT;
    s_previousScreen = SCREEN_INIT;
    s_subScreenIndex = 0;
    s_subScreenCount = 0;
    
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
        const UIScreenConfig_t* config = UIManager_GetScreenConfig(s_currentScreen);
        if (config && config->drawFunc) {
            config->drawFunc();
        }
        
        // 在基础界面上叠加显示Toast提示或对话框
        if (has_toast) {
            UI_DrawToast(toast_message, toast_type);
        }
        
        if (has_dialog) {
            UI_DrawDialog(dialog_title, dialog_message, dialog_type, dialog_selected_button);
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
    return s_currentScreen;
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
    if (s_subScreenCount > 0) {
        s_subScreenIndex = (s_subScreenIndex + 1) % s_subScreenCount;
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
    if (s_subScreenCount > 0) {
        if (s_subScreenIndex > 0) {
            s_subScreenIndex--;
        } else {
            s_subScreenIndex = s_subScreenCount - 1;
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
    s_subScreenIndex = subScreenIndex;
    
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
    s_subScreenIndex = 0;
    s_subScreenCount = 0;
    UIManager_Update();
}

// 获取当前界面的层级信息
UIScreenHierarchy_t UIManager_GetScreenHierarchy(void)
{
    UIScreenHierarchy_t hierarchy;
    hierarchy.currentScreen = s_currentScreen;
    hierarchy.previousScreen = s_previousScreen;
    hierarchy.subScreenIndex = s_subScreenIndex;
    hierarchy.subScreenCount = s_subScreenCount;
    return hierarchy;
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

// 切换到指定界面
void UIManager_SetScreen(UIScreen_t screen)
{
    if (screen < SCREEN_MAX) {
        s_previousScreen = s_currentScreen;
        s_currentScreen = screen;
        s_subScreenIndex = 0;
        
        const UIScreenConfig_t* config = UIManager_GetScreenConfig(screen);
        if (config) {
            s_subScreenCount = config->subScreenCount;
        }
    }
}

// 设置子界面索引
void UIManager_SetSubScreenIndex(uint8_t index)
{
    if (index < s_subScreenCount) {
        s_subScreenIndex = index;
    }
}

// 获取子界面索引
uint8_t UIManager_GetSubScreenIndex(void)
{
    return s_subScreenIndex;
}

// 注册界面配置
void UIManager_RegisterScreen(UIScreen_t screen, const UIScreenConfig_t* config)
{
    if (screen < SCREEN_MAX && config) {
        s_screenConfigs[screen] = *config;
    }
}

// 获取界面配置
const UIScreenConfig_t* UIManager_GetScreenConfig(UIScreen_t screen)
{
    if (screen < SCREEN_MAX) {
        return &s_screenConfigs[screen];
    }
    return NULL;
}

