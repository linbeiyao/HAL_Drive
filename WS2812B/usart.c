/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "ws2812b.h"
#include "string.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
UART_HandleTypeDef huart2;
uint8_t rxBuffer[CMD_LEN];
uint8_t commandBuffer[CMD_LEN];
uint8_t uartDataReady = 0;


unsigned char UART2_Rx_Buf[MAX_REC_LENGTH]; // USART1存储接收数据
unsigned char UART2_Rx_flg;                 // USART1接收完成标志
unsigned int UART2_Rx_cnt;                  // USART1接受数据计数器
unsigned char UART2_temp[REC_LENGTH];       // USART1接收数据缓存

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  // 开启串口接受中断
  // HAL_UART_Receive_IT(&huart2, (uint8_t *)rxBuffer, CMD_LEN);
  HAL_UART_Receive_IT(&huart2, (uint8_t *)UART2_temp, 1);     // 不定长接受
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspInit 0 */

    /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Channel5;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart1_rx);

    /* USER CODE BEGIN USART1_MspInit 1 */

    /* USER CODE END USART1_MspInit 1 */
  }
  else if (uartHandle->Instance == USART2)
  {
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspDeInit 0 */

    /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    /* USER CODE BEGIN USART1_MspDeInit 1 */

    /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

// 数据处理函数
void ProcessReceivedData(uint8_t *data)
{
  if (data[0] == 0x00)
    return;

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  // // 执行命令对应的操作
  // if (strstr((char *)data, LIGHT_HOME) != NULL)
  // {
  //   show_EnvUI(LightHome);
  //   ws2812_set_env(LightHome); // 切换到家庭环境
  // }
  // else if (strstr((char *)data, LIGHT_BOOKBAR) != NULL)
  // {

  //   show_EnvUI(LightBookBar);
  //   ws2812_set_env(LightBookBar); // 切换到书吧环境

  // }
  // else if (strstr((char *)data, LIGHT_MEETING) != NULL)
  // {
  //   show_EnvUI(LightMeeting);
  //   ws2812_set_env(LightMeeting); // 切换到会议环境
  // }
  // else if (strstr((char *)data, LIGHT_REST) != NULL)
  // {
  //   show_EnvUI(LightRest);
  //   ws2812_set_env(LightRest); // 切换到休息环境
  // }
  // else if (strstr((char *)data, LIGHT_ENTER) != NULL)
  // {
  //   show_EnvUI(LightEnter);
  //   ws2812_set_env(LightEnter); // 切换到娱乐模式
  // }
  // else if (strstr((char *)data, LIGHT_OFF)!= NULL)
  // {
  //   show_EnvUI(LightOff);
  //   ws2812_set_env(LightOff); // 关闭灯光
  // }
  // else
  // {
  //   return;
  // }
}

// UART 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart2)
  {
    


      uartDataReady = 1;                  // 设置数据准备标志

      // 数据处理逻辑
      if (strstr((char *)UART2_Rx_Buf, LIGHT_HOME ) != NULL|| UART2_temp[0] == 'h') // 如果当前字符等于 h
      {
        homeflag = 1;
        bookbarflag = 0;
        meetingflag = 0;
        restflag = 0;
        enterflag = 0;
      }
      else if (strstr((char *)UART2_Rx_Buf, LIGHT_BOOKBAR ) != NULL || UART2_temp[0] == 'o')
      {
        homeflag = 0;
        bookbarflag = 1;
        meetingflag = 0;
        restflag = 0;
        enterflag = 0;
      }
      else if (strstr((char *)UART2_Rx_Buf, LIGHT_MEETING) != NULL || UART2_temp[0] == 'm')
      {
        homeflag = 0;
        bookbarflag = 0;
        meetingflag = 1;
        restflag = 0;
        enterflag = 0;
      }
      else if (strstr((char *)UART2_Rx_Buf, LIGHT_REST) != NULL || UART2_temp[0] == 'r')
      {
        homeflag = 0;
        bookbarflag = 0;
        meetingflag = 0;
        restflag = 1;
        enterflag = 0;
      }
      else if (strstr((char *)UART2_Rx_Buf, LIGHT_ENTER) != NULL || UART2_temp[0] == 'e')
      {
        homeflag = 0;
        bookbarflag = 0;
        meetingflag = 0;
        restflag = 0;
        enterflag = 1;
      }

      UART2_Rx_cnt = 0; // 重置计数器，准备接收下一条数据
    }
    else
    {
      UART2_Rx_cnt++; // 增加计数器
      if (UART2_Rx_cnt >= MAX_REC_LENGTH - 1)
      {
        UART2_Rx_cnt = 0; // 防止缓冲区溢出，重置计数器
      }
    }
    

    // 重新启动接收中断
    HAL_UART_Receive_IT(&huart2, (uint8_t *)UART2_temp, 1);
  
}


void SendAsrpro(uint8_t *data)
{
  uint8_t len = strlen((char *)data);
  HAL_UART_Transmit(&huart2, data, len, 100);
}
