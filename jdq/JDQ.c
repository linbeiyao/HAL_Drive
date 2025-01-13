#include "JDQ.h"

// 初始化继电器
void Relay_Init(Relay *relay, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin) {
    relay->GPIO_Port = GPIO_Port;
    relay->GPIO_Pin = GPIO_Pin;
    relay->State = RELAY_OFF;

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
    relay->State = state;
    HAL_GPIO_WritePin(relay->GPIO_Port, relay->GPIO_Pin, (state == RELAY_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 切换继电器状态
void Relay_Toggle(Relay *relay) {
    if (relay->State == RELAY_ON) {
        Relay_SetState(relay, RELAY_OFF);
    } else {
        Relay_SetState(relay, RELAY_ON);
    }
}
