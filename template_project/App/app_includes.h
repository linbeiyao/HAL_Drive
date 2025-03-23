/**
  ******************************************************************************
  * @file           : app_includes.h
  * @brief          : 应用层包含文件
  * @author         : HAL_Drive团队
  * @version        : v1.0.0
  * @date           : 2023-01-01
  ******************************************************************************
  */

#ifndef __APP_INCLUDES_H
#define __APP_INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif

/* 系统头文件 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* 项目配置 */
#include "main.h"
#include "app_config.h"

/* 按钮库 */
#include "../../button/button.h"

/* FSM状态机库 */
#include "../../fsm/fsm.h"

/* OLED显示库 */
#include "../../oled/oled.h"
#include "../../oled/font.h"
#include "../../oled/UIManager.h"
#include "../../oled/UIDrawer.h"

/* 任务管理库 */
#include "../../taskmanager/taskmanager.h"

#ifdef __cplusplus
}
#endif

#endif /* __APP_INCLUDES_H */ 