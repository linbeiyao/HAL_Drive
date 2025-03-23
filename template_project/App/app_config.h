/**
  ******************************************************************************
  * @file           : app_config.h
  * @brief          : 应用层配置文件
  * @author         : HAL_Drive团队
  * @version        : v1.0.0
  * @date           : 2023-01-01
  ******************************************************************************
  */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* 应用层版本信息 -------------------------------------------------------*/
#define APP_VERSION_MAJOR     1
#define APP_VERSION_MINOR     0
#define APP_VERSION_PATCH     0

/* 按钮配置 -------------------------------------------------------------*/
#define BTN_MAX_COUNT         5       /* 最大支持按钮数量 */
#define BTN_DEBOUNCE_TIME     20      /* 去抖时间(ms) */
#define BTN_CLICK_TIME        200     /* 单击最大时间(ms) */
#define BTN_DCLICK_TIME       300     /* 双击间隔时间(ms) */
#define BTN_LONG_TIME         1000    /* 长按时间(ms) */
#define BTN_REPEAT_TIME       100     /* 连发间隔时间(ms) */
#define BTN_REPEAT_START_TIME 500     /* 连发开始时间(ms) */
#define BTN_COMBO_TIME        300     /* 组合按键检测时间窗口(ms) */

/* OLED显示屏配置 --------------------------------------------------------*/
#define OLED_WIDTH            128     /* OLED宽度像素 */
#define OLED_HEIGHT           64      /* OLED高度像素 */
#define OLED_USE_I2C          1       /* 使用I2C通信 */
#define OLED_ADDRESS          0x78    /* OLED I2C地址 */

/* FSM状态机配置 ---------------------------------------------------------*/
#define FSM_MAX_STATES        10      /* 最大状态数 */
#define FSM_MAX_TRANSITIONS   20      /* 最大转换数 */
#define FSM_DEFAULT_TIMEOUT   0       /* 默认超时时间(ms)，0表示无超时 */

/* 任务管理配置 ---------------------------------------------------------*/
#define TASK_MAX_COUNT        10      /* 最大任务数量 */
#define TASK_MIN_PERIOD       1       /* 最小任务周期(ms) */
#define TASK_MAX_PRIORITY     5       /* 最大任务优先级 */
#define TASK_NAME_MAX_LEN     16      /* 任务名称最大长度 */
#define TASK_ENABLE_STATS     1       /* 启用任务统计 */
#define TASK_WATCHDOG_ENABLE  1       /* 启用任务看门狗 */
#define TASK_WATCHDOG_TIMEOUT 1000    /* 任务看门狗超时时间(ms) */

/* 应用层常量定义 --------------------------------------------------------*/
/* 应用状态ID定义 */
typedef enum {
    APP_STATE_IDLE = 0,               /* 空闲状态 */
    APP_STATE_MENU,                   /* 菜单状态 */
    APP_STATE_SETTING,                /* 设置状态 */
    APP_STATE_RUNNING,                /* 运行状态 */
    APP_STATE_ALARM,                  /* 告警状态 */
    APP_STATE_MAX
} AppStateId_t;

/* 应用事件ID定义 */
typedef enum {
    APP_EVENT_NONE = 0,               /* 无事件 */
    APP_EVENT_KEY_PRESS,              /* 按键按下事件 */
    APP_EVENT_KEY_RELEASE,            /* 按键释放事件 */
    APP_EVENT_TIMEOUT,                /* 超时事件 */
    APP_EVENT_DATA_READY,             /* 数据就绪事件 */
    APP_EVENT_ERROR,                  /* 错误事件 */
    APP_EVENT_MAX
} AppEventId_t;

/* 屏幕ID定义 */
typedef enum {
    SCREEN_MAIN = 0,                  /* 主界面 */
    SCREEN_MENU,                      /* 菜单界面 */
    SCREEN_SETTING,                   /* 设置界面 */
    SCREEN_INFO,                      /* 信息界面 */
    SCREEN_MAX
} ScreenId_t;

/* 任务优先级定义 */
typedef enum {
    TASK_PRIO_IDLE = 0,               /* 空闲优先级 */
    TASK_PRIO_LOW,                    /* 低优先级 */
    TASK_PRIO_NORMAL,                 /* 正常优先级 */
    TASK_PRIO_HIGH,                   /* 高优先级 */
    TASK_PRIO_CRITICAL                /* 关键优先级 */
} TaskPriority_t;

/* 功能开关配置 ---------------------------------------------------------*/
#define USE_BUTTON_FSM_MODE       1   /* 使用按钮FSM模式 */
#define USE_BUTTON_COMBO          1   /* 使用组合按键功能 */
#define USE_OLED_ANIMATION        1   /* 使用OLED动画效果 */
#define USE_ERROR_LOGGING         1   /* 使用错误日志记录 */
#define USE_DEBUG_MODE            1   /* 使用调试模式 */

/* 定时器配置 -----------------------------------------------------------*/
#define SYS_TICK_MS               1   /* 系统时钟节拍(ms) */
#define UI_UPDATE_PERIOD          100 /* UI更新周期(ms) */
#define SENSOR_READ_PERIOD        500 /* 传感器读取周期(ms) */

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONFIG_H */ 