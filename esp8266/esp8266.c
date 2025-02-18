#include "esp8266.h"
#include "button.h"
#include "oled.h"
#include "usart.h"
#include "FFVending.h"

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
    // HAL_UART_Receive_IT(esp->huart, &esp->rx_buffer[esp->rx_index], 1);

    // 使用 DMA 进行数据接受
    HAL_UART_Receive_DMA(&huart1, dma_rx_buffer, DMA_BUFFER_SIZE);
}

#define DEBUG_ESP8266_IRQ 0

// UART 中断处理函数（在 USART1_IRQHandler 中调用）
void ESP8266_UART_IRQHandler(ESP8266_HandleTypeDef *esp)
{
#if DEBUG_ESP8266_IRQ
    // 调试信息：进入中断处理函数
    printf("[ESP8266_UART_IRQHandler] Enter, rx_index: %d\r\n", esp->rx_index);
#endif

    // 处理接收到的数据
    for (uint16_t i = 0; i < esp->rx_index; i++)
    {
        uint8_t data = esp->rx_buffer[i];

#if DEBUG_ESP8266_IRQ
        // 调试信息：显示当前处理的数据
        printf("[ESP8266_UART_IRQHandler] Processing data[%d]: 0x%02X\r\n", i, data);
#endif

        if (esp->rx_index < ESP8266_MAX_BUFFER_SIZE - 1)
        {
            esp->rx_buffer[esp->rx_index++] = data;

            // 检测到结束符（例如 '\n'）
            if (data == '\n')
            {
#if DEBUG_ESP8266_IRQ
                // 调试信息：检测到结束符
                printf("[ESP8266_UART_IRQHandler] End of line detected\r\n");
#endif

                esp->rx_buffer[esp->rx_index] = '\0';
                esp->response_received = 1; // 标记响应接收完成

#if DEBUG_ESP8266_IRQ
                // 调试信息：显示接收到的完整数据
                printf("[ESP8266_UART_IRQHandler] Received data: %s\r\n", esp->rx_buffer);
#endif

                if (esp->mqtt_status == MQTT_STATUS_CONNECTED)
                {
#if DEBUG_ESP8266_IRQ
                    // 调试信息：MQTT 已连接，开始处理数据
                    printf("[ESP8266_UART_IRQHandler] MQTT connected, processing data\r\n");
#endif
                    ESP8266_ProcessReceivedData(esp);
                }

                break;
            }
        }
        else
        {
#if DEBUG_ESP8266_IRQ
            // 缓冲区溢出处理
            printf("[ESP8266_UART_IRQHandler] ERROR: rx_buffer overflow! Buffer size: %d, Current index: %d\r\n",
                   ESP8266_MAX_BUFFER_SIZE, esp->rx_index);
#endif
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
    esp->response_received = 0;
    memset(esp->response_buffer, 0, ESP8266_MAX_RESPONSE_SIZE);
    esp->rx_index = 0;

    // 发送命令（添加回车换行）
    char full_cmd[ESP8266_MAX_CMD_SIZE];
    printf("[esp8266send cmd:");
    HAL_UART_Transmit(&huart2, (uint8_t *)full_cmd, strlen(full_cmd), timeout_ms);
    printf("]\r\n");

    snprintf(full_cmd, sizeof(full_cmd), "%s\r\n", cmd);
    HAL_UART_Transmit(esp->huart, (uint8_t *)full_cmd, strlen(full_cmd), 2000);

    HAL_Delay(1000);

    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms)
    {
        if (esp->response_received)
        {
            printf("[SendCommand]:recv:%s\r\n", esp->rx_buffer);
            // 检查是否包含预期响应
            if (strstr((const char *)esp->rx_buffer, expected_response) != NULL)
            {
                // 清空缓冲区
                esp->rx_index = 0;
                memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
                return ESP8266_OK;
            }
            // 清空缓冲区
            esp->rx_index = 0;
            memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
            return ESP8266_ERROR_RESPONSE;
        }
        // 可在此处添加任务切换或低功耗代码
    }
    // 清空缓冲区
    esp->rx_index = 0;
    memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
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

// 查询WiFi连接状态
// 当 ESP station 没有连接上 AP 时，推荐使用此命令查询 Wi-Fi 信息;
// 当 ESP station 已连接上 AP 后，推荐使用 AT+CWJAP 命令查询 Wi-Fi 信息;
int ESP8266_QueryWiFiStatus_CWSTATE(ESP8266_HandleTypeDef *esp, uint32_t timeout)
{
    uint8_t status;
    char command[256];
    snprintf(command, sizeof(command), "AT+CWSTATE?");
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
int ESP8266_QueryWiFiStatus_CWJAP(ESP8266_HandleTypeDef *esp, uint32_t timeout){
    char command[256];
    snprintf(command, sizeof(command), "AT+CWJAP?");
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
    snprintf(command, sizeof(command), "AT+MQTTUSERCFG=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"", link_id, scheme, client_id, username, password, cert_key_ID, CA_ID, path);
    // 发送命令并等待响应
    return ESP8266_SendCommand(esp, command, "OK", timeout);
}

// 连接到 MQTT Broker
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

// 发送已经存在的json数据
int ESP8266_SendJSON(ESP8266_HandleTypeDef *esp, const char *topic, const char *json_data, uint32_t timeout)
{

    char json_str[128];
    // 添加 AT 命令
    // AT+MQTTPUB=0,"topic","\"{\"timestamp\":\"20201121085253\"}\"",0,0  // 发送此命令时，请注意特殊字符是否需要转义。
    sprintf(json_str, "AT+MQTTPUB=0,\"%s\",\"%s\",0,0", topic, json_data);

    return ESP8266_SendCommand(esp, json_str, "OK", timeout);
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

    // 复制响应数据到缓冲区
    strncpy(esp->response_buffer, esp->rx_buffer, esp->rx_index);
    esp->response_buffer[esp->rx_index] = '\0';
    printf("[ESP8266] Response: %s\r\n", esp->response_buffer);

    // 处理充值请求
    if (strstr(esp->response_buffer, "recharge"))
    {
        uint8_t user_card_id[5] = {0};
        uint32_t amount = 0;
        char json_buffer[256] = {0};

        // 提取JSON数据
        if (sscanf(esp->response_buffer, "+MQTTSUBRECV:%*d,\"%*[^\"]\",%*d,%512[^\n]", json_buffer) == 1)
        {
            // 解析JSON格式的充值请求
            if (sscanf(json_buffer, "{\"operation\":\"recharge\",\"user_card_id\":\"%02hhX%02hhX%02hhX%02hhX%02hhX\",\"amount\":\"%lu\"}",
                       &user_card_id[0], &user_card_id[1], &user_card_id[2],
                       &user_card_id[3], &user_card_id[4], &amount) == 6)
            {
                // 保存充值信息并设置状态
                memcpy(RechargTempUser.user_card_id, user_card_id, sizeof(user_card_id));
                RechargTempUser.user_balance = amount;
                rechargeState = RECHARGE_STATE_WAITING;
                printf("[ESP8266] Recharge processed: ID=%02X%02X%02X%02X%02X, Amount=%lu\r\n",
                       user_card_id[0], user_card_id[1], user_card_id[2],
                       user_card_id[3], user_card_id[4], amount);
            }
            else
            {
                printf("[ESP8266] Parse failed: %s\r\n", json_buffer);
            }
        }
        else
        {
            printf("[ESP8266] JSON extract failed: %s\r\n", esp->response_buffer);
        }
    }

    // 清空缓冲区，准备接收新数据
    esp->rx_index = 0;
    esp->response_received = 0;
    memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
    memset(esp->response_buffer, 0, ESP8266_MAX_RESPONSE_SIZE);
}

#define DEBUG_MQTT_CONNECT 0 // 控制调试信息输出，1为启用，0为禁用

void MQTT_Connect(ESP8266_HandleTypeDef *data)
{
    int result;

    // 发送AT指令
    while ((result = ESP8266_SendCommand(data, "AT", "OK", TIMEOUT)) != ESP8266_OK)
    {
#if DEBUG_MQTT_CONNECT
        printf("AT failed: %d\r\n", result);
#endif
        HAL_Delay(500);
    }
#if DEBUG_MQTT_CONNECT
    printf("AT success: %d\r\n", result);
#endif
    HAL_Delay(500);

    while ((result = HAL_UART_Transmit(data->huart, (uint8_t *)"ATE\r\n", sizeof("ATE\r\n") - 1, 10)) != HAL_OK)
    {
#if DEBUG_MQTT_CONNECT
        printf("ATE failed: %d\r\n", result);
#endif
        HAL_Delay(500);
    }
#if DEBUG_MQTT_CONNECT
    printf("ATE success: %d\r\n", result);
#endif
    HAL_Delay(500);

    // 设置WiFi模式
    while ((result = ESP8266_SendCommand(data, "AT+CWMODE=1", "OK", TIMEOUT)) != ESP8266_OK)
    {
#if DEBUG_MQTT_CONNECT
        printf("WiFi mode failed: %d\r\n", result);
#endif
        HAL_Delay(500);
    }
#if DEBUG_MQTT_CONNECT
    printf("WiFi mode success: %d\r\n", result);
#endif
    HAL_Delay(500);

    // 启用DHCP
    while ((result = ESP8266_SendCommand(data, "AT+CWDHCP=1,1", "OK", TIMEOUT)) != ESP8266_OK)
    {
#if DEBUG_MQTT_CONNECT
        printf("DHCP failed: %d\r\n", result);
#endif
        HAL_Delay(500);
    }
#if DEBUG_MQTT_CONNECT
    printf("DHCP success: %d\r\n", result);
#endif
    HAL_Delay(500);

    // 连接WiFi
    while ((result = ESP8266_SendCommand(data, "AT+CWJAP=\"1\",\"88888888\"", "OK", 10000)) != ESP8266_OK)
    {
#if DEBUG_MQTT_CONNECT
        printf("WiFi connection failed: %d\r\n", result);
#endif
        HAL_Delay(500);
    }
#if DEBUG_MQTT_CONNECT
    printf("WiFi connection success: %d\r\n", result);
#endif
    HAL_Delay(500);

    // 设置MQTT
    result = ESP8266_SetMQTTConfig(data, 0, 1, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, 0, 0, "", TIMEOUT);
#if DEBUG_MQTT_CONNECT
    if (result != ESP8266_OK)
    {
        printf("MQTT configuration failed: %d\r\n", result);
    }
    printf("MQTT configuration success: %d\r\n", result);
#endif

    // 连接MQTT
    result = ESP8266_ConnectMQTT(data, 0, MQTT_IP, MQTT_PORT, 1, 2000);
#if DEBUG_MQTT_CONNECT
    if (result == 1)
    {
        printf("MQTT connection success\r\n");
    }
    else
    {
        printf("MQTT connection failed\r\n");
    }
#endif
    HAL_Delay(500);

    // 订阅MQTT
    ESP8266_SubscribeMQTT(data, 0, MQTT_TOPIC, MQTT_QoS0, TIMEOUT);
#if DEBUG_MQTT_CONNECT
    printf("MQTT subscription success: %d\r\n", result);
#endif
    HAL_Delay(500);

    data->mqtt_status = MQTT_STATUS_CONNECTED;

    // 显示成功
    OLED_PrintString(32, 16, "MQTT OK", &font16x8, OLED_COLOR_NORMAL);
    HAL_Delay(1000);
    OLED_ShowFrame();
    UIManager_SwitchScreen(SCREEN_MAIN);
    return;
}

#include <stdarg.h>

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

