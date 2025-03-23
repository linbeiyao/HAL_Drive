// debug.c
#include "debug.h"
#include <string.h>
#include <stdio.h>

// 定义串口句柄实例（需要在main.c中初始化）
UART_HandleTypeDef huart1;

/**
 * @brief 初始化调试串口（UART1）
 * @param baudrate 波特率设置
 */
void Debug_Init(uint32_t baudrate)
{
    // 串口1引脚设置
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能GPIOA时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* 使能USART1时钟 */
    __HAL_RCC_USART1_CLK_ENABLE();
    
    /* 配置串口引脚 */
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;  // PA9: TX, PA10: RX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* 配置UART */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = baudrate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        // 初始化失败处理
        while(1);
    }
    
    // 初始化后发送一条消息
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "\r\n[INFO] Debug UART1 initialized at %d bps\r\n", (int)baudrate);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/**
 * @brief 发送原始数据
 * @param data 数据指针
 * @param size 数据大小
 */
void Debug_Send(uint8_t *data, uint16_t size)
{
    HAL_UART_Transmit(&huart1, data, size, HAL_MAX_DELAY);
}

/**
 * @brief 开始计时
 * @param timer 计时器结构体指针
 * @param name 计时器名称
 */
void Debug_TimerStart(Debug_Timer_t *timer, const char *name)
{
    if (timer)
    {
        timer->start_time = HAL_GetTick();
        timer->elapsed_time = 0;
        strncpy(timer->name, name, sizeof(timer->name) - 1);
        timer->name[sizeof(timer->name) - 1] = '\0'; // 确保字符串结束
        
        #if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "[TIMER] '%s' started\r\n", timer->name);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
        #endif
    }
}

/**
 * @brief 停止计时
 * @param timer 计时器结构体指针
 */
void Debug_TimerStop(Debug_Timer_t *timer)
{
    if (timer)
    {
        timer->elapsed_time = HAL_GetTick() - timer->start_time;
    }
}

/**
 * @brief 打印计时结果
 * @param timer 计时器结构体指针
 */
void Debug_TimerPrint(Debug_Timer_t *timer)
{
    if (timer)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "[TIMER] '%s' elapsed: %lu ms\r\n", 
                 timer->name, timer->elapsed_time);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    }
}

/**
 * @brief 打印十六进制数据
 * @param desc 数据描述
 * @param addr 数据地址
 * @param len 数据长度
 */
void Debug_HexDump(const char *desc, const void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
    char line[80];
    int line_len = 0;
    
    // 输出描述信息
    if (desc != NULL)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%s [%d bytes]:\r\n", desc, len);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    }
    
    // 处理每一行（16字节）
    for (i = 0; i < len; i++)
    {
        // 每行开始时，先打印偏移地址
        if ((i % 16) == 0)
        {
            if (i != 0)
            {
                // 打印ASCII部分
                line_len += snprintf(line + line_len, sizeof(line) - line_len, "  %s\r\n", buff);
                HAL_UART_Transmit(&huart1, (uint8_t*)line, line_len, HAL_MAX_DELAY);
                line_len = 0;
            }
            
            // 打印新行的偏移地址
            line_len = snprintf(line, sizeof(line), "  %04x ", i);
        }
        
        // 打印16进制数据
        line_len += snprintf(line + line_len, sizeof(line) - line_len, " %02x", pc[i]);
        
        // 准备ASCII显示部分
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    
    // 打印最后一行剩余部分
    while ((i % 16) != 0)
    {
        line_len += snprintf(line + line_len, sizeof(line) - line_len, "   ");
        i++;
    }
    
    line_len += snprintf(line + line_len, sizeof(line) - line_len, "  %s\r\n", buff);
    HAL_UART_Transmit(&huart1, (uint8_t*)line, line_len, HAL_MAX_DELAY);
} 