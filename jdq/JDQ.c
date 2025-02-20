#include "JDQ.h"

// 初始化继电器
void Relay_Init(Relay *relay, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin) {
    // 参数检查
    if (relay == NULL || GPIO_Port == NULL) {
        return;
    }

    relay->GPIO_Port = GPIO_Port;
    relay->GPIO_Pin = GPIO_Pin;
    relay->State = RELAY_OFF;
    relay->LastToggleTime = 0;

    // 配置 GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIO_Port, &GPIO_InitStruct);

    // 初始化为关闭状态
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_RESET);
}

// 设置继电器状态
void Relay_SetState(Relay *relay, RelayState state) {
    if (relay == NULL) {
        return;
    }
    relay->State = state;
    HAL_GPIO_WritePin(relay->GPIO_Port, relay->GPIO_Pin, (state == RELAY_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 切换继电器状态
void Relay_Toggle(Relay *relay) {
    if (relay == NULL) {
        return;
    }
    // 防抖处理
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - relay->LastToggleTime < 50) { // 50ms防抖
        return;
    }
    relay->LastToggleTime = currentTime;
    
    Relay_SetState(relay, (relay->State == RELAY_ON) ? RELAY_OFF : RELAY_ON);
}

// 安全控制继电器
void HAL_Relay_Control(Relay *relay, RelayState state) {
    if (relay == NULL || relay->State == state) {
        return;
    }
    relay->State = state;
    HAL_GPIO_WritePin(relay->GPIO_Port, relay->GPIO_Pin, (state == RELAY_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 获取继电器当前状态
RelayState Relay_GetState(Relay *relay) {
    if (relay == NULL) {
        return RELAY_OFF;
    }
    return relay->State;
}

// 检查继电器是否处于开启状态
uint8_t Relay_IsOn(Relay *relay) {
    if (relay == NULL) {
        return 0;
    }
    return (relay->State == RELAY_ON);
}

// 检查继电器是否处于关闭状态
uint8_t Relay_IsOff(Relay *relay) {
    if (relay == NULL) {
        return 1;
    }
    return (relay->State == RELAY_OFF);
}
