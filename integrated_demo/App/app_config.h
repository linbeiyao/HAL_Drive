/**
 * @file app_config.h
 * @brief 应用程序配置文件
 * @details 配置各个驱动模块的参数和功能开关
 */
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含标准头文件 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* 包含HAL库头文件 */
#include "main.h"

/*====================== OLED显示配置 ======================*/
/* OLED显示配置 */
#define OLED_USE_I2C              1       // 使用I2C接口
#define OLED_I2C_ADDRESS          0x78    // OLED I2C地址
#define OLED_WIDTH                128     // OLED宽度(像素)
#define OLED_HEIGHT               64      // OLED高度(像素)
#define OLED_BUFFER_SIZE          1024    // OLED缓冲区大小

/* UI界面配置 */
#define UI_MAX_SCREENS            8       // 最大界面数量
#define UI_MAX_PAGES_PER_SCREEN   4       // 每个界面最大页数
#define UI_MAX_MENU_ITEMS         6       // 菜单最大项目数
#define UI_UPDATE_INTERVAL        100     // UI更新间隔(ms)
#define UI_ANIMATION_ENABLE       1       // 启用动画效果

/*====================== 按键配置 ======================*/
/* 按键数量和引脚定义 */
#define BTN_COUNT                 4       // 按键数量
#define BTN_UP_GPIO               GPIOA   // 上按键GPIO
#define BTN_UP_PIN                GPIO_PIN_0  // 上按键引脚
#define BTN_DOWN_GPIO             GPIOA   // 下按键GPIO
#define BTN_DOWN_PIN              GPIO_PIN_1  // 下按键引脚
#define BTN_OK_GPIO               GPIOA   // 确认按键GPIO
#define BTN_OK_PIN                GPIO_PIN_2  // 确认按键引脚
#define BTN_BACK_GPIO             GPIOA   // 返回按键GPIO
#define BTN_BACK_PIN              GPIO_PIN_3  // 返回按键引脚

/* 按键参数配置 */
#define BTN_DEBOUNCE_TIME         20      // 去抖时间(ms)
#define BTN_LONG_PRESS_TIME       1000    // 长按时间(ms)
#define BTN_DOUBLE_CLICK_TIME     300     // 双击间隔(ms)
#define BTN_REPEAT_TIME           150     // 重复触发间隔(ms)
#define BTN_REPEAT_START_TIME     500     // 开始重复前等待时间(ms)

/*====================== 状态机配置 ======================*/
/* 状态机参数 */
#define FSM_MAX_STATES            10      // 最大状态数
#define FSM_MAX_TRANSITIONS       20      // 最大转换数
#define FSM_DEFAULT_TIMEOUT       5000    // 默认超时时间(ms)
#define FSM_UPDATE_INTERVAL       10      // 状态机更新间隔(ms)

/* 应用状态定义 */
typedef enum {
    STATE_INIT = 0,               // 初始化状态
    STATE_IDLE,                   // 空闲状态
    STATE_MENU,                   // 菜单状态
    STATE_DATA_DISPLAY,           // 数据显示状态
    STATE_SETTINGS,               // 设置状态
    STATE_ALARM,                  // 告警状态
    STATE_SLEEP,                  // 休眠状态
    STATE_ERROR                   // 错误状态
} AppState_t;

/* 应用事件定义 */
typedef enum {
    EVENT_NONE = 0,               // 无事件
    EVENT_INIT_DONE,              // 初始化完成事件
    EVENT_KEY_PRESS,              // 按键按下事件
    EVENT_TIMEOUT,                // 超时事件
    EVENT_DATA_UPDATE,            // 数据更新事件
    EVENT_ERROR,                  // 错误事件
    EVENT_ALARM,                  // 告警事件
    EVENT_SLEEP,                  // 进入休眠事件
    EVENT_WAKE_UP                 // 唤醒事件
} AppEvent_t;

/*====================== 任务管理器配置 ======================*/
/* 任务配置 */
#define TASK_MAX_COUNT            10      // 最大任务数量
#define TASK_TICK_MS              1       // 任务时钟节拍(ms)
#define TASK_STATS_ENABLE         1       // 启用任务统计

/* 任务优先级定义 */
#define TASK_PRIO_HIGH            1       // 高优先级
#define TASK_PRIO_MEDIUM          5       // 中优先级
#define TASK_PRIO_LOW             10      // 低优先级
#define TASK_PRIO_IDLE            255     // 空闲优先级

/* 任务周期定义 */
#define TASK_PERIOD_UI            100     // UI任务周期(ms)
#define TASK_PERIOD_KEY           10      // 按键任务周期(ms)
#define TASK_PERIOD_DATA          500     // 数据任务周期(ms)
#define TASK_PERIOD_MONITOR       1000    // 监控任务周期(ms)
#define TASK_PERIOD_LED           200     // LED任务周期(ms)

/*====================== 其他配置 ======================*/
/* 调试配置 */
#define DEBUG_ENABLE              1       // 启用调试输出
#define DEBUG_UART                USART1  // 调试串口

/* 系统配置 */
#define SYS_CLOCK_FREQ            72000000    // 系统时钟频率(Hz)
#define USE_HAL_DELAY             1           // 使用HAL_Delay函数

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONFIG_H */ 