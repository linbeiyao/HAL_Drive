#include "esp8266.h"
#include <string.h>
#include <stdio.h>

// 静态变量定义
static uint32_t s_last_processing_time = 0;  // 上次处理时间戳
static volatile uint8_t s_is_processing = 0; // 处理锁，防止重入

// 环形缓冲区函数实现
/**
 * @brief 初始化环形缓冲区
 * @param ring 环形缓冲区指针
 */
void ESP8266_RingBuffer_Init(ESP8266_RingBuffer_t *ring) {
    if (ring == NULL) {
        return;
    }
    memset(ring->buffer, 0, RING_BUFFER_SIZE);
    ring->head = 0;
    ring->tail = 0;
    ring->count = 0;
}

/**
 * @brief 向环形缓冲区写入数据
 * @param ring 环形缓冲区指针
 * @param data 数据指针
 * @param length 数据长度
 * @return 实际写入的字节数
 */
uint16_t ESP8266_RingBuffer_Write(ESP8266_RingBuffer_t *ring, uint8_t *data, uint16_t length) {
    if (ring == NULL || data == NULL || length == 0) {
        return 0;
    }
    
    // 防止中断重入时修改环形缓冲区
    __disable_irq();
    
    uint16_t space = RING_BUFFER_SIZE - ring->count;
    if (length > space) {
        length = space; // 调整写入长度，避免溢出
    }
    
    // 写入数据
    for (uint16_t i = 0; i < length; i++) {
        ring->buffer[ring->head] = data[i];
        ring->head = (ring->head + 1) % RING_BUFFER_SIZE;
        ring->count++;
    }
    
    __enable_irq();
    return length;
}

/**
 * @brief 从环形缓冲区读取数据
 * @param ring 环形缓冲区指针
 * @param data 目标缓冲区
 * @param length 期望读取的长度
 * @return 实际读取的字节数
 */
uint16_t ESP8266_RingBuffer_Read(ESP8266_RingBuffer_t *ring, uint8_t *data, uint16_t length) {
    if (ring == NULL || data == NULL || length == 0 || ring->count == 0) {
        return 0;
    }
    
    __disable_irq();
    
    if (length > ring->count) {
        length = ring->count; // 调整读取长度，避免越界
    }
    
    // 读取数据
    for (uint16_t i = 0; i < length; i++) {
        data[i] = ring->buffer[ring->tail];
        ring->tail = (ring->tail + 1) % RING_BUFFER_SIZE;
        ring->count--;
    }
    
    __enable_irq();
    return length;
}

/**
 * @brief 获取环形缓冲区中可读取的数据量
 * @param ring 环形缓冲区指针
 * @return 可读取的字节数
 */
uint16_t ESP8266_RingBuffer_Available(ESP8266_RingBuffer_t *ring) {
    if (ring == NULL) {
        return 0;
    }
    return ring->count;
}

/**
 * @brief 清空环形缓冲区
 * @param ring 环形缓冲区指针
 */
void ESP8266_RingBuffer_Flush(ESP8266_RingBuffer_t *ring) {
    if (ring == NULL) {
        return;
    }
    
    __disable_irq();
    ring->head = 0;
    ring->tail = 0;
    ring->count = 0;
    __enable_irq();
}

/**
 * @brief 查找环形缓冲区中的字符串
 * @param ring 环形缓冲区指针
 * @param str 要查找的字符串
 * @return 找到返回1，否则返回0
 */
uint8_t ESP8266_RingBuffer_FindString(ESP8266_RingBuffer_t *ring, const char *str) {
    if (ring == NULL || str == NULL || ring->count == 0) {
        return 0;
    }
    
    uint16_t str_len = strlen(str);
    if (str_len == 0 || str_len > ring->count) {
        return 0;
    }
    
    __disable_irq();
    
    // 从当前读取位置开始查找
    for (uint16_t i = 0; i < ring->count - str_len + 1; i++) {
        uint8_t match = 1;
        
        for (uint16_t j = 0; j < str_len; j++) {
            uint16_t pos = (ring->tail + i + j) % RING_BUFFER_SIZE;
            if (ring->buffer[pos] != (uint8_t)str[j]) {
                match = 0;
                break;
            }
        }
        
        if (match) {
            __enable_irq();
            return 1;
        }
    }
    
    __enable_irq();
    return 0;
}

// UART2+DMA函数实现
/**
 * @brief 初始化ESP8266 UART2扩展功能
 * @param esp ESP8266_UART2_HandleTypeDef指针
 * @param huart UART句柄指针
 * @param hdma_rx DMA接收句柄指针
 */
void ESP8266_UART2_Init(ESP8266_UART2_HandleTypeDef *esp, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma_rx) {
    if (esp == NULL || huart == NULL || hdma_rx == NULL) {
        return;
    }
    
    // 初始化基础ESP8266结构
    ESP8266_Init(&esp->esp_base, huart);
    
    // 初始化环形缓冲区
    ESP8266_RingBuffer_Init(&esp->ring_buffer);
    
    // 保存DMA句柄
    esp->hdma_rx = hdma_rx;
    
    // 初始化DMA缓冲区
    memset(esp->dma_buffer, 0, ESP8266_DMA_BUFFER_SIZE);
    
    // 初始化标志位
    esp->dma_busy = 0;
    esp->data_ready = 0;
    esp->retry_count = 0;
    esp->last_retry_time = 0;
    
    // 清空重发队列
    ESP8266_UART2_ClearRetryQueue(esp);
    
    // 启用UART空闲中断
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    
    // 启动DMA接收
    esp->dma_busy = 1;
    HAL_UART_Receive_DMA(huart, esp->dma_buffer, ESP8266_DMA_BUFFER_SIZE);
    
    // 初始化静态变量
    s_last_processing_time = HAL_GetTick();
    s_is_processing = 0;
}

/**
 * @brief UART2中断处理函数，在USART2_IRQHandler中调用
 * @param esp ESP8266_UART2_HandleTypeDef指针
 */
void ESP8266_UART2_IRQHandler(ESP8266_UART2_HandleTypeDef *esp) {
    if (esp == NULL || esp->esp_base.huart == NULL) {
        return;
    }
    
    UART_HandleTypeDef *huart = esp->esp_base.huart;
    
    // 检查是否为IDLE中断
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)) {
        // 清除IDLE标志
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        
        // 停止DMA接收
        HAL_UART_DMAStop(huart);
        
        // 计算接收到的数据长度
        uint16_t dma_rx_len = ESP8266_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(esp->hdma_rx);
        
        if (dma_rx_len > 0) {
            // 将DMA缓冲区数据写入环形缓冲区
            ESP8266_RingBuffer_Write(&esp->ring_buffer, esp->dma_buffer, dma_rx_len);
            
            // 设置数据就绪标志
            esp->data_ready = 1;
            
            // 清空DMA缓冲区
            memset(esp->dma_buffer, 0, ESP8266_DMA_BUFFER_SIZE);
        }
        
        // 重新启动DMA接收
        esp->dma_busy = 1;
        HAL_UART_Receive_DMA(huart, esp->dma_buffer, ESP8266_DMA_BUFFER_SIZE);
    }
}

/**
 * @brief 处理接收到的数据，应在主循环中调用
 * @param esp ESP8266_UART2_HandleTypeDef指针
 */
void ESP8266_UART2_Process(ESP8266_UART2_HandleTypeDef *esp) {
    if (esp == NULL || s_is_processing) {
        return;
    }
    
    uint32_t current_time = HAL_GetTick();
    
    // 限制处理频率，不要太频繁
    if (current_time - s_last_processing_time < 10) {
        return;
    }
    
    s_is_processing = 1;
    s_last_processing_time = current_time;
    
    // 1. 处理接收到的数据
    if (esp->data_ready && ESP8266_RingBuffer_Available(&esp->ring_buffer) > 0) {
        uint8_t temp_buffer[128];
        uint16_t read_length;
        
        // 从环形缓冲区读取数据块
        while ((read_length = ESP8266_RingBuffer_Read(&esp->ring_buffer, temp_buffer, sizeof(temp_buffer) - 1)) > 0) {
            // 确保字符串结束
            temp_buffer[read_length] = '\0'; 
            
            // 复制到ESP8266基础结构体的接收缓冲区
            memcpy(esp->esp_base.rx_buffer, temp_buffer, read_length);
            esp->esp_base.rx_index = read_length;
            esp->esp_base.rx_buffer[read_length] = '\0';
            
            // 处理接收到的数据
            ESP8266_ProcessReceivedData(&esp->esp_base);
            
            // 防止阻塞太久
            if (HAL_GetTick() - current_time > 50) {
                break;
            }
        }
        
        // 如果环形缓冲区中还有数据，保持data_ready标志
        esp->data_ready = (ESP8266_RingBuffer_Available(&esp->ring_buffer) > 0);
    }
    
    // 2. 处理重发队列
    ESP8266_UART2_ProcessRetryQueue(esp);
    
    s_is_processing = 0;
}

/**
 * @brief 通过UART2发送AT命令
 * @param esp ESP8266_UART2_HandleTypeDef指针
 * @param cmd 命令字符串
 * @param expected_response 期望的响应字符串
 * @param timeout_ms 超时时间(毫秒)
 * @return 操作结果
 */
int ESP8266_UART2_SendCommand(ESP8266_UART2_HandleTypeDef *esp, const char *cmd, const char *expected_response, uint32_t timeout_ms) {
    if (esp == NULL || cmd == NULL) {
        return ESP8266_ERROR;
    }
    
    // 清空环形缓冲区，确保不会读取旧数据
    ESP8266_RingBuffer_Flush(&esp->ring_buffer);
    
    // 直接调用基础函数
    return ESP8266_SendCommand(&esp->esp_base, cmd, expected_response, timeout_ms);
}

/**
 * @brief 使用UART2发布MQTT消息，支持自动重发
 * @param esp ESP8266_UART2_HandleTypeDef指针
 * @param topic MQTT主题
 * @param message 消息内容
 * @param qos 服务质量
 * @param retain 保留标志
 * @param timeout 超时时间(毫秒)
 * @return 操作结果
 */
int ESP8266_UART2_PublishMQTT(ESP8266_UART2_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout) {
    if (esp == NULL || topic == NULL || message == NULL) {
        return ESP8266_ERROR;
    }
    
    // 如果MQTT连接断开，直接加入重发队列
    if (esp->esp_base.mqtt_status != MQTT_STATUS_CONNECTED) {
        ESP8266_UART2_AddToRetryQueue(esp, topic, message, qos, retain);
        return ESP8266_ERROR_MQTT;
    }
    
    // 尝试发送消息
    int result = ESP8266_PublishMQTT(&esp->esp_base, topic, message, qos, retain, timeout);
    
    // 如果发送失败，添加到重发队列
    if (result != ESP8266_OK) {
        ESP8266_UART2_AddToRetryQueue(esp, topic, message, qos, retain);
    }
    
    return result;
}

/**
 * @brief 使用UART2发布JSON格式MQTT消息，支持自动重发
 * @param esp ESP8266_UART2_HandleTypeDef指针
 * @param topic MQTT主题
 * @param key JSON键
 * @param value JSON值
 * @param qos 服务质量
 * @param retain 保留标志
 * @param timeout 超时时间(毫秒)
 * @return 操作结果
 */
int ESP8266_UART2_PublishMQTT_JSON(ESP8266_UART2_HandleTypeDef *esp, const char *topic, const char *key, const char *value, MQTT_QoS qos, uint8_t retain, uint32_t timeout) {
    if (esp == NULL || topic == NULL || key == NULL || value == NULL) {
        return ESP8266_ERROR;
    }
    
    // 构建JSON数据
    char json_buffer[ESP8266_MAX_JSON_SIZE];
    int json_len = ESP8266_BuildJSON(json_buffer, sizeof(json_buffer), key, value);
    
    if (json_len <= 0) {
        return ESP8266_ERROR;
    }
    
    // 发布消息
    return ESP8266_UART2_PublishMQTT(esp, topic, json_buffer, qos, retain, timeout);
}

/**
 * @brief 将消息添加到重发队列
 * @param esp ESP8266_UART2_HandleTypeDef指针
 * @param topic MQTT主题
 * @param message 消息内容
 * @param qos 服务质量
 * @param retain 保留标志
 * @return 操作结果
 */
int ESP8266_UART2_AddToRetryQueue(ESP8266_UART2_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain) {
    if (esp == NULL || topic == NULL || message == NULL) {
        return ESP8266_ERROR;
    }
    
    // 检查是否是重复消息，避免重复添加
    for (uint8_t i = 0; i < ESP8266_RETRY_QUEUE_SIZE; i++) {
        if (esp->retry_queue[i].active && 
            strcmp(esp->retry_queue[i].topic, topic) == 0 && 
            strcmp(esp->retry_queue[i].message, message) == 0) {
            
            // 更新时间戳，确保消息会被重试
            esp->retry_queue[i].timestamp = HAL_GetTick();
            esp->retry_queue[i].retry_count = 0;
            return ESP8266_OK;
        }
    }
    
    // 检查队列是否已满
    if (esp->retry_count >= ESP8266_RETRY_QUEUE_SIZE) {
        // 队列已满，查找最旧或重试次数最多的消息替换
        uint8_t replace_index = 0;
        uint32_t oldest_time = HAL_GetTick();
        uint8_t most_retries = 0;
        
        for (uint8_t i = 0; i < ESP8266_RETRY_QUEUE_SIZE; i++) {
            if (esp->retry_queue[i].active) {
                // 优先考虑重试次数
                if (esp->retry_queue[i].retry_count > most_retries) {
                    most_retries = esp->retry_queue[i].retry_count;
                    replace_index = i;
                } 
                // 其次考虑时间
                else if (esp->retry_queue[i].retry_count == most_retries && 
                         esp->retry_queue[i].timestamp < oldest_time) {
                    oldest_time = esp->retry_queue[i].timestamp;
                    replace_index = i;
                }
            }
        }
        
        // 替换选定的消息
        strncpy(esp->retry_queue[replace_index].topic, topic, ESP8266_MAX_TOPIC_SIZE - 1);
        esp->retry_queue[replace_index].topic[ESP8266_MAX_TOPIC_SIZE - 1] = '\0';
        
        strncpy(esp->retry_queue[replace_index].message, message, ESP8266_MAX_MESSAGE_SIZE - 1);
        esp->retry_queue[replace_index].message[ESP8266_MAX_MESSAGE_SIZE - 1] = '\0';
        
        esp->retry_queue[replace_index].qos = qos;
        esp->retry_queue[replace_index].retain = retain;
        esp->retry_queue[replace_index].timestamp = HAL_GetTick();
        esp->retry_queue[replace_index].retry_count = 0;
        esp->retry_queue[replace_index].active = 1;
        
        return ESP8266_OK;
    } else {
        // 寻找空闲槽位
        for (uint8_t i = 0; i < ESP8266_RETRY_QUEUE_SIZE; i++) {
            if (!esp->retry_queue[i].active) {
                // 添加新消息
                strncpy(esp->retry_queue[i].topic, topic, ESP8266_MAX_TOPIC_SIZE - 1);
                esp->retry_queue[i].topic[ESP8266_MAX_TOPIC_SIZE - 1] = '\0';
                
                strncpy(esp->retry_queue[i].message, message, ESP8266_MAX_MESSAGE_SIZE - 1);
                esp->retry_queue[i].message[ESP8266_MAX_MESSAGE_SIZE - 1] = '\0';
                
                esp->retry_queue[i].qos = qos;
                esp->retry_queue[i].retain = retain;
                esp->retry_queue[i].timestamp = HAL_GetTick();
                esp->retry_queue[i].retry_count = 0;
                esp->retry_queue[i].active = 1;
                
                esp->retry_count++;
                return ESP8266_OK;
            }
        }
    }
    
    return ESP8266_ERROR;
}

/**
 * @brief 处理重发队列，应在主循环中调用
 * @param esp ESP8266_UART2_HandleTypeDef指针
 */
void ESP8266_UART2_ProcessRetryQueue(ESP8266_UART2_HandleTypeDef *esp) {
    if (esp == NULL) {
        return;
    }
    
    uint32_t current_time = HAL_GetTick();
    
    // 检查是否达到重试时间间隔
    if (current_time - esp->last_retry_time < ESP8266_RETRY_INTERVAL) {
        return;
    }
    
    esp->last_retry_time = current_time;
    
    // 遍历重试队列
    for (int i = 0; i < ESP8266_MAX_RETRY_QUEUE_SIZE; i++) {
        if (esp->retry_queue[i].in_use) {
            // 如果未达到最大重试次数，尝试重发
            if (esp->retry_queue[i].retry_count < ESP8266_MAX_RETRY_ATTEMPTS) {
                // 重发MQTT消息
                char cmd[256];
                snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",%d,%d",
                         esp->retry_queue[i].topic,
                         esp->retry_queue[i].message,
                         esp->retry_queue[i].qos,
                         esp->retry_queue[i].retain);
                
                int result = ESP8266_UART2_SendCommand(esp, cmd, "OK", 2000);
                
                if (result == ESP8266_OK) {
                    // 重发成功，从队列中移除
                    esp->retry_queue[i].in_use = 0;
                    memset(esp->retry_queue[i].topic, 0, ESP8266_MAX_TOPIC_SIZE);
                    memset(esp->retry_queue[i].message, 0, ESP8266_MAX_MESSAGE_SIZE);
                } else {
                    // 重发失败，增加重试计数
                    esp->retry_queue[i].retry_count++;
                }
            } else {
                // 达到最大重试次数，从队列中移除
                esp->retry_queue[i].in_use = 0;
                memset(esp->retry_queue[i].topic, 0, ESP8266_MAX_TOPIC_SIZE);
                memset(esp->retry_queue[i].message, 0, ESP8266_MAX_MESSAGE_SIZE);
            }
        }
    }
}

/**
 * @brief 清空重发队列
 * @param esp ESP8266_UART2_HandleTypeDef指针
 */
void ESP8266_UART2_ClearRetryQueue(ESP8266_UART2_HandleTypeDef *esp) {
    if (esp == NULL) {
        return;
    }
    
    // 清空重发队列
    memset(esp->retry_queue, 0, sizeof(ESP8266_RetryMessage_t) * ESP8266_RETRY_QUEUE_SIZE);
    esp->retry_count = 0;
} 