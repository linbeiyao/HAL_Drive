/**
 * @file main.h
 * @brief 应用程序主头文件
 * @details 包含HAL库基本定义和GPIO配置
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含HAL库头文件 */
/* 注意：实际使用时替换为实际的HAL库头文件 */
/* 如：#include "stm32f1xx_hal.h" */
#include <stdint.h>

/* 为演示目的模拟HAL库的一些定义 */
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

typedef uint16_t GPIO_TypeDef;
typedef uint16_t GPIO_InitTypeDef;
typedef uint16_t I2C_HandleTypeDef;
typedef uint16_t UART_HandleTypeDef;

#define HAL_OK      0
#define HAL_ERROR   1
#define HAL_BUSY    2
#define HAL_TIMEOUT 3

#define GPIO_PIN_0  0x0001
#define GPIO_PIN_1  0x0002
#define GPIO_PIN_2  0x0004
#define GPIO_PIN_3  0x0008
#define GPIO_PIN_4  0x0010
#define GPIO_PIN_5  0x0020
#define GPIO_PIN_6  0x0040
#define GPIO_PIN_7  0x0080
#define GPIO_PIN_8  0x0100
#define GPIO_PIN_9  0x0200
#define GPIO_PIN_10 0x0400
#define GPIO_PIN_11 0x0800
#define GPIO_PIN_12 0x1000
#define GPIO_PIN_13 0x2000
#define GPIO_PIN_14 0x4000
#define GPIO_PIN_15 0x8000

/* GPIO和外设句柄定义 */
extern GPIO_TypeDef* GPIOA;
extern GPIO_TypeDef* GPIOB;
extern GPIO_TypeDef* GPIOC;
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

/* 模拟HAL库的一些基本函数 */
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t Delay);
void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void HAL_IncTick(void);

/* 应用程序初始化函数和主循环函数 */
void SystemClock_Config(void);
void Error_Handler(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */ 