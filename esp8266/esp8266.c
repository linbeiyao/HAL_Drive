#include "esp8266.h"
#include "button.h"
#include "oled.h"
#include "usart.h"

#include <stdarg.h>

/* 全局变量 */
uint8_t g_wifi_status_need_parse_CWSTATE = 0;
uint8_t g_wifi_status_need_parse_CWJAP = 0;
uint8_t g_mqtt_status_need_parse_MQTTCONN = 0;

uint8_t iswificonnflag = 0;

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
    esp->expected_response_received = 0; // 初始化预期响应标志

    // 初始化 MQTT 连接状态为断开
    esp->mqtt_status = MQTT_STATUS_DISCONNECTED;

    // 初始化 MQTT 消息回调函数指针为 NULL，表示尚未设置回调函数
    esp->mqtt_message_callback = NULL;

    // 启动 UART 接收中断，准备接收来自 ESP8266 模块的数据
    // HAL_UART_Receive_IT(esp->huart, &esp->rx_buffer[esp->rx_index], 1);

    // 使用 DMA 进行数据接受
    HAL_UART_Receive_DMA(esp->huart, dma_rx_buffer, DMA_BUFFER_SIZE);
}

#define DEBUG_ESP8266_IRQ 0

// UART 中断处理函数（在 USART1_IRQHandler 中调用）
void ESP8266_UART_IRQHandler(ESP8266_HandleTypeDef *esp)
{
    // 处理接收到的数据
    for (uint16_t i = 0; i < esp->rx_index; i++)
    {
        uint8_t data = esp->rx_buffer[i];

        if (esp->rx_index < ESP8266_MAX_BUFFER_SIZE - 1)
        {
            esp->rx_buffer[esp->rx_index++] = data;
            // 检测到结束符（例如 '\n'）
            if (data == '\n')
            {
                esp->rx_buffer[esp->rx_index] = '\0';
                esp->response_received = 1; // 标记响应接收完成

                ESP8266_ProcessReceivedData(esp);

                break;
            }
        }
        else
        {
            esp->rx_index = 0;
            memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);

            break;
        }
    }

#if DEBUG_ESP8266_IRQ
    // 调试信息：退出中断处理函数
    printf("[ESP8266_UART_IRQHandler] Exit, rx_index: %d\r\n", esp->rx_index);
#endif
}

// 发送 AT 指令并等待期望的响应
int ESP8266_SendCommand(ESP8266_HandleTypeDef *esp, const char *cmd, const char *expected_response, uint32_t timeout_ms)
{
    // 初始化操作
    esp->expected_response_received = 0; // 重置预期响应标志
    esp->rx_index = 0;
    esp->expected_response = expected_response;

    // 构建并发送AT命令
    char full_cmd[ESP8266_MAX_CMD_SIZE];
    int cmd_len = snprintf(full_cmd, sizeof(full_cmd), "%s\r\n", cmd);

    HAL_StatusTypeDef uart_status = HAL_UART_Transmit(esp->huart, (uint8_t *)full_cmd, cmd_len, 2000);
    if (uart_status != HAL_OK)
    {
        esp->expected_response = NULL;
        return ESP8266_ERROR;
    }

    // 等待响应或超时
    uint32_t start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < timeout_ms)
    {
        if (esp->expected_response_received) // 使用新的标志位
        {
            esp->expected_response = NULL; // 清除期望响应
            return ESP8266_OK;
        }
        HAL_Delay(1);
    }

    // 超时处理
    printf("[ESP8266] Command timeout: %s\r\n", cmd);
    esp->expected_response = NULL; // 清除期望响应
    esp->rx_index = 0;
    return ESP8266_ERROR_TIMEOUT;
}

// 连接到指定的 WiFi 网络
int ESP8266_ConnectWiFi(ESP8266_HandleTypeDef *esp, const char *ssid, const char *password, uint32_t timeout)
{
    char command[256];
    // AT+CWJAP="abc","0123456789"
    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    if (ESP8266_SendCommand(esp, command, "WIFI CONNECTED", timeout))
    {
        if (ESP8266_SendCommand(esp, "", "WIFI GOT IP", timeout))
        {
            if (ESP8266_SendCommand(esp, "", "OK", timeout))
            {

                return 1; // 成功
            }
        }
    }
    return 0; // 失败
}

int ESP8266_ReconnectWiFi(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    char command[256];
    // AT+CWJAP="abc","0123456789"
    snprintf(command, sizeof(command), "AT+CWJAP");
    if (ESP8266_SendCommand(esp, command, "OK", timeout) == ESP8266_OK)
    {
        return 1;
    }
    return 0;
}

// 查询WiFi连接状态
// 当 ESP station 没有连接上 AP 时，推荐使用此命令查询 Wi-Fi 信息;
// 当 ESP station 已连接上 AP 后，推荐使用 AT+CWJAP 命令查询 Wi-Fi 信息;
int ESP8266_QueryWiFiStatus_CWSTATE(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    uint8_t status;
    char command[256];
    snprintf(command, sizeof(command), "AT+CWSTATE?\r\n");
    status = HAL_UART_Transmit(esp->huart, (uint8_t *)command, strlen(command), timeout);
    if (status != HAL_OK)
    {

        if (status == HAL_ERROR)
        {
            printf("ESP8266_QueryWiFiStatus: HAL_ERROR\r\n");
            return status;
        }
        else if (status == HAL_BUSY)
        {
            printf("ESP8266_QueryWiFiStatus: HAL_BUSY\r\n");
            return status;
        }
        else if (status == HAL_TIMEOUT)
        {
            printf("ESP8266_QueryWiFiStatus: HAL_TIMEOUT\r\n");
            return status;
        }
        else
        {
            printf("ESP8266_QueryWiFiStatus: Unknown error\r\n");
            return status;
        }
    }

    g_wifi_status_need_parse_CWSTATE = 1;
    return status;
}

// 查询与ESP Station连接的AP信息
// 当 ESP station 已连接上 AP 后，推荐使用 AT+CWJAP 命令查询 Wi-Fi 信息;
// 当 ESP station 没有连接上 AP 时，推荐使用 AT+CWSTATE 命令查询 Wi-Fi 信息;
int ESP8266_QueryWiFiStatus_CWJAP(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    char command[256];
    snprintf(command, sizeof(command), "AT+CWJAP?\r\n");
    int status = HAL_UART_Transmit(esp->huart, (uint8_t *)command, strlen(command), timeout);
    if (status != HAL_OK)
    {
        if (status == HAL_ERROR)
        {
            printf("ESP8266_QueryWiFiStatus: HAL_ERROR\r\n");
            return status;
        }
        else if (status == HAL_BUSY)
        {
            printf("ESP8266_QueryWiFiStatus: HAL_BUSY\r\n");
            return status;
        }
        else if (status == HAL_TIMEOUT)
        {
            printf("ESP8266_QueryWiFiStatus: HAL_TIMEOUT\r\n");
            return status;
        }
        else
        {
            printf("ESP8266_QueryWiFiStatus: Unknown error\r\n");
            return status;
        }
    }

    g_wifi_status_need_parse_CWJAP = 1;
    return status;
}

// 设置 MQTT 用户名、密码、客户端 ID 以及连接方案
int ESP8266_SetMQTTConfig(ESP8266_HandleTypeDef *esp, int link_id, int scheme, const char *client_id, const char *username, const char *password, int cert_key_ID, int CA_ID, const char *path, uint32_t timeout)
{
    char command[512];
    // 构建 AT+MQTTUSERCFG 命令，格式为：AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">
    //
    snprintf(command, sizeof(command), "AT+MQTTUSERCFG=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"", link_id, scheme, client_id, username, password, cert_key_ID, CA_ID, path);
    // 发送命令并等待响应
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

/**
 * @brief 设置MQTT遗嘱信息
 *
 * @param esp ESP8266句柄
 * @param link_id 连接ID（当前仅支持0）
 * @param topic 遗嘱主题，最大长度128字节
 * @param message 遗嘱消息，最大长度64字节
 * @param qos 遗嘱QoS等级（0/1/2，默认0）
 * @param retain 遗嘱保留标志（0/1，默认0）：保留消息，如果为1，则消息在设备异常断开时发送到指定主题 保留到主题下次订阅
 * @param timeout 超时时间
 * @return int 执行结果
 *
 * @note 使用AT+MQTTCONNCFG命令设置MQTT连接属性：
 * 命令格式：AT+MQTTCONNCFG=<LinkID>,<keepalive>,<disable_clean_session>,<"lwt_topic">,<"lwt_msg">,<lwt_qos>,<lwt_retain>
 * 参数说明：
 * - keepalive: MQTT ping超时时间，单位秒，范围[0,7200]，默认0（会被强制改为120秒）
 * - disable_clean_session: MQTT清理会话标志
 *   - 0: 使能清理会话
 *   - 1: 禁用清理会话
 *
 * @attention 遗嘱信息将在设备异常断开时发送到指定主题
 */
int ESP8266_SetMQTTWill(ESP8266_HandleTypeDef *esp, int link_id, const char *topic, const char *message, MQTT_QoS qos, uint8_t retain, uint32_t timeout)
{
    char command[512];
    // 构建 AT+MQTTCONNCFG 命令，设置遗嘱信息
    snprintf(command, sizeof(command), "AT+MQTTCONNCFG=%d,120,0,\"%s\",\"%s\",%d,%d",
             link_id, topic, message, qos, retain);
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

/**
 * @brief MQTT Broker 连接参数说明
 *
 * @param LinkID 连接ID，当前仅支持0
 * @param host MQTT broker域名，最大长度128字节
 * @param port MQTT broker端口，范围0-65535
 * @param path 资源路径，最大长度32字节
 * @param reconnect 自动重连标志：
 *                  - 0: 不自动重连
 *                  - 1: 自动重连（会消耗较多内存）
 * @param state MQTT状态：0-未初始化,1-已设用户配置,2-已设连接配置,3-连接断开,4-已连接,5-已连未订阅,6-已连已订阅
 * @param scheme 连接方案：1-MQTT/TCP,2-MQTT/TLS(不校验),3-MQTT/TLS(校服),4-MQTT/TLS(客证),5-MQTT/TLS(双向),
 *              6-MQTT/WS(TCP),7-MQTT/WSS(不校验),8-MQTT/WSS(校服),9-MQTT/WSS(客证),10-MQTT/WSS(双向)
 */
int ESP8266_ConnectMQTT(ESP8266_HandleTypeDef *esp, int link_id, const char *broker_ip, uint16_t port, int reconnect, uint32_t timeout)
{
    char command[512];
    snprintf(command, sizeof(command), "AT+MQTTCONN=%d,\"%s\",%d,%d", link_id, broker_ip, port, reconnect);

    int ret = ESP8266_SendCommand(esp, command, "OK", timeout);
    if (ret == ESP8266_OK)
    {
        esp->mqtt_status = MQTT_STATUS_CONNECTED;
        return 1; // 连接成功
    }
    else
    {
        esp->mqtt_status = MQTT_STATUS_DISCONNECTED;
        return 0; // 连接失败
    }
}

int ESP8266_QueryMQTTStatus_Connect(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    char command[256];
    snprintf(command, sizeof(command), "AT+MQTTCONN?\r\n");
    int status = HAL_UART_Transmit(esp->huart, (uint8_t *)command, strlen(command), timeout);
    if (status != HAL_OK)
    {
        if (status == HAL_ERROR)
        {
            printf("ESP8266_QueryMQTTStatus_Connect: HAL_ERROR\r\n");
            return status;
        }
        else if (status == HAL_BUSY)
        {
            printf("ESP8266_QueryMQTTStatus_Connect: HAL_BUSY\r\n");
            return status;
        }
        else if (status == HAL_TIMEOUT)
        {
            printf("ESP8266_QueryMQTTStatus_Connect: HAL_TIMEOUT\r\n");
            return status;
        }
        else
        {
            printf("ESP8266_QueryMQTTStatus_Connect: Unknown error\r\n");
            return status;
        }
    }

    g_mqtt_status_need_parse_MQTTCONN = 1;
    return status;
}

// 订阅 MQTT Topic
int ESP8266_SubscribeMQTT(ESP8266_HandleTypeDef *esp, int link_id, const char *topic, MQTT_QoS qos, uint32_t timeout)
{
    char command[256];
    snprintf(command, sizeof(command), "AT+MQTTSUB=%d,\"%s\",%d", link_id, topic, qos);
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
    // AT+MQTTPUB=0,"topic","\"{\"timestamp\":\"20201121085253\"}\"",0,0  // 发送此命令时，请注意特殊字符是否需要转义。
    snprintf(command, sizeof(command), "AT+MQTTPUB=0,\"%s\",\"%s\",%d,%d", topic, message, qos, retain);

    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

// 发布 MQTT 消息（JSON 格式）
int ESP8266_PublishMQTT_JSON(ESP8266_HandleTypeDef *esp, const char *topic, const char *key, const char *value, MQTT_QoS qos, uint8_t retain, uint32_t timeout)
{
    char json_message[ESP8266_MAX_JSON_SIZE];
    // 构建简单的 JSON 字符串，如 {"key":"value"}
    // AT+MQTTPUB=0,"topic","\"{\"timestamp\":\"20201121085253\"}\"",0,0  // 发送此命令时，请注意特殊字符是否需要转义。
    int ret = snprintf(json_message, sizeof(json_message), "\"{\"%s\":\"%s\"}\"", key, value);
    if (ret < 0 || ret >= sizeof(json_message))
    {
        return 0; // JSON 构建失败
    }
    return ESP8266_PublishMQTT(esp, topic, json_message, qos, retain, timeout);
}

// 发送已经存在的json数据,带重试机制
int ESP8266_SendJSON(ESP8266_HandleTypeDef *esp, const char *topic, const char *json_data, uint32_t timeout)
{
    const uint8_t MAX_RETRY = 3;      // 最大重试次数
    const uint16_t RETRY_DELAY = 500; // 重试间隔(ms)
    uint8_t retry_count = MAX_RETRY;  // 当前剩余重试次数

    while (retry_count > 0)
    {
        char json_str[128];
        sprintf(json_str, "AT+MQTTPUB=0,\"%s\",\"%s\",0,0", topic, json_data);

        int result = ESP8266_SendCommand(esp, json_str, "OK", timeout);
        if (result == ESP8266_OK)
        {
            printf("Network connected, send data to server successfully\n");
            return ESP8266_OK;
        }

        retry_count--;
        if (retry_count > 0)
        {
            printf("Send failed, retrying... (%d attempts left)\n", retry_count);
            HAL_Delay(RETRY_DELAY);
        }
    }

    printf("Network not connected after %d retries\n", MAX_RETRY);
    return ESP8266_ERROR;
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
    if (buffer == NULL || key == NULL || value == NULL)
    {
        return 0; // 无效参数
    }

    int needed = snprintf(buffer, buffer_size, "\"{\"%s\":\"%s\"}\"", key, value);
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
    // 检查是否有接收到响应
    if (!esp->response_received)
        return;

    char *current_ptr = (char *)esp->rx_buffer;
    char json_buffer[256] = {0};

    while (*current_ptr)
    {
        printf("[ESP8266] Processing packet: %s\r\n", current_ptr);

        // 处理 WiFi 连接状态
        if (strstr(current_ptr, "WIFI CONNECTED"))
            iswificonnflag = 1;

        // 检查 MQTT 连接状态和数据
        if (esp->mqtt_status == MQTT_STATUS_CONNECTED)
        {
            // 尝试提取 MQTT 订阅消息
            if (sscanf(current_ptr, "+MQTTSUBRECV:%*d,\"%*[^\"]\",%*d,%255[^\n]", json_buffer) == 1)
            {
                // 根据操作类型进行不同处理
                char operation[20] = {0};
                if (sscanf(json_buffer, "{\"operation\":\"%19[^\"]\"", operation) == 1)
                {
                    if (strcmp(operation, "recharge") == 0)     // 请修改为要处理的操作
                    {

                    }
                    else if (strcmp(operation, "replenish") == 0)       // 请修改为要处理的操作
                    {
                      
                    }

                    // 移动剩余数据到缓冲区头部
                    char *next = strchr(current_ptr, '\r\n');
                    if (next && *(next + 1))
                    {
                        size_t keep_len = strlen(next + 1);
                        memmove(esp->rx_buffer, next + 1, keep_len + 1);
                        esp->rx_index = keep_len;
                        return;
                    }
                }
            }
        }

        // 检查是否匹配期望的响应
        if (esp->expected_response != NULL &&
            strstr(current_ptr, esp->expected_response) != NULL)
        {
            esp->expected_response_received = 1;
            break;
        }

        // 跳转到下一个数据包
        char *next = strchr(current_ptr, '\r\n');
        if (!next)
            break;
        current_ptr = next + 1;
    }

    // 清空接收缓冲区
    if (esp->mqtt_status == MQTT_STATUS_CONNECTED)
    {
        esp->rx_index = 0;
        memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
    }
}

#define DEBUG_MQTT_CONNECT 0 // 控制调试信息输出，1为启用，0为禁用

void MQTT_Connect(ESP8266_HandleTypeDef *data)
{
    int result;
    const char *commands[] = {
        "AT", "ATE0\r\n", "AT+CWMODE=1", "AT+CWDHCP=1,1", "AT+CWJAP=\"1\",\"88888888\""};
    const char *responses[] = {
        "OK", "OK", "OK", "OK", "OK"};
    const int timeouts[] = {
        3000, 10, TIMEOUT, TIMEOUT, 10000};
    const int delays[] = {
        200, 200, 200, 200, 200};

    for (int i = 0; i < 5; i++)
    {
        while ((result = ESP8266_SendCommand(data, commands[i], responses[i], timeouts[i])) != ESP8266_OK)
        {
            if (i == 4 && iswificonnflag)
                break;
            HAL_Delay(delays[i]);
        }
        HAL_Delay(delays[i]);
    }

    ESP8266_SetMQTTConfig(data, 0, 1, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, 0, 0, "", TIMEOUT);
    ESP8266_SetMQTTWill(data, 0, MQTT_TOPIC_Will, MQTT_TOPIC_Will_Message, MQTT_QoS0, 1, TIMEOUT);
    ESP8266_ConnectMQTT(data, 0, MQTT_IP, MQTT_PORT, 1, 2000);
    ESP8266_SubscribeMQTT(data, 0, MQTT_TOPIC_Subscribe, MQTT_QoS0, TIMEOUT);

    data->mqtt_status = MQTT_STATUS_CONNECTED;
    UIManager_SwitchScreen(SCREEN_MAIN);
}

/**
 * @brief 通用的 JSON 打包模板函数
 *
 * 该函数根据传入的格式字符串 fmt 以及后续变参，
 * 利用 vsprintf 将数据格式化到 json_str 缓冲区中。
 * 注意：格式字符串中需对双引号 (") 和逗号 (,) 进行转义，
 * 例如：\\" 表示实际输出 \"，\\, 表示输出 \,。
 *
 * @param fmt      格式模板字符串，要求包含转义符，如：
 *                 "\\\"{\\\"key1\\\":\\\"%d\\\"\\,\\\"key2\\\":\\\"%s\\\"}\\\""
 * @param json_str 存放生成 JSON 字符串的缓冲区指针
 * @param ...      格式字符串对应的参数列表
 */
void JSON_Template_Pack(const char *fmt, char *json_str, ...)
{
    va_list args;
    va_start(args, json_str);
    vsprintf(json_str, fmt, args);
    va_end(args);
}
