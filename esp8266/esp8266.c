#include "esp8266.h"

/**
 * ESP8266 初始化函数
 *
 * @param esp ESP8266 的处理结构体指针，用于存储 ESP8266 的相关状态和数据
 * @param huart UART 的处理结构体指针，用于 ESP8266 与 MCU 之间的串口通信
 *
 * 此函数负责初始化 ESP8266 的相关状态和缓冲区，并启动 UART 接收中断
 * 以便开始接收来自 ESP8266 模块的数据
 */
void ESP8266_Init(ESP8266_HandleTypeDef *esp, UART_HandleTypeDef *huart)
{
    // 将传入的 UART 处理结构体指针赋值给 ESP8266 处理结构体中的对应成员
    esp->huart = huart;

    // 清除接收缓冲区，确保初始状态为干净
    memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);

    // 初始化接收缓冲区索引为 0
    esp->rx_index = 0;

    // 清除响应缓冲区，用于存储 ESP8266 的响应数据
    memset(esp->response_buffer, 0, ESP8266_MAX_RESPONSE_SIZE);

    // 初始化响应接收状态标志为 0，表示尚未接收到有效响应
    esp->response_received = 0;

    // 初始化 MQTT 连接状态为断开
    esp->mqtt_status = MQTT_STATUS_DISCONNECTED;

    // 初始化 MQTT 消息回调函数指针为 NULL，表示尚未设置回调函数
    esp->mqtt_message_callback = NULL;

    // 启动 UART 接收中断，准备接收来自 ESP8266 模块的数据
    HAL_UART_Receive_IT(esp->huart, &esp->rx_buffer[esp->rx_index], 1);
}

// UART 中断处理函数（需要在实际的 UART IRQ Handler 中调用）
void ESP8266_UART_IRQHandler(ESP8266_HandleTypeDef *esp)
{
    if (__HAL_UART_GET_FLAG(esp->huart, UART_FLAG_RXNE))
    {
        uint8_t data;
        HAL_UART_Receive_IT(esp->huart, &data, 1); // 继续接收下一个字节
        if (esp->rx_index < ESP8266_MAX_BUFFER_SIZE - 1)
        {
            esp->rx_buffer[esp->rx_index++] = data;
            esp->rx_buffer[esp->rx_index] = '\0';

            // 检查是否接收到完整响应（以换行符结束）
            if (data == '\n')
            {
                esp->response_received = 1;
            }
        }
        else
        {
            // 缓冲区溢出，重置
            esp->rx_index = 0;
            memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
        }
    }
}

// 发送 AT 指令并等待期望的响应
int ESP8266_SendCommand(ESP8266_HandleTypeDef *esp, const char *cmd, const char *expected_response, uint32_t timeout)
{
    esp->response_received = 0;
    memset(esp->response_buffer, 0, ESP8266_MAX_RESPONSE_SIZE);
    esp->rx_index = 0;

    // 发送命令
    HAL_UART_Transmit(esp->huart, (uint8_t *)cmd, strlen(cmd), timeout);
    HAL_UART_Transmit(esp->huart, (uint8_t *)"\r\n", 2, timeout);

    // 等待响应
    uint32_t start = HAL_GetTick();
    while (!esp->response_received && (HAL_GetTick() - start) < timeout)
    {
        // 可以在这里添加低功耗模式或其他任务
    }

    // 复制接收到的响应
    strncpy(esp->response_buffer, esp->rx_buffer, esp->rx_index);
    esp->response_buffer[esp->rx_index] = '\0';
    esp->rx_index = 0;

    // 检查期望的响应是否存在
    if (strstr(esp->response_buffer, expected_response) != NULL)
    {
        return 1; // 成功
    }
    return 0; // 失败
}

// 连接到指定的 WiFi 网络
int ESP8266_ConnectWiFi(ESP8266_HandleTypeDef *esp, const char *ssid, const char *password, uint32_t timeout)
{
    char command[256];
    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

// 设置 MQTT 用户名、密码和客户端 ID
int ESP8266_SetMQTTConfig(ESP8266_HandleTypeDef *esp, const char *username, const char *password, const char *client_id, uint32_t timeout)
{
    char command[512];
    snprintf(command, sizeof(command), "AT+MQTTUSERCFG=\"%s\",\"%s\",\"%s\"", username, password, client_id);
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

// 连接到 MQTT Broker
int ESP8266_ConnectMQTT(ESP8266_HandleTypeDef *esp, const char *broker_ip, uint16_t port, uint32_t timeout)
{
    char command[512];
    snprintf(command, sizeof(command), "AT+MQTTCONNCFG=\"%s\",%d", broker_ip, port);
    if (ESP8266_SendCommand(esp, command, "OK", timeout))
    {
        esp->mqtt_status = MQTT_STATUS_CONNECTED;
        return 1;
    }
    return 0;
}

// 订阅 MQTT Topic
int ESP8266_SubscribeMQTT(ESP8266_HandleTypeDef *esp, const char *topic, MQTT_QoS qos, uint32_t timeout)
{
    char command[256];
    snprintf(command, sizeof(command), "AT+MQTTSUB=\"%s\",%d", topic, qos);
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

// 发布 MQTT 消息（字符串）
/**
 * AT+MQTTPUB=<LinkID>,<"topic">,<"data">,<qos>,<retain>
 * <LinkID>：当前仅支持 link ID 0。
 * <topic>：MQTT topic，最大长度：128 字节。
 * <data>：MQTT 字符串消息。
 * <qos>：发布消息的 QoS，参数可选 0、1、或 2，默认值：0。
 * <retain>：发布 retain。
 *
 * 响应：OK
 */
int ESP8266_PublishMQTT(ESP8266_HandleTypeDef *esp, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout)
{
    char command[512];
    snprintf(command, sizeof(command), "AT+MQTTPUB=\"%s\",\"%s\",%d,%d", topic, message, qos, retain);
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

// 发布 MQTT 消息（JSON 格式）
int ESP8266_PublishMQTT_JSON(ESP8266_HandleTypeDef *esp, const char *topic, const char *key, const char *value, MQTT_QoS qos, uint8_t retain, uint32_t timeout)
{
    char json_message[ESP8266_MAX_JSON_SIZE];
    // 构建简单的 JSON 字符串，如 {"key":"value"}
    int ret = snprintf(json_message, sizeof(json_message), "{\"%s\":\"%s\"}", key, value);
    if (ret < 0 || ret >= sizeof(json_message))
    {
        return 0; // JSON 构建失败
    }
    return ESP8266_PublishMQTT(esp, topic, json_message, qos, retain, timeout);
}

// 断开 MQTT 连接
int ESP8266_DisconnectMQTT(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    int result = ESP8266_SendCommand(esp, "AT+MQTTUNSUB", "OK", timeout);
    result &= ESP8266_SendCommand(esp, "AT+MQTTCLEAN", "OK", timeout);
    if (result)
    {
        esp->mqtt_status = MQTT_STATUS_DISCONNECTED;
    }
    return result;
}

// 关闭 ESP8266（复位）
int ESP8266_Close(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    int result = ESP8266_SendCommand(esp, "AT+RST", "OK", timeout);
    HAL_Delay(1000); // 等待复位完成
    return result;
}

// 构建简单的 JSON 字符串（键值对）
int ESP8266_BuildJSON(char *buffer, size_t buffer_size, const char *key, const char *value)
{
    int needed = snprintf(buffer, buffer_size, "{\"%s\":\"%s\"}", key, value);
    if (needed < 0 || (size_t)needed >= buffer_size)
    {
        return 0; // 构建失败
    }
    return 1; // 成功
}

// 设置 MQTT 消息回调函数
void ESP8266_SetMQTTMessageCallback(ESP8266_HandleTypeDef *esp, void (*callback)(const char *topic, const char *message))
{
    esp->mqtt_message_callback = callback;
}

// 定义回调函数
void my_mqtt_callback(const char *topic, const char *message)
{
    // 打印接收到的MQTT消息的主题和内容
    printf("Received MQTT Message:\n");
    printf("Topic: %s\r\n", topic);
    printf("Message: %s\r\n", message);
}

// 处理接收到的数据，应在主循环中定期调用
void ESP8266_ProcessReceivedData(ESP8266_HandleTypeDef *esp)
{
    if (esp->response_received)
    {
        // 复制响应
        strncpy(esp->response_buffer, esp->rx_buffer, esp->rx_index);
        esp->response_buffer[esp->rx_index] = '\0';
        esp->rx_index = 0;
        esp->response_received = 0;

        // 检查是否有 MQTT 消息
        char *mqtt_msg = strstr(esp->response_buffer, "+MQTTMSG:");
        if (mqtt_msg != NULL && esp->mqtt_message_callback != NULL)
        {
            char topic[ESP8266_MAX_TOPIC_SIZE];
            char message[ESP8266_MAX_MESSAGE_SIZE];
            // 假设格式为 +MQTTMSG:"topic","message"
            if (sscanf(mqtt_msg, "+MQTTMSG:\"%[^\"]\",\"%[^\"]\"", topic, message) == 2)
            {
                // 调用回调函数
                esp->mqtt_message_callback(topic, message);
            }
        }

        // 你可以在这里添加更多的响应处理逻辑
    }
}

/**
 * MQTT_Connect函数用于建立与MQTT服务器的连接，并订阅指定的主题。
 * 此函数依次执行以下步骤：
 * 1. 连接到WiFi网络。
 * 2. 设置MQTT服务器的配置信息。
 * 3. 建立与MQTT服务器的连接。
 * 4. 订阅指定的MQTT主题。
 */
void MQTT_Connect()
{
    // 连接到指定SSID和密码的WiFi网络，带超时设置
    ESP8266_ConnectWiFi(&g_fruitVendingData.esp8266, WIFI_SSID, WIFI_PASSWORD, TIMEOUT);

    // 设置MQTT服务器的连接配置，包括服务器地址、端口、用户名、密码、客户端ID和超时时间
    ESP8266_SetMQTTConfig(&g_fruitVendingData.esp8266, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, MQTT_CLIENT_ID, TIMEOUT);

    // 使用指定的IP地址和端口号连接到MQTT服务器，带超时设置
    ESP8266_ConnectMQTT(&g_fruitVendingData.esp8266, MQTT_IP, MQTT_PORT_NUM, TIMEOUT);

    // 订阅指定的MQTT主题，设置QoS级别为0，带超时设置
    ESP8266_SubscribeMQTT(&g_fruitVendingData.esp8266, MQTT_TOPIC, MQTT_QoS0, TIMEOUT);
}