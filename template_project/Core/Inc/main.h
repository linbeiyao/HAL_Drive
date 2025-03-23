/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : 项目主头文件
  * @author         : HAL_Drive团队
  * @version        : v1.0.0
  * @date           : 2023-01-01
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含STM32硬件抽象层头文件 */
/* 这里需要根据具体的STM32型号替换 */
#include "stm32f1xx_hal.h" 

/* 导出常量和类型 --------------------------------------------------------*/
typedef enum {
  ERROR = 0,
  SUCCESS = !ERROR
} ErrorStatus;

/* 导出宏 ------------------------------------------------------------*/
#ifndef TRUE
  #define TRUE  1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

/* 导出功能原型 ---------------------------------------------------------*/
void Error_Handler(void);
void SystemClock_Config(void);

/* 用户按钮定义 ---------------------------------------------------------*/
#define KEY1_GPIO_PORT                GPIOA
#define KEY1_GPIO_PIN                 GPIO_PIN_0
#define KEY1_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define KEY1_ACTIVE_LEVEL             0  /* 低电平有效 */

#define KEY2_GPIO_PORT                GPIOC
#define KEY2_GPIO_PIN                 GPIO_PIN_13
#define KEY2_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOC_CLK_ENABLE()
#define KEY2_ACTIVE_LEVEL             0  /* 低电平有效 */

/* OLED显示屏定义 -------------------------------------------------------*/
#define OLED_SCL_GPIO_PORT            GPIOB
#define OLED_SCL_GPIO_PIN             GPIO_PIN_8
#define OLED_SCL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()

#define OLED_SDA_GPIO_PORT            GPIOB
#define OLED_SDA_GPIO_PIN             GPIO_PIN_9
#define OLED_SDA_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()

/* 用户LED定义 ----------------------------------------------------------*/
#define LED_GPIO_PORT                 GPIOC
#define LED_GPIO_PIN                  GPIO_PIN_0
#define LED_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()
#define LED_ON()                      HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET)
#define LED_OFF()                     HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET)
#define LED_TOGGLE()                  HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN)

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */ 