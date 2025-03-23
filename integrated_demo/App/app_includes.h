/**
 * @file app_includes.h
 * @brief 应用程序包含文件
 * @details 包含所有使用到的驱动库头文件
 */
#ifndef __APP_INCLUDES_H
#define __APP_INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif

/* 标准库包含 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HAL驱动库包含 */
#include "main.h"

/* 应用配置包含 */
#include "app_config.h"

/* 驱动库包含 */
// OLED显示驱动
#include "oled.h"
#include "UIManager.h"
#include "UIDrawer.h"
#include "font.h"

// 按键驱动
#include "button.h"

// 状态机驱动
#include "fsm.h"

// 任务管理器
#include "taskmanager.h"

/* 应用层包含 */
#include "app_fsm.h"

/**
 * @brief 应用初始化函数
 * @return 0成功，非0失败
 */
int App_Init(void);

/**
 * @brief 应用主循环函数
 */
void App_MainLoop(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_INCLUDES_H */ 