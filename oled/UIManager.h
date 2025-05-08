#ifndef __UI_MANAGER_H
#define __UI_MANAGER_H

#include <stdint.h>
#include "oled.h"
#include "main.h"
#include "UIDrawer.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************ 可自定义部分 ************************/
// 界面切换限制，定义可切换的最大界面数量
#define SCREEN_SWITCH_LIMIT   SCREEN_TEST  // 限制只能切换到数据界面为止

/**
 * @brief 界面ID的枚举，统一管理所有UI页面
 * @note  添加新界面时，在此处添加对应的枚举值
 *        注意保持SCREEN_MAX在最后，它用于界面总数的边界检查
 */
typedef enum
{
    /* 系统界面 */
    SCREEN_INIT = 0,              // 初始化界面
    SCREEN_LOADING_NETDATA,       // 网络数据加载界面
    
    /* 功能界面 */
    SCREEN_MAIN,                  // 主界面
    SCREEN_STATUS,                // 状态界面
    SCREEN_DATA,                  // 数据界面
    SCREEN_ENV,                   // 环境信息界面


    SCREEN_MAX                    // 界面总数
} UIScreen_t;                     // 界面类型定义，用于标识不同的界面

/************************ 通用功能部分 ************************/
/**
 * @brief UI管理器基础功能函数
 * @note  这些函数提供了基础的UI管理功能，通常不需要修改
 */
void UIManager_Init(void);                    // 初始化UI管理器
void UIManager_Update(void);                  // 更新当前界面显示
void UIManager_SetScreen(UIScreen_t screen);     // 切换到指定界面
UIScreen_t UIManager_GetCurrentScreen(void);    // 获取当前界面ID

/************************ 界面层级管理 ************************/
/**
 * @brief 界面层级结构定义
 * @note  用于实现多级菜单功能，管理界面的父子关系
 *        - currentScreen: 当前界面的ID
 *        - previousScreen: 上一个界面的ID
 *        - subScreenIndex: 当前显示的子界面编号
 *        - subScreenCount: 当前界面下的子界面总数
 */
typedef struct {
    UIScreen_t currentScreen;      // 当前界面ID
    UIScreen_t previousScreen;       // 上一个界面ID
    uint8_t subScreenIndex;         // 子界面索引
    uint8_t subScreenCount;         // 子界面总数
} UIScreenHierarchy_t;

/**
 * @brief 界面导航功能函数
 * @note  这些函数提供了界面间的导航功能：
 *        - SetSubScreenIndex: 设置子界面索引
 *        - GetSubScreenIndex: 获取子界面索引
 */
void UIManager_SetSubScreenIndex(uint8_t index);
uint8_t UIManager_GetSubScreenIndex(void);

/************************ 新增弹窗和模态对话框功能 ************************/
// 提示类型定义
typedef enum {
    TOAST_INFO,     // 普通信息提示
    TOAST_SUCCESS,  // 成功提示
    TOAST_WARNING,  // 警告提示
    TOAST_ERROR     // 错误提示
} ToastType_t;

// 模态对话框类型定义
typedef enum {
    DIALOG_INFO,    // 信息对话框
    DIALOG_CONFIRM, // 确认对话框
    DIALOG_INPUT    // 输入对话框(预留)
} DialogType_t;

// 对话框回调函数定义
typedef void (*DialogCallback_t)(uint8_t result);

/**
 * @brief 显示Toast提示
 * @param message 提示内容
 * @param type 提示类型
 * @param timeout_ms 自动消失时间(毫秒)，0表示不自动消失
 */
void UIManager_ShowToast(const char* message, ToastType_t type, uint32_t timeout_ms);

/**
 * @brief 显示模态对话框
 * @param title 对话框标题
 * @param message 对话框内容
 * @param type 对话框类型
 * @param callback 用户操作后的回调函数
 */
void UIManager_ShowDialog(const char* title, const char* message, DialogType_t type, DialogCallback_t callback);

/**
 * @brief 关闭当前提示或对话框
 */
void UIManager_CloseOverlay(void);

/**
 * @brief 对话框用户操作 - 确认
 */
void UIManager_DialogConfirm(void);

/**
 * @brief 对话框用户操作 - 取消
 */
void UIManager_DialogCancel(void);

/**
 * @brief 检查当前是否有提示或对话框显示
 * @return 1:有 0:没有
 */
uint8_t UIManager_HasOverlay(void);

// 界面配置结构体
typedef struct {
    const char* title;           // 界面标题
    UIDrawFunc_t drawFunc;       // 绘制函数
    uint8_t subScreenCount;      // 子界面数量
    const char** subScreenTitles; // 子界面标题数组
} UIScreenConfig_t;

// 界面层级结构
typedef struct {
    UIScreen_t currentScreen;
    UIScreen_t previousScreen;
    uint8_t subScreenIndex;
    uint8_t subScreenCount;
} UIScreenHierarchy_t;

// 界面管理函数
void UIManager_SetScreenHierarchy(const UIScreenHierarchy_t* hierarchy);
UIScreenHierarchy_t UIManager_GetScreenHierarchy(void);

// 界面配置函数
void UIManager_RegisterScreen(UIScreen_t screen, const UIScreenConfig_t* config);
const UIScreenConfig_t* UIManager_GetScreenConfig(UIScreen_t screen);

#ifdef __cplusplus
}
#endif

#endif // __UI_MANAGER_H
