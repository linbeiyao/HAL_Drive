#include "taskmanager.h"
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>

/* LED引脚定义（根据实际硬件调整） */
#define LED1_GPIO_PORT GPIOA
#define LED1_GPIO_PIN  GPIO_PIN_1
#define LED2_GPIO_PORT GPIOA
#define LED2_GPIO_PIN  GPIO_PIN_2

/* 定义任务句柄 */
TaskHandle_t* led_task_handle = NULL;
TaskHandle_t* button_task_handle = NULL;
TaskHandle_t* print_task_handle = NULL;
TaskHandle_t* monitor_task_handle = NULL;

/* 按钮状态 */
uint8_t button_pressed = 0;

/* 任务函数声明 */
void LED_Task(void* param);
void Button_Task(void* param);
void Print_Task(void* param);
void Monitor_Task(void* param);

/**
 * @brief 初始化任务系统示例
 */
void TaskManager_Example_Init(void)
{
    printf("任务管理器示例初始化\r\n");
    
    // 初始化任务管理器（最多支持10个任务）
    if (TaskManager_Init(10) != 0) {
        printf("任务管理器初始化失败!\r\n");
        return;
    }
    
    // 创建LED控制任务（周期性任务，500ms执行一次，优先级3）
    led_task_handle = TaskManager_CreateTask("LED", LED_Task, NULL, 3, 500);
    if (led_task_handle == NULL) {
        printf("LED任务创建失败!\r\n");
    }
    
    // 创建按钮检测任务（周期性任务，100ms执行一次，优先级2）
    button_task_handle = TaskManager_CreateTask("BUTTON", Button_Task, NULL, 2, 100);
    if (button_task_handle == NULL) {
        printf("按钮任务创建失败!\r\n");
    }
    
    // 创建消息队列（用于按钮任务向LED任务发送消息）
    if (led_task_handle != NULL) {
        TaskManager_CreateTaskQueue(led_task_handle, 5, 32);
    }
    
    // 创建打印任务（周期性任务，1000ms执行一次，优先级4）
    print_task_handle = TaskManager_CreateTask("PRINT", Print_Task, NULL, 4, 1000);
    if (print_task_handle == NULL) {
        printf("打印任务创建失败!\r\n");
    }
    
    // 创建监控任务（周期性任务，2000ms执行一次，优先级5）
    monitor_task_handle = TaskManager_CreateTask("MONITOR", Monitor_Task, NULL, 5, 2000);
    if (monitor_task_handle == NULL) {
        printf("监控任务创建失败!\r\n");
    }
    
    printf("所有任务创建完成，开始任务调度\r\n");
    
    // 启动任务调度器
    TaskManager_StartScheduler();
}

/**
 * @brief LED控制任务函数
 * @param param 任务参数（未使用）
 */
void LED_Task(void* param)
{
    static uint8_t led_state = 0;
    static uint32_t run_count = 0;
    uint8_t msg_buffer[32];
    uint16_t msg_size = 0;
    
    // 增加运行计数
    run_count++;
    
    // 检查是否有消息
    if (TaskManager_ReceiveTaskMessage(led_task_handle, msg_buffer, sizeof(msg_buffer), &msg_size) == 0) {
        // 收到消息，处理命令
        if (strcmp((char*)msg_buffer, "LED_ON") == 0) {
            led_state = 1;
            printf("LED任务: 收到开灯命令\r\n");
        } else if (strcmp((char*)msg_buffer, "LED_OFF") == 0) {
            led_state = 0;
            printf("LED任务: 收到关灯命令\r\n");
        } else if (strcmp((char*)msg_buffer, "LED_TOGGLE") == 0) {
            led_state = !led_state;
            printf("LED任务: 收到切换命令\r\n");
        }
    }
    
    // LED控制
    if (led_state) {
        HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_GPIO_PIN, GPIO_PIN_RESET);
    }
    
    // 每10次闪烁一下第二个LED
    if (run_count % 10 == 0) {
        HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_GPIO_PIN);
    }
}

/**
 * @brief 按钮检测任务函数
 * @param param 任务参数（未使用）
 */
void Button_Task(void* param)
{
    static uint8_t last_state = 0;
    uint8_t current_state;
    
    // 读取按钮状态（假设低电平为按下状态）
    // 注意：实际项目中应加入去抖逻辑
    current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 1 : 0;
    
    // 检测按钮按下事件（边沿触发）
    if (current_state && !last_state) {
        // 按钮按下，发送LED切换命令
        const char* msg = "LED_TOGGLE";
        if (TaskManager_SendTaskMessage(led_task_handle, msg, strlen(msg) + 1, 0) == 0) {
            printf("按钮任务: 发送LED切换命令\r\n");
        } else {
            printf("按钮任务: 发送消息失败\r\n");
        }
        
        // 设置全局按钮状态
        button_pressed = 1;
    }
    
    // 更新上次状态
    last_state = current_state;
}

/**
 * @brief 打印任务函数
 * @param param 任务参数（未使用）
 */
void Print_Task(void* param)
{
    static uint32_t counter = 0;
    
    // 打印计数和任务信息
    printf("打印任务: 计数器 = %lu\r\n", ++counter);
    
    // 模拟耗时操作
    for (volatile uint32_t i = 0; i < 100000; i++) {
        // 空循环
    }
    
    // 检查和重置按钮状态
    if (button_pressed) {
        printf("打印任务: 检测到按钮按下事件\r\n");
        button_pressed = 0;
    }
}

/**
 * @brief 监控任务函数
 * @param param 任务参数（未使用）
 */
void Monitor_Task(void* param)
{
    uint32_t task_switch_count = 0;
    uint32_t idle_count = 0;
    uint32_t led_run_count = 0;
    
    // 获取系统统计信息
    TaskManager_GetSystemStats(&task_switch_count, &idle_count);
    
    // 获取LED任务运行次数
    if (led_task_handle != NULL) {
        TaskManager_GetTaskStats(led_task_handle, &led_run_count);
    }
    
    // 打印系统信息
    printf("\r\n============ 系统状态 ============\r\n");
    printf("任务切换次数: %lu\r\n", task_switch_count);
    printf("空闲计数: %lu\r\n", idle_count);
    printf("LED任务运行次数: %lu\r\n", led_run_count);
    printf("===================================\r\n\r\n");
    
    // 每隔5次运行，挂起/恢复打印任务，演示任务控制
    static uint8_t suspend_counter = 0;
    if (++suspend_counter >= 5) {
        suspend_counter = 0;
        
        if (print_task_handle != NULL) {
            if (print_task_handle->status == TASK_READY || 
                print_task_handle->status == TASK_BLOCKED) {
                // 挂起打印任务
                TaskManager_SuspendTask(print_task_handle);
                printf("监控任务: 挂起打印任务\r\n");
            } else if (print_task_handle->status == TASK_SUSPENDED) {
                // 恢复打印任务
                TaskManager_ResumeTask(print_task_handle);
                printf("监控任务: 恢复打印任务\r\n");
            }
        }
    }
}

/**
 * @brief 启动任务管理器示例
 */
void TaskManager_Example_Run(void)
{
    TaskManager_Example_Init();
} 