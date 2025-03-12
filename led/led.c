#include "led.h"

// 心跳LED状态变量
static unsigned int heartbeat_last_time = 0;
static unsigned char heartbeat_state = 0;

/**
 * @brief  初始化LED
 * @param  None
 * @retval None
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIOC时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // 配置PC13为输出模式
    GPIO_InitStruct.Pin = LED_HEARTBEAT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_HEARTBEAT_GPIO_PORT, &GPIO_InitStruct);
    
    // 初始状态为熄灭
    LED_HeartbeatOff();
}

/**
 * @brief  切换心跳LED状态
 * @param  None
 * @retval None
 */
void LED_HeartbeatToggle(void)
{
    HAL_GPIO_TogglePin(LED_HEARTBEAT_GPIO_PORT, LED_HEARTBEAT_PIN);
}

/**
 * @brief  点亮心跳LED
 * @param  None
 * @retval None
 */
void LED_HeartbeatOn(void)
{
    HAL_GPIO_WritePin(LED_HEARTBEAT_GPIO_PORT, LED_HEARTBEAT_PIN, LED_ON);
}

/**
 * @brief  熄灭心跳LED
 * @param  None
 * @retval None
 */
void LED_HeartbeatOff(void)
{
    HAL_GPIO_WritePin(LED_HEARTBEAT_GPIO_PORT, LED_HEARTBEAT_PIN, LED_OFF);
}

/**
 * @brief  心跳LED处理函数，需要在主循环中调用
 * @param  None
 * @retval None
 */
void LED_HeartbeatProcess(void)
{
    unsigned int current_time = HAL_GetTick();
    
    // 心跳模式：快闪两次，然后暂停
    switch (heartbeat_state)
    {
        case 0:  // 第一次点亮
            LED_HeartbeatOn();
            heartbeat_last_time = current_time;
            heartbeat_state = 1;
            break;
            
        case 1:  // 第一次熄灭
            if (current_time - heartbeat_last_time >= 100)
            {
                LED_HeartbeatOff();
                heartbeat_last_time = current_time;
                heartbeat_state = 2;
            }
            break;
            
        case 2:  // 短暂暂停
            if (current_time - heartbeat_last_time >= 100)
            {
                heartbeat_last_time = current_time;
                heartbeat_state = 3;
            }
            break;
            
        case 3:  // 第二次点亮
            if (current_time - heartbeat_last_time >= 100)
            {
                LED_HeartbeatOn();
                heartbeat_last_time = current_time;
                heartbeat_state = 4;
            }
            break;
            
        case 4:  // 第二次熄灭
            if (current_time - heartbeat_last_time >= 100)
            {
                LED_HeartbeatOff();
                heartbeat_last_time = current_time;
                heartbeat_state = 5;
            }
            break;
            
        case 5:  // 长暂停
            if (current_time - heartbeat_last_time >= 1000)
            {
                heartbeat_last_time = current_time;
                heartbeat_state = 0;
            }
            break;
            
        default:
            heartbeat_state = 0;
            break;
    }
}