/**
 * @file button.h
 * @brief 高级按钮处理库，支持多种按键事件和状态机集成
 * @author HAL_Drive项目组
 * @version 2.0
 * @date 2023-03-23
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HAL库头文件，实际项目中应根据具体平台调整 */
#include "main.h"  // 包含GPIO相关定义

/* 配置参数 */
#define BTN_CONFIG_MAX_BUTTONS        16   // 最大支持按钮数
#define BTN_CONFIG_DEBOUNCE_TIME      20   // 去抖时间(ms)
#define BTN_CONFIG_CLICK_TIME         300  // 单击最大时间(ms)
#define BTN_CONFIG_DOUBLE_CLICK_TIME  300  // 双击间隔时间(ms)
#define BTN_CONFIG_LONG_PRESS_TIME    1000 // 长按时间(ms)
#define BTN_CONFIG_REPEAT_TIME        100  // 重复触发间隔(ms)
#define BTN_CONFIG_REPEAT_START_TIME  500  // 重复触发开始时间(ms)
#define BTN_CONFIG_COMBO_TIME         300  // 组合键时间窗口(ms)

/**
 * @brief 按钮检测模式
 */
typedef enum {
    BTN_DETECT_POLLING = 0,  // 轮询模式
    BTN_DETECT_INTERRUPT     // 中断模式
} BTN_DetectMode_t;

/**
 * @brief 按钮状态
 */
typedef enum {
    BTN_STATE_RELEASED = 0,  // 释放状态
    BTN_STATE_PRESSED        // 按下状态
} BTN_State_t;

/**
 * @brief 按钮工作模式
 */
typedef enum {
    BTN_MODE_STANDARD = 0,   // 标准模式：支持所有事件
    BTN_MODE_INSTANT,        // 即时模式：仅支持按下/释放
    BTN_MODE_REPEAT,         // 重复模式：支持连续重复触发
    BTN_MODE_FSM             // FSM模式：与状态机集成
} BTN_Mode_t;

/**
 * @brief 按钮事件
 */
typedef enum {
    BTN_EVENT_PRESSED = 0,       // 按下事件
    BTN_EVENT_RELEASED,          // 释放事件
    BTN_EVENT_CLICK,             // 点击事件(通用)
    BTN_EVENT_SINGLE_CLICK,      // 单击事件
    BTN_EVENT_DOUBLE_CLICK,      // 双击事件
    BTN_EVENT_TRIPLE_CLICK,      // 三击事件
    BTN_EVENT_LONG_PRESS,        // 长按事件
    BTN_EVENT_LONG_RELEASED,     // 长按释放
    BTN_EVENT_REPEAT,            // 重复触发
    BTN_EVENT_COMBO              // 组合键事件
} BTN_Event_t;

/**
 * @brief 按钮ID类型
 */
typedef uint8_t BTN_ID_t;

/**
 * @brief 组合键掩码类型
 */
typedef uint32_t BTN_ComboMask_t;

/**
 * @brief 获取按钮掩码宏
 */
#define BTN_MASK(btn_id) (1UL << (btn_id))

/**
 * @brief 按钮配置
 */
typedef struct {
    uint16_t debounce_time;       // 去抖时间(ms)
    uint16_t click_time;          // 单击最大时间(ms)
    uint16_t double_click_time;   // 双击间隔时间(ms)
    uint16_t long_press_time;     // 长按时间(ms)
    uint16_t repeat_time;         // 重复触发间隔(ms)
    uint16_t repeat_start_time;   // 重复触发开始时间(ms)
    uint16_t combo_time;          // 组合键时间窗口(ms)
    BTN_Mode_t mode;              // 按钮工作模式
    BTN_DetectMode_t detect_mode; // 按钮检测模式
    uint8_t active_level;         // 有效电平(0:低电平有效, 1:高电平有效)
    uint8_t enable_repeat;        // 启用重复触发
    uint8_t enable_long_press;    // 启用长按
    uint8_t enable_combo;         // 启用组合键
} BTN_Config_t;

/**
 * @brief 按钮回调函数类型
 */
typedef void (*BTN_Callback_t)(BTN_ID_t button_id, BTN_Event_t event, void* user_data);

/**
 * @brief 按钮状态机事件处理函数类型
 */
typedef void (*BTN_FSMEventHandler_t)(BTN_ID_t button_id, BTN_Event_t event, void* fsm_instance, void* user_data);

/**
 * @brief 按钮硬件读取函数类型
 */
typedef BTN_State_t (*BTN_HwReadFunc_t)(void* param);

/**
 * @brief 按钮硬件信息
 */
typedef struct {
    BTN_HwReadFunc_t read_func;  // 硬件读取函数
    GPIO_TypeDef* port;          // GPIO端口
    uint16_t pin;                // GPIO引脚
    uint8_t active_level;        // 有效电平
    void* param;                 // 硬件参数
    uint32_t exti_line;          // 外部中断线(中断模式下使用)
} BTN_Hardware_t;

/**
 * @brief 按钮FSM信息
 */
typedef struct {
    void* fsm_instance;            // FSM实例
    BTN_FSMEventHandler_t handler; // FSM事件处理函数
} BTN_FSM_t;

/**
 * @brief 按钮私有数据
 */
typedef struct {
    BTN_State_t state;           // 当前状态
    BTN_State_t last_state;      // 上次状态
    uint32_t last_debounce_time; // 上次去抖时间
    uint32_t press_time;         // 按下时间
    uint32_t release_time;       // 释放时间
    uint32_t last_repeat_time;   // 上次重复触发时间
    uint8_t click_count;         // 点击计数
    uint8_t is_pressed;          // 是否处于按下状态
    uint8_t is_long_press;       // 是否处于长按状态
} BTN_Private_t;

/**
 * @brief 按钮句柄
 */
typedef struct {
    BTN_ID_t id;                 // 按钮ID
    char name[16];               // 按钮名称
    BTN_Hardware_t hardware;     // 硬件信息
    BTN_Config_t config;         // 配置信息
    BTN_Private_t private;       // 私有数据
    BTN_FSM_t fsm;               // FSM信息
    BTN_Callback_t callback;     // 回调函数
    void* user_data;             // 用户数据
} BTN_Handle_t;

/**
 * @brief 组合键配置
 */
typedef struct {
    BTN_ComboMask_t mask;        // 组合键掩码
    BTN_Callback_t callback;     // 回调函数
    void* user_data;             // 用户数据
    uint16_t time_window;        // 时间窗口(ms)，0表示使用默认值
} BTN_ComboConfig_t;

/* API函数 */

/**
 * @brief 获取默认按钮配置
 */
void BTN_GetDefaultConfig(BTN_Config_t* config);

/**
 * @brief 初始化按钮库
 */
void BTN_Init(void);

/**
 * @brief 清理按钮库资源
 */
void BTN_Deinit(void);

/**
 * @brief 查找按钮句柄
 */
BTN_Handle_t* BTN_GetHandleByID(BTN_ID_t id);

/**
 * @brief 创建一个新按钮
 */
BTN_Handle_t* BTN_Create(BTN_ID_t id, 
                        const char* name, 
                        BTN_Hardware_t* hardware, 
                        BTN_Config_t* config, 
                        BTN_Callback_t callback, 
                        void* user_data);

/**
 * @brief 创建一个GPIO按钮
 */
BTN_Handle_t* BTN_CreateGPIO(BTN_ID_t id, 
                           const char* name, 
                           GPIO_TypeDef* port, 
                           uint16_t pin, 
                           uint8_t active_level, 
                           BTN_Config_t* config, 
                           BTN_Callback_t callback, 
                           void* user_data);

/**
 * @brief 销毁按钮
 */
int BTN_Destroy(BTN_Handle_t* handle);

/**
 * @brief 获取按钮当前状态
 */
BTN_State_t BTN_GetState(BTN_Handle_t* handle);

/**
 * @brief 设置按钮回调函数
 */
int BTN_SetCallback(BTN_Handle_t* handle, BTN_Callback_t callback, void* user_data);

/**
 * @brief 更新按钮配置
 */
int BTN_UpdateConfig(BTN_Handle_t* handle, BTN_Config_t* config);

/**
 * @brief 设置按钮工作模式
 */
int BTN_SetMode(BTN_Handle_t* handle, BTN_Mode_t mode);

/**
 * @brief 绑定FSM处理函数
 */
int BTN_BindFSM(BTN_Handle_t* handle, void* fsm_instance, BTN_FSMEventHandler_t handler);

/**
 * @brief 解除FSM绑定
 */
int BTN_UnbindFSM(BTN_Handle_t* handle);

/**
 * @brief 注册组合键
 */
int BTN_RegisterCombo(BTN_ComboConfig_t* combo_config);

/**
 * @brief 取消注册组合键
 */
int BTN_UnregisterCombo(BTN_ComboMask_t mask);

/**
 * @brief 检查按钮是否处于按下状态
 */
uint8_t BTN_IsPressed(BTN_Handle_t* handle);

/**
 * @brief 检查按钮是否处于长按状态
 */
uint8_t BTN_IsLongPressed(BTN_Handle_t* handle);

/**
 * @brief 中断处理函数
 */
void BTN_HandleInterrupt(uint16_t gpio_pin);

/**
 * @brief 按钮处理函数
 */
void BTN_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H */
