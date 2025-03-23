#ifndef ESP8266_MQTT_H
#define ESP8266_MQTT_H

#include "main.h" 
#include "NETConfig.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"


// ESP-AT指令集网址：https://espressif-docs.readthedocs-hosted.com/projects/esp-at/zh-cn/release-v2.2.0.0_esp8266/




// 这里使用 ESP8266 的头文件
// 使用前请修改对应的宏定义
//
//
// 请修改USART中断中 ESP 对象的的传递
//
//
// 



// 环形缓冲区定义
#define RING_BUFFER_SIZE 1024

// 定义DMA缓冲区大小
#define ESP8266_DMA_BUFFER_SIZE 64

// 定义重发消息队列长度
#define ESP8266_RETRY_QUEUE_SIZE 10
#define ESP8266_MAX_RETRY_COUNT 3

// 定义宏
#define WIFI_SSID "1"
#define WIFI_PASSWORD "88888888"
#define MQTT_SERVER "47.104.253.100"
#define MQTT_PORT 1883
#define MQTT_USERNAME "FFVending"
#define MQTT_PASSWORD "FFVending"
#define MQTT_CLIENT_ID "1"
#define MQTT_IP "47.104.253.100"
#define MQTT_PORT_NUM "18083"
#define MQTT_TOPIC_Subscribe "/FFVending/control"            // 替换为你的 MQTT 主题  
#define MQTT_TOPIC_Publish "/FFVending/data"
#define MQTT_TOPIC_Will "/FFVending/Will"
#define MQTT_TOPIC_Will_Message "FFVending:offline"

#define TIMEOUT 1000

typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR_TIMEOUT,
    ESP8266_ERROR_RESPONSE,
    ESP8266_ERROR_WIFI,
    ESP8266_ERROR_MQTT,
    ESP8266_ERROR,
    ESP8266_ERROR_BUSY,
    ESP8266_WAITING
} ESP8266_Status;

// 命令状态枚举
typedef enum {
    CMD_STATE_IDLE,
    CMD_STATE_WAITING,
    CMD_STATE_COMPLETED,
    CMD_STATE_TIMEOUT
} ESP8266_CmdState;

// 定义最大缓冲区大小
#define ESP8266_MAX_BUFFER_SIZE 512
#define ESP8266_MAX_CMD_SIZE 256
#define ESP8266_MAX_RESPONSE_SIZE 512
#define ESP8266_MAX_TOPIC_SIZE 128
#define ESP8266_MAX_MESSAGE_SIZE 512
#define ESP8266_MAX_JSON_SIZE 512

// 定义 MQTT 服务质量（QoS）
typedef enum
{
    MQTT_QoS0 = 0,
    MQTT_QoS1 = 1,
    MQTT_QoS2 = 2
} MQTT_QoS;

// 定义 MQTT 状态
typedef enum
{
    MQTT_STATUS_DISCONNECTED = 0,
    MQTT_STATUS_CONNECTED
} MQTT_Status;

// 定义 WIFI 状态
typedef enum
{
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTED
} WIFI_Status;

// 环形缓冲区结构体
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
} ESP8266_RingBuffer_t;

// 消息重发结构体
typedef struct {
    char topic[ESP8266_MAX_TOPIC_SIZE];
    char message[ESP8266_MAX_MESSAGE_SIZE];
    MQTT_QoS qos;
    uint8_t retain;
    uint32_t timestamp;  // 上次发送时间
    uint8_t retry_count; // 已重试次数
    uint8_t active;      // 是否有效
} ESP8266_RetryMessage_t;

// 定义 ESP8266 句柄结构体
typedef struct
{
    UART_HandleTypeDef *huart;                                  // 指向 UART 句柄
    uint8_t rx_buffer[ESP8266_MAX_BUFFER_SIZE];                // 接收缓冲区
    volatile uint16_t rx_index;                                // 当前接收缓冲区索引
    char response_buffer[ESP8266_MAX_RESPONSE_SIZE];           // 响应缓冲区
    volatile int response_received;                            // 响应接收标志
    volatile int expected_response_received;                   // 预期响应接收标志
    MQTT_Status mqtt_status;                                   // 当前 MQTT 状态

    WIFI_Status init_wifi_status;                              // 当前 WIFI 状态        此状态 回调函数中不进行处理
    void (*mqtt_message_callback)(const char *topic, const char *message); // MQTT 消息回调函数

    const char *expected_response;                             // 期望的响应字符串
    volatile uint8_t immediate_process_flag;                   // 立即处理标志，1表示需要立即处理，0表示在主循环中处理

} ESP8266_HandleTypeDef;

// DMA+环形缓冲区扩展结构体
typedef struct {
    ESP8266_HandleTypeDef esp_base;                      // 基础ESP8266句柄
    DMA_HandleTypeDef *hdma_rx;                          // DMA接收句柄
    ESP8266_RingBuffer_t ring_buffer;                    // 环形缓冲区
    uint8_t dma_buffer[ESP8266_DMA_BUFFER_SIZE];         // DMA缓冲区
    volatile uint8_t dma_busy;                           // DMA忙标志
    volatile uint8_t data_ready;                         // 数据就绪标志
    ESP8266_RetryMessage_t retry_queue[ESP8266_RETRY_QUEUE_SIZE]; // 重发消息队列
    uint8_t retry_count;                                 // 当前队列中的消息数量
    uint32_t last_retry_time;                            // 上次重试时间
} ESP8266_UART2_HandleTypeDef;

// 全局变量
extern uint8_t g_wifi_status_need_parse_CWSTATE;
extern uint8_t g_wifi_status_need_parse_CWJAP;
extern uint8_t g_mqtt_status_need_parse_MQTTCONN;

// extern ESP8266_HandleTypeDef esp8266_handler;

// 函数声明
void            ESP8266_Init                		(ESP8266_HandleTypeDef *esp, UART_HandleTypeDef *huart);
void            ESP8266_UART_IRQHandler     		(ESP8266_HandleTypeDef *esp);

int             ESP8266_SendCommand           		(ESP8266_HandleTypeDef *esp, const char *cmd, const char *expected_response, uint32_t timeout_ms);
int             ESP8266_ProcessCommandState    		(ESP8266_HandleTypeDef *esp);
ESP8266_CmdState ESP8266_GetCommandState       		(void);
void            ESP8266_ResetCommandState      		(void);
int             ESP8266_ConnectWiFi            		(ESP8266_HandleTypeDef *esp, const char *ssid, const char *password, uint32_t timeout);
int             ESP8266_ReconnectWiFi          	    (ESP8266_HandleTypeDef *esp, uint32_t timeout);
int             ESP8266_QueryWiFiStatus_CWSTATE     (ESP8266_HandleTypeDef *esp, uint32_t timeout);
int             ESP8266_QueryWiFiStatus_CWJAP       (ESP8266_HandleTypeDef *esp, uint32_t timeout);
void            ESP8266_EnableImmediateProcessing   (ESP8266_HandleTypeDef *esp);
void            ESP8266_DisableImmediateProcessing  (ESP8266_HandleTypeDef *esp);
int             ESP8266_SetMQTTConfig        		(ESP8266_HandleTypeDef *esp, int link_id, int scheme, const char *client_id, const char *username, const char *password, int cert_key_ID, int CA_ID, const char *path, uint32_t timeout);
int             ESP8266_SetMQTTWill        		    (ESP8266_HandleTypeDef *esp, int link_id, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
int             ESP8266_ConnectMQTT          		(ESP8266_HandleTypeDef *esp, int link_id, const char *broker_ip, uint16_t port, int reconnect, uint32_t timeout);
int             ESP8266_QueryMQTTStatus_Connect     (ESP8266_HandleTypeDef *esp, uint32_t timeout);
int             ESP8266_SubscribeMQTT        		(ESP8266_HandleTypeDef *esp, int link_id, const char *topic, MQTT_QoS qos, uint32_t timeout);
int             ESP8266_PublishMQTT          		(ESP8266_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
int             ESP8266_PublishMQTT_JSON     		(ESP8266_HandleTypeDef *esp, const char *topic, const char *key, const char *value, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
uint8_t         ESP8266_SendJSON            		(ESP8266_HandleTypeDef *esp, const char *topic, const char *json_data, uint32_t timeout);
int             ESP8266_DisconnectMQTT      		(ESP8266_HandleTypeDef *esp, uint32_t timeout);
int             ESP8266_Close                		(ESP8266_HandleTypeDef *esp, uint32_t timeout);
void            ESP8266_SetMQTTMessageCallback	    (ESP8266_HandleTypeDef *esp, void (*callback)(const char *topic, const char *message));
void            ESP8266_ProcessReceivedData			(ESP8266_HandleTypeDef *esp);

void            my_mqtt_callback            		(const char *topic, const char *message);
void            MQTT_Connect                		(ESP8266_HandleTypeDef *data);


uint8_t         ESP8266_CallBack                            (ESP8266_HandleTypeDef *esp,const char *message);
uint8_t         ESP8266_QueryWiFiStatus_CWSTATE_Callback    (ESP8266_HandleTypeDef *esp,const char *response);
uint8_t         ESP8266_QueryWiFiStatus_CWJAP_Callback      (ESP8266_HandleTypeDef *esp,const char *response);
uint8_t         ESP8266_QueryMQTTStatus_Connect_Callback    (ESP8266_HandleTypeDef *esp,const char *response);


// JSON 工具函数
int             ESP8266_BuildJSON           		(char *buffer, size_t buffer_size, const char *key, const char *value);
void 						JSON_Template_Pack							(const char *fmt, char *json_str, ...);

// 环形缓冲区函数
void            ESP8266_RingBuffer_Init              (ESP8266_RingBuffer_t *ring);
uint16_t        ESP8266_RingBuffer_Write             (ESP8266_RingBuffer_t *ring, uint8_t *data, uint16_t length);
uint16_t        ESP8266_RingBuffer_Read              (ESP8266_RingBuffer_t *ring, uint8_t *data, uint16_t length);
uint16_t        ESP8266_RingBuffer_Available         (ESP8266_RingBuffer_t *ring);
void            ESP8266_RingBuffer_Flush             (ESP8266_RingBuffer_t *ring);

// UART2+DMA函数
void            ESP8266_UART2_Init                   (ESP8266_UART2_HandleTypeDef *esp, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma_rx);
void            ESP8266_UART2_IRQHandler             (ESP8266_UART2_HandleTypeDef *esp);
void            ESP8266_UART2_Process                (ESP8266_UART2_HandleTypeDef *esp);
int             ESP8266_UART2_SendCommand            (ESP8266_UART2_HandleTypeDef *esp, const char *cmd, const char *expected_response, uint32_t timeout_ms);

// 消息重发函数
int             ESP8266_UART2_PublishMQTT            (ESP8266_UART2_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
int             ESP8266_UART2_PublishMQTT_JSON       (ESP8266_UART2_HandleTypeDef *esp, const char *topic, const char *key, const char *value, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
int             ESP8266_UART2_AddToRetryQueue        (ESP8266_UART2_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain);
void            ESP8266_UART2_ProcessRetryQueue      (ESP8266_UART2_HandleTypeDef *esp);
void            ESP8266_UART2_ClearRetryQueue        (ESP8266_UART2_HandleTypeDef *esp);

#endif // ESP8266_MQTT_H

