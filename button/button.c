/**
 * @file button.c
 * @brief 高级按钮处理库实现，支持多种按键事件和状态机集成
 * @author HAL_Drive项目组
 * @version 2.0
 * @date 2023-03-23
 */

#include "button.h"
#include <stdio.h>   // printf等函数
#include <string.h>
#include <stdlib.h>

/* 按钮管理 */
static BTN_Handle_t* s_btn_handles[BTN_CONFIG_MAX_BUTTONS] = {NULL};
static uint8_t s_btn_count = 0;

/* 组合键管理 */
#define MAX_COMBO_KEYS 8
static BTN_ComboConfig_t s_combo_configs[MAX_COMBO_KEYS] = {0};
static uint8_t s_combo_count = 0;
static BTN_ComboMask_t s_current_pressed_mask = 0;
static uint32_t s_combo_start_time = 0;

/**
 * @brief 获取默认按钮配置
 */
void BTN_GetDefaultConfig(BTN_Config_t* config)
{
    if (!config) return;
    
    memset(config, 0, sizeof(BTN_Config_t));
    config->debounce_time = BTN_CONFIG_DEBOUNCE_TIME;
    config->click_time = BTN_CONFIG_CLICK_TIME;
    config->double_click_time = BTN_CONFIG_DOUBLE_CLICK_TIME;
    config->long_press_time = BTN_CONFIG_LONG_PRESS_TIME;
    config->repeat_time = BTN_CONFIG_REPEAT_TIME;
    config->repeat_start_time = BTN_CONFIG_REPEAT_START_TIME;
    config->combo_time = BTN_CONFIG_COMBO_TIME;
    config->mode = BTN_MODE_STANDARD;
    config->detect_mode = BTN_DETECT_POLLING;
    config->active_level = 0; // 低电平有效
    config->enable_repeat = 1;
    config->enable_long_press = 1;
    config->enable_combo = 1;
}

/**
 * @brief 初始化按钮库
 */
void BTN_Init(void)
{
    // 清空按钮句柄数组
    memset(s_btn_handles, 0, sizeof(s_btn_handles));
    s_btn_count = 0;
    
    // 清空组合键配置
    memset(s_combo_configs, 0, sizeof(s_combo_configs));
    s_combo_count = 0;
    s_current_pressed_mask = 0;
    s_combo_start_time = 0;
}

/**
 * @brief 清理按钮库资源
 */
void BTN_Deinit(void)
{
    // 释放所有按钮资源
    for (uint8_t i = 0; i < BTN_CONFIG_MAX_BUTTONS; i++) {
        if (s_btn_handles[i]) {
            free(s_btn_handles[i]);
            s_btn_handles[i] = NULL;
        }
    }
    s_btn_count = 0;
    
    // 清空组合键配置
    memset(s_combo_configs, 0, sizeof(s_combo_configs));
    s_combo_count = 0;
}

/**
 * @brief 查找按钮句柄
 */
BTN_Handle_t* BTN_GetHandleByID(BTN_ID_t id)
{
    for (uint8_t i = 0; i < BTN_CONFIG_MAX_BUTTONS; i++) {
        if (s_btn_handles[i] && s_btn_handles[i]->id == id) {
            return s_btn_handles[i];
        }
    }
    return NULL;
}

/**
 * @brief 创建一个新按钮
 */
BTN_Handle_t* BTN_Create(BTN_ID_t id, 
                         const char* name, 
                         BTN_Hardware_t* hardware, 
                         BTN_Config_t* config, 
                         BTN_Callback_t callback, 
                         void* user_data)
{
    // 检查参数
    if (!hardware || !hardware->read_func) {
        return NULL;
    }
    
    // 检查ID是否已存在
    if (BTN_GetHandleByID(id)) {
        return NULL;
    }
    
    // 检查按钮数量
    if (s_btn_count >= BTN_CONFIG_MAX_BUTTONS) {
        return NULL;
    }
    
    // 分配内存
    BTN_Handle_t* handle = (BTN_Handle_t*)malloc(sizeof(BTN_Handle_t));
    if (!handle) {
        return NULL;
    }
    
    // 初始化按钮结构
    memset(handle, 0, sizeof(BTN_Handle_t));
    handle->id = id;
    
    // 设置按钮名称
    if (name) {
        strncpy(handle->name, name, sizeof(handle->name) - 1);
    } else {
        snprintf(handle->name, sizeof(handle->name), "BTN_%d", id);
    }
    
    // 设置硬件信息
    memcpy(&handle->hardware, hardware, sizeof(BTN_Hardware_t));
    
    // 设置配置
    if (config) {
        memcpy(&handle->config, config, sizeof(BTN_Config_t));
    } else {
        BTN_GetDefaultConfig(&handle->config);
    }
    
    // 设置回调函数
    handle->callback = callback;
    handle->user_data = user_data;
    
    // 初始化私有数据
    handle->private.state = BTN_STATE_RELEASED;
    handle->private.last_state = BTN_STATE_RELEASED;
    handle->private.last_debounce_time = HAL_GetTick();
    
    // 保存按钮句柄
    for (uint8_t i = 0; i < BTN_CONFIG_MAX_BUTTONS; i++) {
        if (!s_btn_handles[i]) {
            s_btn_handles[i] = handle;
            s_btn_count++;
            break;
        }
    }
    
    return handle;
}

/**
 * @brief 读取GPIO按钮状态
 */
static BTN_State_t BTN_ReadGPIO(void* param)
{
    BTN_Hardware_t* hw = (BTN_Hardware_t*)param;
    GPIO_PinState pinState = HAL_GPIO_ReadPin(hw->port, hw->pin);
    
    // 根据有效电平确定按钮状态
    if ((hw->active_level == 0 && pinState == GPIO_PIN_RESET) ||
        (hw->active_level == 1 && pinState == GPIO_PIN_SET)) {
        return BTN_STATE_PRESSED;
    } else {
        return BTN_STATE_RELEASED;
    }
}

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
                           void* user_data)
{
    // 创建硬件信息
    BTN_Hardware_t hardware;
    memset(&hardware, 0, sizeof(hardware));
    hardware.read_func = BTN_ReadGPIO;
    hardware.port = port;
    hardware.pin = pin;
    hardware.param = &hardware; // 参数指向自身
    
    // 如果是中断模式，需要额外配置
    if (config && config->detect_mode == BTN_DETECT_INTERRUPT) {
        // 这里需要根据具体平台配置中断线
        // 例如：hardware.exti_line = __HAL_GPIO_EXTI_GET_IT(pin);
    }
    
    // 创建按钮
    return BTN_Create(id, name, &hardware, config, callback, user_data);
}

/**
 * @brief 销毁按钮
 */
int BTN_Destroy(BTN_Handle_t* handle)
{
    if (!handle) return -1;
    
    // 查找并移除按钮
    for (uint8_t i = 0; i < BTN_CONFIG_MAX_BUTTONS; i++) {
        if (s_btn_handles[i] == handle) {
            free(s_btn_handles[i]);
            s_btn_handles[i] = NULL;
            s_btn_count--;
            return 0;
        }
    }
    
    return -1;
}

/**
 * @brief 获取按钮当前状态
 */
BTN_State_t BTN_GetState(BTN_Handle_t* handle)
{
    if (!handle) return BTN_STATE_RELEASED;
    return handle->private.state;
}

/**
 * @brief 设置按钮回调函数
 */
int BTN_SetCallback(BTN_Handle_t* handle, BTN_Callback_t callback, void* user_data)
{
    if (!handle) return -1;
    
    handle->callback = callback;
    handle->user_data = user_data;
    
    return 0;
}

/**
 * @brief 更新按钮配置
 */
int BTN_UpdateConfig(BTN_Handle_t* handle, BTN_Config_t* config)
{
    if (!handle || !config) return -1;
    
    memcpy(&handle->config, config, sizeof(BTN_Config_t));
    
    return 0;
}

/**
 * @brief 设置按钮工作模式
 */
int BTN_SetMode(BTN_Handle_t* handle, BTN_Mode_t mode)
{
    if (!handle) return -1;
    
    handle->config.mode = mode;
    
    return 0;
}

/**
 * @brief 绑定FSM处理函数
 */
int BTN_BindFSM(BTN_Handle_t* handle, void* fsm_instance, BTN_FSMEventHandler_t handler)
{
    if (!handle || !fsm_instance || !handler) return -1;
    
    handle->config.mode = BTN_MODE_FSM;
    handle->fsm.fsm_instance = fsm_instance;
    handle->fsm.handler = handler;
    
    return 0;
}

/**
 * @brief 解除FSM绑定
 */
int BTN_UnbindFSM(BTN_Handle_t* handle)
{
    if (!handle) return -1;
    
    handle->config.mode = BTN_MODE_STANDARD;
    handle->fsm.fsm_instance = NULL;
    handle->fsm.handler = NULL;
    
    return 0;
}

/**
 * @brief 注册组合键
 */
int BTN_RegisterCombo(BTN_ComboConfig_t* combo_config)
{
    if (!combo_config || combo_config->mask == 0) return -1;
    
    // 检查是否已存在
    for (uint8_t i = 0; i < s_combo_count; i++) {
        if (s_combo_configs[i].mask == combo_config->mask) {
            // 更新配置
            memcpy(&s_combo_configs[i], combo_config, sizeof(BTN_ComboConfig_t));
            return 0;
        }
    }
    
    // 检查数量
    if (s_combo_count >= MAX_COMBO_KEYS) {
        return -1;
    }
    
    // 添加新配置
    memcpy(&s_combo_configs[s_combo_count], combo_config, sizeof(BTN_ComboConfig_t));
    s_combo_count++;
    
    return 0;
}

/**
 * @brief 取消注册组合键
 */
int BTN_UnregisterCombo(BTN_ComboMask_t mask)
{
    for (uint8_t i = 0; i < s_combo_count; i++) {
        if (s_combo_configs[i].mask == mask) {
            // 移除配置
            if (i < s_combo_count - 1) {
                memmove(&s_combo_configs[i], &s_combo_configs[i+1], 
                       (s_combo_count - i - 1) * sizeof(BTN_ComboConfig_t));
            }
            s_combo_count--;
            return 0;
        }
    }
    
    return -1; // 未找到
}

/**
 * @brief 检测组合键
 */
static void BTN_ProcessCombo(uint32_t current_time)
{
    if (s_current_pressed_mask == 0) {
        // 没有按键被按下
        return;
    }
    
    // 检查每个组合键配置
    for (uint8_t i = 0; i < s_combo_count; i++) {
        BTN_ComboConfig_t* combo = &s_combo_configs[i];
        
        // 检查是否匹配
        if ((s_current_pressed_mask & combo->mask) == combo->mask) {
            uint32_t combo_time = current_time - s_combo_start_time;
            
            // 时间窗口检查
            if (combo_time <= (combo->time_window ? combo->time_window : BTN_CONFIG_COMBO_TIME)) {
                // 触发组合键回调
                if (combo->callback) {
                    combo->callback(0, BTN_EVENT_COMBO, combo->user_data);
                }
            }
        }
    }
}

/**
 * @brief 更新按钮状态，并检查事件
 */
static void BTN_UpdateButton(BTN_Handle_t* handle, uint32_t current_time)
{
    // 读取硬件状态
    BTN_State_t hw_state = handle->hardware.read_func(handle->hardware.param);
    
    // 去抖动处理
    if ((hw_state != handle->private.last_state) && 
        (current_time - handle->private.last_debounce_time >= handle->config.debounce_time)) {
        
        // 更新状态
        handle->private.last_state = hw_state;
        handle->private.last_debounce_time = current_time;
        
        if (hw_state == BTN_STATE_PRESSED) {
            // 按下事件
            handle->private.state = BTN_STATE_PRESSED;
            handle->private.is_pressed = 1;
            handle->private.press_time = current_time;
            handle->private.is_long_press = 0;
            
            // 更新组合键掩码
            s_current_pressed_mask |= BTN_MASK(handle->id);
            
            // 如果是第一个按下的键，记录组合键开始时间
            if (s_combo_start_time == 0) {
                s_combo_start_time = current_time;
            }
            
            // 触发回调
            if (handle->callback) {
                handle->callback(handle->id, BTN_EVENT_PRESSED, handle->user_data);
            }
            
            // 如果是FSM模式，触发FSM事件
            if (handle->config.mode == BTN_MODE_FSM && handle->fsm.handler) {
                handle->fsm.handler(handle->id, BTN_EVENT_PRESSED, 
                                   handle->fsm.fsm_instance, handle->user_data);
            }
        } else {
            // 释放事件
            handle->private.state = BTN_STATE_RELEASED;
            handle->private.is_pressed = 0;
            handle->private.release_time = current_time;
            
            // 更新组合键掩码
            s_current_pressed_mask &= ~BTN_MASK(handle->id);
            
            // 如果没有按键被按下，重置组合键开始时间
            if (s_current_pressed_mask == 0) {
                s_combo_start_time = 0;
            }
            
            // 计算按下时长
            uint32_t press_duration = current_time - handle->private.press_time;
            
            // 根据不同模式处理释放事件
            if (handle->config.mode == BTN_MODE_STANDARD || 
                handle->config.mode == BTN_MODE_FSM) {
                
                // 先触发通用释放事件
                if (handle->callback) {
                    handle->callback(handle->id, BTN_EVENT_RELEASED, handle->user_data);
                }
                
                // 如果是FSM模式，触发FSM释放事件
                if (handle->config.mode == BTN_MODE_FSM && handle->fsm.handler) {
                    handle->fsm.handler(handle->id, BTN_EVENT_RELEASED, 
                                       handle->fsm.fsm_instance, handle->user_data);
                }
                
                // 根据长按状态处理
                if (handle->private.is_long_press) {
                    // 长按释放
                    if (handle->callback) {
                        handle->callback(handle->id, BTN_EVENT_LONG_RELEASED, handle->user_data);
                    }
                    
                    // FSM长按释放
                    if (handle->config.mode == BTN_MODE_FSM && handle->fsm.handler) {
                        handle->fsm.handler(handle->id, BTN_EVENT_LONG_RELEASED, 
                                           handle->fsm.fsm_instance, handle->user_data);
                    }
                    
                    // 长按后不再处理点击事件
                    handle->private.click_count = 0;
                } 
                else if (press_duration <= handle->config.click_time) {
                    // 短按 - 累计点击次数
                    handle->private.click_count++;
                    
                    // 通用点击事件
                    if (handle->callback) {
                        handle->callback(handle->id, BTN_EVENT_CLICK, handle->user_data);
                    }
                    
                    // FSM点击事件
                    if (handle->config.mode == BTN_MODE_FSM && handle->fsm.handler) {
                        handle->fsm.handler(handle->id, BTN_EVENT_CLICK, 
                                           handle->fsm.fsm_instance, handle->user_data);
                    }
                }
            } 
            else if (handle->config.mode == BTN_MODE_INSTANT) {
                // 即时模式，只触发释放事件
                if (handle->callback) {
                    handle->callback(handle->id, BTN_EVENT_RELEASED, handle->user_data);
                }
            }
            else if (handle->config.mode == BTN_MODE_REPEAT) {
                // 重复模式，结束重复触发
                if (handle->callback) {
                    handle->callback(handle->id, BTN_EVENT_RELEASED, handle->user_data);
                }
            }
        }
    }
    
    // 长按检测
    if (handle->private.state == BTN_STATE_PRESSED && 
        handle->config.enable_long_press && 
        !handle->private.is_long_press) {
        
        uint32_t press_duration = current_time - handle->private.press_time;
        if (press_duration >= handle->config.long_press_time) {
            // 设置长按标志
            handle->private.is_long_press = 1;
            
            // 触发长按事件
            if (handle->callback) {
                handle->callback(handle->id, BTN_EVENT_LONG_PRESS, handle->user_data);
            }
            
            // FSM长按事件
            if (handle->config.mode == BTN_MODE_FSM && handle->fsm.handler) {
                handle->fsm.handler(handle->id, BTN_EVENT_LONG_PRESS, 
                                   handle->fsm.fsm_instance, handle->user_data);
            }
        }
    }
    
    // 重复触发检测
    if (handle->private.state == BTN_STATE_PRESSED && 
        handle->config.mode == BTN_MODE_REPEAT && 
        handle->config.enable_repeat) {
        
        uint32_t press_duration = current_time - handle->private.press_time;
        
        // 首次重复触发需要等待较长时间
        if (press_duration >= handle->config.repeat_start_time) {
            uint32_t repeat_duration = current_time - handle->private.last_repeat_time;
            
            // 检查重复间隔
            if (repeat_duration >= handle->config.repeat_time || 
                handle->private.last_repeat_time == 0) {
                
                // 更新最后重复时间
                handle->private.last_repeat_time = current_time;
                
                // 触发重复事件
                if (handle->callback) {
                    handle->callback(handle->id, BTN_EVENT_REPEAT, handle->user_data);
                }
            }
        }
    }
    
    // 多击检测
    if (handle->private.click_count > 0 && 
        handle->private.state == BTN_STATE_RELEASED && 
        (handle->config.mode == BTN_MODE_STANDARD || 
         handle->config.mode == BTN_MODE_FSM)) {
        
        uint32_t release_duration = current_time - handle->private.release_time;
        
        // 超过双击时间间隔，判定点击事件
        if (release_duration > handle->config.double_click_time) {
            BTN_Event_t click_event;
            
            // 根据点击次数确定事件类型
            switch (handle->private.click_count) {
                case 1:
                    click_event = BTN_EVENT_SINGLE_CLICK;
                    break;
                case 2:
                    click_event = BTN_EVENT_DOUBLE_CLICK;
                    break;
                case 3:
                default:
                    click_event = BTN_EVENT_TRIPLE_CLICK;
                    break;
            }
            
            // 触发事件
            if (handle->callback) {
                handle->callback(handle->id, click_event, handle->user_data);
            }
            
            // FSM事件
            if (handle->config.mode == BTN_MODE_FSM && handle->fsm.handler) {
                handle->fsm.handler(handle->id, click_event, 
                                   handle->fsm.fsm_instance, handle->user_data);
            }
            
            // 重置点击计数
            handle->private.click_count = 0;
        }
    }
}

/**
 * @brief 检查按钮是否处于按下状态
 */
uint8_t BTN_IsPressed(BTN_Handle_t* handle)
{
    if (!handle) return 0;
    return handle->private.is_pressed;
}

/**
 * @brief 检查按钮是否处于长按状态
 */
uint8_t BTN_IsLongPressed(BTN_Handle_t* handle)
{
    if (!handle) return 0;
    return handle->private.is_long_press;
}

/**
 * @brief 中断处理函数
 */
void BTN_HandleInterrupt(uint16_t gpio_pin)
{
    uint32_t current_time = HAL_GetTick();
    
    // 查找匹配的按钮
    for (uint8_t i = 0; i < BTN_CONFIG_MAX_BUTTONS; i++) {
        BTN_Handle_t* handle = s_btn_handles[i];
        
        if (handle && handle->config.detect_mode == BTN_DETECT_INTERRUPT && 
            handle->hardware.pin == gpio_pin) {
            
            // 读取硬件状态
            BTN_State_t hw_state = handle->hardware.read_func(handle->hardware.param);
            
            // 更新状态
            handle->private.last_state = hw_state;
            handle->private.last_debounce_time = current_time;
            
            // 剩余处理在BTN_Process中完成
        }
    }
}

/**
 * @brief 按钮处理函数
 */
void BTN_Process(void)
{
    uint32_t current_time = HAL_GetTick();
    
    // 处理所有按钮
    for (uint8_t i = 0; i < BTN_CONFIG_MAX_BUTTONS; i++) {
        BTN_Handle_t* handle = s_btn_handles[i];
        if (!handle) continue;
        
        // 仅处理轮询模式或状态需要更新的中断模式按钮
        if (handle->config.detect_mode == BTN_DETECT_POLLING || 
            (handle->config.detect_mode == BTN_DETECT_INTERRUPT && 
             handle->private.last_state != handle->private.state)) {
            
            BTN_UpdateButton(handle, current_time);
        }
    }
    
    // 处理组合键
    if (s_combo_count > 0) {
        BTN_ProcessCombo(current_time);
    }
}
