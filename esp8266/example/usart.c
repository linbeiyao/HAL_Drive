/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "oled.h"
#include "tim.h"


extern volatile uint16_t dma_rx_index;      // 当前接收索引
extern volatile uint8_t dma_rx_complete;    // 接收完成标志



UART_HandleTypeDef huart1;



void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    /* 空闲中断 */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    /* 启动 DMA 接收 
     * 利用串口的空闲中断实现不定长数据字符串的接受
     * 配置合适的 串口 ,dma 通道,dma 的缓冲区，以及DMA的缓冲区的大小
     * 				这里 DMA 缓冲区使用的是两个静态缓冲区，双BUF操作，避免串口数据丢失
     * 开始中断
     * 记得在 hal_cofig 中开启串口的回调函数
     */ 
   


    // HAL_UART_Receive_DMA(&huart1, dma_rx_buffer, DMA_BUFFER_SIZE);
    // 已经在 esp8266 初始化中开启 
		
    HAL_NVIC_SetPriority(USART1_IRQn,1,0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}



// 定义两个静态缓冲区
static uint8_t dma_rx_buffer_a[DMA_BUFFER_SIZE] = {0};
static uint8_t dma_rx_buffer_b[DMA_BUFFER_SIZE] = {0};
// 当前活动缓冲区标志 (0:buffer_a, 1:buffer_b)
static uint8_t current_buffer = 0;
// 指向当前接收缓冲区的指针
static uint8_t *current_rx_buffer = dma_rx_buffer_a;
// 指向处理缓冲区的指针
static uint8_t *process_rx_buffer = dma_rx_buffer_b;
// 缓冲区数据长度标记
static uint16_t buffer_a_length = 0;
static uint16_t buffer_b_length = 0;
// 缓冲区处理标志
static uint8_t buffer_a_need_process = 0;
static uint8_t buffer_b_need_process = 0;
// 初始化阶段标志
uint8_t g_uart_initialization_complete = 0; // 0表示初始化阶段，1表示正常运行阶段

// 处理数据函数的前向声明
static void process_buffer_data(uint8_t *buffer, uint16_t length);

// 设置初始化阶段结束的函数
void UART_InitializationComplete(void) {
    g_uart_initialization_complete = 1;
}

void USART1_IRQHandler(void) {
    // 检查空闲中断标志
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);  // 清除空闲中断标志
        HAL_UART_DMAStop(&huart1);
        uint16_t dma_data_length = DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
        
        // 记录当前缓冲区接收到的数据长度并设置处理标志
        if (current_buffer == 0) {
            buffer_a_length = dma_data_length;
            buffer_a_need_process = 1;
        } else {
            buffer_b_length = dma_data_length;
            buffer_b_need_process = 1;
        }
        
        // 切换缓冲区
        if (current_buffer == 0) {
            current_buffer = 1;
            current_rx_buffer = dma_rx_buffer_b;
            process_rx_buffer = dma_rx_buffer_a;
        } else {
            current_buffer = 0;
            current_rx_buffer = dma_rx_buffer_a;
            process_rx_buffer = dma_rx_buffer_b;
        }
        
        // 重新启动DMA接收，使用新的缓冲区
        HAL_UART_Receive_DMA(&huart1, current_rx_buffer, DMA_BUFFER_SIZE);
        
        // 设置DMA接收完成标志，通知主循环处理数据
        dma_rx_complete = 1;
        
        // 获取ESP8266对象，用于检查立即处理标志
        ESP8266_HandleTypeDef *esp = FruitVendingData_GetEspObject(&g_fruitVendingData);
        
        // 判断是否需要立即处理数据（在中断中处理）
        // 只有在初始化阶段或明确设置了立即处理标志时才立即处理
        if (!g_uart_initialization_complete || (esp && esp->immediate_process_flag)) {
            // 只有在需要立即处理时，才在中断中处理数据
            if (buffer_a_need_process) {
                process_buffer_data(dma_rx_buffer_a, buffer_a_length);
                buffer_a_need_process = 0;
                buffer_a_length = 0;
            }
            
            if (buffer_b_need_process) {
                process_buffer_data(dma_rx_buffer_b, buffer_b_length);
                buffer_b_need_process = 0;
                buffer_b_length = 0;
            }
        }
        // 否则，数据将在主循环中由UART_ProcessReceivedData处理
    }

    // 调用HAL库的默认中断处理函数
    HAL_UART_IRQHandler(&huart1);
}

// 处理接收到的数据
static void process_buffer_data(uint8_t *buffer, uint16_t length) {
    if (length > 0) {
        // 获取ESP8266对象
        ESP8266_HandleTypeDef *esp = FruitVendingData_GetEspObject(&g_fruitVendingData);
        
        // 确保数据不会溢出接收缓冲区
        if (length > ESP8266_MAX_BUFFER_SIZE) {
            length = ESP8266_MAX_BUFFER_SIZE;
        }
        
        // 将处理缓冲区中的数据复制到ESP8266的接收缓冲区
        memcpy(esp->rx_buffer, buffer, length);
        esp->rx_buffer[length] = '\0'; // 确保字符串结束符
        esp->rx_index = length;
        
        // 设置接收完成标志
        esp->response_received = 1;
        
        // 判断是否需要立即处理数据
        // 如果是在初始化阶段或设置了立即处理标志，则直接处理
        // 如果是在主循环中调用(UART_ProcessReceivedData)，则总是处理
        if (!g_uart_initialization_complete || (esp && esp->immediate_process_flag)) {
            ESP8266_UART_IRQHandler(esp);
        }
        
        // 标记DMA接收完成
        dma_rx_complete = 1;
    }
}

// 在主循环中调用此函数处理UART接收到的数据
void UART_ProcessReceivedData(void) {
    // 检查是否有缓冲区需要处理
    if (buffer_a_need_process) {
        // 获取ESP8266对象
        ESP8266_HandleTypeDef *esp = FruitVendingData_GetEspObject(&g_fruitVendingData);
        
        process_buffer_data(dma_rx_buffer_a, buffer_a_length);
        buffer_a_need_process = 0;
        buffer_a_length = 0;
        
        // 在主循环中，如果数据没有在中断中被处理，则在这里处理
        if (g_uart_initialization_complete && esp && !esp->immediate_process_flag && esp->response_received) {
					esp->rx_buffer[esp->rx_index] = '\0'; // 添加字符串结束符
					esp->response_received = 1;           // 标记响应接收完成

							ESP8266_ProcessReceivedData(esp);
        }
    }
    
    if (buffer_b_need_process) {
        // 获取ESP8266对象
        ESP8266_HandleTypeDef *esp = FruitVendingData_GetEspObject(&g_fruitVendingData);
        
        process_buffer_data(dma_rx_buffer_b, buffer_b_length);
        buffer_b_need_process = 0;
        buffer_b_length = 0;
        
        // 在主循环中，如果数据没有在中断中被处理，则在这里处理
        if (g_uart_initialization_complete && esp && !esp->immediate_process_flag && esp->response_received) {
					esp->rx_buffer[esp->rx_index] = '\0'; // 添加字符串结束符
					esp->response_received = 1;           // 标记响应接收完成


							ESP8266_ProcessReceivedData(esp);

        }
    }
}

// 回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
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

  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  }
}

