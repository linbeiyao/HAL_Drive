#ifndef ESP8266_MQTT_H
#define ESP8266_MQTT_H

#include "main.h" // 根据你的 STM32 系列进行调整
#include <string.h>
#include <stdio.h>

// 定义宏
#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"
#define MQTT_SERVER "your_mqtt_server"
#define MQTT_PORT "your_mqtt_port"
#define MQTT_USERNAME "your_mqtt_username"
#define MQTT_PASSWORD "your_mqtt_password"
#define MQTT_CLIENT_ID "your_mqtt_client_id"
#define MQTT_IP "IP"
#define MQTT_PORT_NUM "PORT"
#define MQTT_TOPIC "/your_topic"            // 替换为你的 MQTT 主题  
#define TIMEOUT 1000



// 定义最大缓冲区大小
#define ESP8266_MAX_BUFFER_SIZE 1024
#define ESP8266_MAX_CMD_SIZE 256
#define ESP8266_MAX_RESPONSE_SIZE 1024
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

// 定义 ESP8266 句柄结构体
typedef struct
{
    UART_HandleTypeDef *huart;                                  // 指向 UART 句柄
    uint8_t rx_buffer[ESP8266_MAX_BUFFER_SIZE];                 // 接收缓冲区
    volatile uint16_t rx_index;                                 // 当前接收缓冲区索引
    char response_buffer[ESP8266_MAX_RESPONSE_SIZE];            // 响应缓冲区
    volatile int response_received;                             // 响应接收标志
    MQTT_Status mqtt_status;                                    // 当前 MQTT 状态
    // MQTT 消息回调函数
    void (*mqtt_message_callback)(const char *topic, const char *message);
} ESP8266_HandleTypeDef;

// extern ESP8266_HandleTypeDef esp8266_handler;

// 函数声明
void            ESP8266_Init                (ESP8266_HandleTypeDef *esp, UART_HandleTypeDef *huart);
void            ESP8266_UART_IRQHandler     (ESP8266_HandleTypeDef *esp);

int             SP8266_SendCommand          (ESP8266_HandleTypeDef *esp, const char *cmd, const char *expected_response, uint32_t timeout);
int             SP8266_ConnectWiFi          (ESP8266_HandleTypeDef *esp, const char *ssid, const char *password, uint32_t timeout);
int             SP8266_SetMQTTConfig        (ESP8266_HandleTypeDef *esp, const char *username, const char *password, const char *client_id, uint32_t timeout);
int             SP8266_ConnectMQTT          (ESP8266_HandleTypeDef *esp, const char *broker_ip, uint16_t port, uint32_t timeout);
int             SP8266_SubscribeMQTT        (ESP8266_HandleTypeDef *esp, const char *topic, MQTT_QoS qos, uint32_t timeout);
int             SP8266_PublishMQTT          (ESP8266_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
int             SP8266_PublishMQTT_JSON     (ESP8266_HandleTypeDef *esp, const char *topic, const char *key, const char *value, MQTT_QoS qos, uint8_t retain, uint32_t timeout);
int             SP8266_DisconnectMQT        (ESP8266_HandleTypeDef *esp, uint32_t timeout);
int             SP8266_Close                (ESP8266_HandleTypeDef *esp, uint32_t timeout);

void            my_mqtt_callback            (const char *topic, const char *message);

// JSON 工具函数
int             ESP8266_BuildJSON           (char *buffer, size_t buffer_size, const char *key, const char *value);

#endif // ESP8266_MQTT_H

