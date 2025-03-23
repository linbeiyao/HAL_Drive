/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 主程序
  * @author         : HAL_Drive团队
  * @version        : v1.0.0
  * @date           : 2023-01-01
  ******************************************************************************
  */

#include "main.h"
#include "../../App/app_includes.h"

/* 私有变量 -------------------------------------------------------------*/
/* 私有函数原型 ---------------------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);

/**
  * @brief  应用程序入口函数
  * @retval int
  */
int main(void)
{
  /* MCU配置 */
  HAL_Init();

  /* 配置系统时钟 */
  SystemClock_Config();

  /* 初始化所有配置的外设 */
  GPIO_Init();

  /* 运行应用程序 */
  App_Run();

  /* 不应该到达这里 */
  while (1)
  {
  }
}

/**
  * @brief 系统时钟配置
  * @note  这里应该根据具体的STM32型号进行配置
  * @param None
  * @retval None
  */
void SystemClock_Config(void)
{
  /* 配置系统时钟为72MHz(对于STM32F103系列) */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* 配置LSI、HSE和PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* 配置系统时钟源、AHB、APB1和APB2总线时钟 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO初始化
  * @param None
  * @retval None
  */
static void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 使能GPIO时钟 */
  KEY1_GPIO_CLK_ENABLE();
  KEY2_GPIO_CLK_ENABLE();
  LED_GPIO_CLK_ENABLE();
  OLED_SCL_GPIO_CLK_ENABLE();
  OLED_SDA_GPIO_CLK_ENABLE();

  /* 配置LED引脚 */
  GPIO_InitStruct.Pin = LED_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
  
  /* 配置按键1引脚 */
  GPIO_InitStruct.Pin = KEY1_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStruct);
  
  /* 配置按键2引脚 */
  GPIO_InitStruct.Pin = KEY2_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStruct);
  
  /* 配置OLED引脚 */
  GPIO_InitStruct.Pin = OLED_SCL_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = OLED_SDA_GPIO_PIN;
  HAL_GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStruct);

  /* 设置默认状态 */
  LED_OFF();
}

/**
  * @brief  SysTick回调函数
  * @param  None
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
  /* 更新系统时间 */
  App_SysTickUpdate();
}

/**
  * @brief  错误处理函数
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* 错误指示 */
  LED_ON();
  
  /* 无限循环 */
  while (1)
  {
    /* 闪烁LED指示错误 */
    LED_TOGGLE();
    HAL_Delay(200);
  }
} 