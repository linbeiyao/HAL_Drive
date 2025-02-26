#include "esp8266.h"
#include "button.h"
#include "oled.h"
#include "usart.h"
#include "string.h"

#include <stdarg.h>


// 定义需要订阅的主题数组
const char* topics[] = {        
    MQTT_TOPIC_PROPERTY_POST_REPLY,
    MQTT_TOPIC_PROPERTY_SET,
    MQTT_TOPIC_PROPERTY_GET_REPLY,
};
    

/* 全局变量 */
uint8_t g_wifi_status_need_parse_CWSTATE = 0;
uint8_t g_wifi_status_need_parse_CWJAP = 0;
uint8_t g_mqtt_status_need_parse_MQTTCONN = 0;

/* DMA相关变量 */
#define DMA_BUFFER_SIZE 512
uint8_t dma_rx_buffer[DMA_BUFFER_SIZE];  // DMA接收缓冲区
volatile uint16_t dma_rx_index = 0;      // 当前接收索引
volatile uint8_t dma_rx_complete = 0;    // 接收完成标志

/* 静态变量 */
static char expected_response[128] = {0};  // 期望的响应字符串

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

    // 注意：不在此处启用DMA接收，将在初始化完成后启用
}


void ESP8266_UART_IRQHandler(ESP8266_HandleTypeDef *esp)
{
    // DMA 已将数据放入 rx_buffer，rx_index 表示接收到的数据长度
    if (esp->rx_index > 0) {
        esp->rx_buffer[esp->rx_index] = '\0'; // 确保字符串终止
        esp->response_received = 1;           // 标记数据接收完成
        ESP8266_ProcessReceivedData(esp);     // 立即处理
    }
}

// 发送 AT 指令并等待期望的响应
int ESP8266_SendCommand(ESP8266_HandleTypeDef *esp, const char *cmd, const char *expected_response_param, uint32_t timeout_ms)
{
    // 初始化操作
    esp->expected_response_received = 0; // 重置预期响应标志
    esp->rx_index = 0;
    memset(expected_response, 0, sizeof(expected_response)); // 清空预期响应缓冲区
    strncpy(expected_response, expected_response_param, sizeof(expected_response)-1);
    expected_response[sizeof(expected_response)-1] = '\0';

    // 构建并发送AT命令
    char full_cmd[ESP8266_MAX_CMD_SIZE];
    memset(full_cmd, 0, sizeof(full_cmd)); // 清空命令缓冲区
    int cmd_len = snprintf(full_cmd, sizeof(full_cmd), "%s\r\n", cmd);

    HAL_StatusTypeDef uart_status = HAL_UART_Transmit(esp->huart, (uint8_t *)full_cmd, cmd_len, timeout_ms);
    if (uart_status != HAL_OK)
    {
        printf("[ESP8266] UART传输错误: %d\r\n", uart_status);
        switch(uart_status) {
            case HAL_ERROR:
                printf("[ESP8266] HAL错误,尝试重新初始化UART...\r\n");
                break;
            case HAL_BUSY:
                printf("[ESP8266] UART忙,等待释放...\r\n");
                HAL_Delay(100);  // 等待100ms
                break;
            case HAL_TIMEOUT:
                printf("[ESP8266] UART超时,增加重试次数\r\n");
                break;
            default:
                printf("[ESP8266] 未知错误\r\n");
                break;
        }
        memset(expected_response, 0, sizeof(expected_response)); // 清空预期响应
        esp->rx_index = 0;  // 重置接收索引
        return ESP8266_ERROR;
    }

    // 等待响应或超时
    uint32_t start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < timeout_ms)
    {
        if (esp->expected_response_received) // 使用新的标志位
        {
            memset(expected_response, 0, sizeof(expected_response)); // 清空预期响应
            return ESP8266_OK;
        }
        HAL_Delay(1);
    }

    // 超时处理
    printf("[ESP8266] Command timeout: %s\r\n", cmd);
    memset(expected_response, 0, sizeof(expected_response)); // 清空预期响应
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
    snprintf(command, sizeof(command), "AT+MQTTUSERCFG=%d,%d,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"\r\n", link_id, scheme, client_id, username, password, cert_key_ID, CA_ID, path);
		
		//snprintf(command,sizeof(command),"AT+MQTTUSERCFG=0,1,\"SoilMoisture_System\",\"X15T01ZVAx\",\"version=2018-10-31&res=products%%2FX15T01ZVAx%%2Fdevices%%2FSoilMoisture_System&et=1988121600&method=sha256&sign=IS8MliNutFuKugawuj7bnaHrCFrmLAgcmbXs1dIjazA%3D\",0,0,\"\"");
		
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
    //
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
    char command[1024];
    snprintf(command, sizeof(command), "AT+MQTTCONN=%d,\"%s\",%d,%d", link_id, broker_ip, port, reconnect);
		//snprintf(command, sizeof(command), "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1");
	

    int ret = ESP8266_SendCommand(esp, command, "OK", timeout);
    if (ret == ESP8266_OK)
    {
        esp->mqtt_status = MQTT_STATUS_CONNECTED;
        return ESP8266_OK; // 连接成功
    }
    else
    {
        esp->mqtt_status = MQTT_STATUS_DISCONNECTED;
        return ESP8266_ERROR; // 连接失败
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



#define DEBUG_MQTT_CONNECT 0 // 控制调试信息输出，1为启用，0为禁用

void MQTT_Connect(ESP8266_HandleTypeDef *data)
{
    const char *commands[] = {
        "AT\r\n", "ATE0\r\n", "AT+CWMODE=1\r\n", "AT+CWDHCP=1,1\r\n", 
        "AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASSWORD "\"\r\n"
    };
    const char *responses[] = {
        "OK", "OK", "OK", "OK", "OK"
    };
    const uint32_t timeouts[] = {
        3000, 1000, TIMEOUT, TIMEOUT, 10000
    };
    const uint32_t delays[] = {
        200, 200, 200, 200, 200
    };
    
    char rx_buffer[256] = {0};
    uint32_t start_time;
    uint8_t response_found;
    uint16_t length;
    
    printf("开始ESP8266初始化\r\n");
    
    // 执行初始化命令序列（前4个命令）
    for (int i = 0; i < 4; i++) {
        printf("执行命令: %s", commands[i]);
        
        // 多次尝试直到命令成功
        response_found = 0;
        while (!response_found) {
            // 发送命令
            HAL_UART_Transmit(data->huart, (uint8_t*)commands[i], strlen(commands[i]), 1000);
            
            // 等待响应
            memset(rx_buffer, 0, sizeof(rx_buffer));
            start_time = HAL_GetTick();
            
            while ((HAL_GetTick() - start_time) < timeouts[i]) {
                // 轮询接收数据
                if (HAL_UART_Receive(data->huart, (uint8_t*)&rx_buffer[strlen(rx_buffer)], 1, 100) == HAL_OK) {
                    // 检查是否包含期望的响应
                    if (strstr(rx_buffer, responses[i]) != NULL) {
                        printf("收到响应: %s\r\n", responses[i]);
                        response_found = 1;
                        break;
                    }
                }
                HAL_Delay(1);
            }
            
            if (!response_found) {
                printf("命令超时，重试...\r\n");
                HAL_Delay(delays[i]);
            }
        }
        
        HAL_Delay(delays[i]);
    }
    
    // 发送WiFi状态查询命令
    printf("查询WiFi状态\r\n");
    HAL_UART_Transmit(data->huart, (uint8_t*)"AT+CWSTATE?\r\n", 13, 1000);
    HAL_Delay(300);
    
    // 处理WIFI连接命令
    if (data->init_wifi_status != WIFI_STATUS_CONNECTED) {
        printf("WiFi未连接，尝试连接...\r\n");
        response_found = 0;
        
        while (!response_found) {
            // 发送连接WiFi命令
            HAL_UART_Transmit(data->huart, (uint8_t*)commands[4], strlen(commands[4]), 1000);
            
            // 等待响应
            memset(rx_buffer, 0, sizeof(rx_buffer));
            start_time = HAL_GetTick();
            
            while ((HAL_GetTick() - start_time) < timeouts[4]) {
                // 轮询接收数据
                if (HAL_UART_Receive(data->huart, (uint8_t*)&rx_buffer[strlen(rx_buffer)], 1, 100) == HAL_OK) {
                    // 检查是否包含WIFI连接相关响应
                    if (strstr(rx_buffer, "WIFI CONNECTED") != NULL && 
                        strstr(rx_buffer, "WIFI GOT IP") != NULL && 
                        strstr(rx_buffer, "OK") != NULL) {
                        printf("WiFi连接成功\r\n");
                        data->init_wifi_status = WIFI_STATUS_CONNECTED;
                        response_found = 1;
                        break;
                    }
                }
                HAL_Delay(1);
            }
            
            if (!response_found && strstr(rx_buffer, "WIFI CONNECTED") != NULL) {
                // 如果已经连接但没收到完整响应，也认为成功
                printf("WiFi部分连接成功\r\n");
                data->init_wifi_status = WIFI_STATUS_CONNECTED;
                break;
            }
            
            if (!response_found) {
                printf("WiFi连接超时，重试...\r\n");
                HAL_Delay(delays[4]);
                
                // 再次查询WiFi状态
                HAL_UART_Transmit(data->huart, (uint8_t*)"AT+CWSTATE?\r\n", 13, 1000);
                HAL_Delay(300);
                
                if (data->init_wifi_status == WIFI_STATUS_CONNECTED) {
                    break;
                }
            }
        }
    } else {
        printf("WiFi已连接\r\n");
    }
    
    HAL_Delay(5000); // 等待WiFi稳定
    
    // 配置MQTT用户
    printf("配置MQTT用户\r\n");
    char mqtt_user_cmd[] = "AT+MQTTUSERCFG=0,1,\"SoilMoisture_System\",\"X15T01ZVAx\",\"version=2018-10-31&res=products%%2FX15T01ZVAx%%2Fdevices%%2FSoilMoisture_System&et=1988121600&method=sha256&sign=IS8MliNutFuKugawuj7bnaHrCFrmLAgcmbXs1dIjazA%%3D\",0,0,\"\"\r\n";
    
    response_found = 0;
    while (!response_found) {
        HAL_UART_Transmit(data->huart, (uint8_t*)mqtt_user_cmd, strlen(mqtt_user_cmd), 1000);
        
        // 等待响应
        memset(rx_buffer, 0, sizeof(rx_buffer));
        start_time = HAL_GetTick();
        
        while ((HAL_GetTick() - start_time) < TIMEOUT) {
            if (HAL_UART_Receive(data->huart, (uint8_t*)&rx_buffer[strlen(rx_buffer)], 1, 100) == HAL_OK) {
                if (strstr(rx_buffer, "OK") != NULL) {
                    printf("MQTT用户配置成功\r\n");
                    response_found = 1;
                    break;
                }
            }
            HAL_Delay(1);
        }
        
        if (!response_found) {
            printf("MQTT用户配置超时，重试...\r\n");
        }
    }
    
#if (SECELTMQTTPLATFORM == NOMAL)
    // 设置MQTT遗嘱
    printf("设置MQTT遗嘱\r\n");
    char mqtt_will_cmd[256];
    snprintf(mqtt_will_cmd, sizeof(mqtt_will_cmd), "AT+MQTTCONNCFG=0,120,0,\"%s\",\"%s\",0,1\r\n", 
            MQTT_TOPIC_Will, MQTT_TOPIC_Will_Message);
    
    response_found = 0;
    while (!response_found) {
        HAL_UART_Transmit(data->huart, (uint8_t*)mqtt_will_cmd, strlen(mqtt_will_cmd), 1000);
        
        // 等待响应
        memset(rx_buffer, 0, sizeof(rx_buffer));
        start_time = HAL_GetTick();
        
        while ((HAL_GetTick() - start_time) < TIMEOUT) {
            if (HAL_UART_Receive(data->huart, (uint8_t*)&rx_buffer[strlen(rx_buffer)], 1, 100) == HAL_OK) {
                if (strstr(rx_buffer, "OK") != NULL) {
                    printf("MQTT遗嘱设置成功\r\n");
                    response_found = 1;
                    break;
                }
            }
            HAL_Delay(1);
        }
        
        if (!response_found) {
            printf("MQTT遗嘱设置超时，重试...\r\n");
            HAL_Delay(1000);
        }
    }
#endif
    
    // 连接MQTT服务器
    printf("连接MQTT服务器\r\n");
    char mqtt_conn_cmd[] = "AT+MQTTCONN=0,\"47.104.253.100\",1883,1\r\n";
    
    response_found = 0;
    while (!response_found) {
        HAL_UART_Transmit(data->huart, (uint8_t*)mqtt_conn_cmd, strlen(mqtt_conn_cmd), 1000);
        
        // 等待响应
        memset(rx_buffer, 0, sizeof(rx_buffer));
        start_time = HAL_GetTick();
        
        while ((HAL_GetTick() - start_time) < 5000) {
            if (HAL_UART_Receive(data->huart, (uint8_t*)&rx_buffer[strlen(rx_buffer)], 1, 100) == HAL_OK) {
                if (strstr(rx_buffer, "OK") != NULL) {
                    printf("MQTT服务器连接成功\r\n");
                    response_found = 1;
                    data->mqtt_status = MQTT_STATUS_CONNECTED;
                    break;
                }
            }
            HAL_Delay(1);
        }
        
        if (!response_found) {
            printf("MQTT服务器连接超时，重试...\r\n");
            HAL_Delay(1000);
        }
    }
    
    // 订阅所有主题
    printf("订阅MQTT主题\r\n");
    for (int i = 0; i < sizeof(topics)/sizeof(topics[0]); i++) {
        char mqtt_sub_cmd[256];
        snprintf(mqtt_sub_cmd, sizeof(mqtt_sub_cmd), "AT+MQTTSUB=0,\"%s\",0\r\n", topics[i]);
        
        response_found = 0;
        while (!response_found) {
            HAL_UART_Transmit(data->huart, (uint8_t*)mqtt_sub_cmd, strlen(mqtt_sub_cmd), 1000);
            
            // 等待响应
            memset(rx_buffer, 0, sizeof(rx_buffer));
            start_time = HAL_GetTick();
            
            while ((HAL_GetTick() - start_time) < TIMEOUT) {
                if (HAL_UART_Receive(data->huart, (uint8_t*)&rx_buffer[strlen(rx_buffer)], 1, 100) == HAL_OK) {
                    if (strstr(rx_buffer, "OK") != NULL) {
                        printf("MQTT主题\"%s\"订阅成功\r\n", topics[i]);
                        response_found = 1;
                        break;
                    }
                }
                HAL_Delay(1);
            }
            
            if (!response_found) {
                printf("MQTT主题订阅超时，重试...\r\n");
                HAL_Delay(1000);
            }
        }
        
        HAL_Delay(500); // 订阅间隔延时
    }
    
    // 启动DMA接收
    printf("初始化完成，启动DMA接收\r\n");
    HAL_UART_Receive_DMA(data->huart, dma_rx_buffer, DMA_BUFFER_SIZE);
    
    data->mqtt_status = MQTT_STATUS_CONNECTED;
    UIManager_SwitchScreen(SCREEN_MAIN);
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
		char line_buffer[512] = {0};
		
		printf("[ESP8266] Processing data: %s\r\n", current_ptr);
		
		// char json_buffer[256] = {0};

    while (*current_ptr)
    {
			// 查找下一行结束位置
			char *line_end = strstr(current_ptr,"\r\n");
			if (!line_end) {
				// 如果没有找到换行符，可能是数据不完整，保留剩余数据
				size_t remaining_len = strlen(current_ptr);
				if(remaining_len > 0){
						memmove(esp->rx_buffer, current_ptr ,remaining_len + 1);
						esp->rx_index  = remaining_len;
				} else {
						esp->rx_index = 0;
				}
				break;
			}
			
			// 提取一行数据
			size_t line_len = line_end - current_ptr;
			if(line_len > sizeof(line_buffer)){
				printf("[ESP8266] Line too long, truncated\r\n");
				line_len = sizeof(line_buffer) - 1;
			}
			strncpy(line_buffer, current_ptr, line_len);
      line_buffer[line_len] = '\0';
			
			// 处理当前行
      printf("[ESP8266] Processing line: %s\r\n", line_buffer);
			
			// 检查 WiFi 状态
        if (strstr(line_buffer, "WIFI CONNECTED")) {
            esp->init_wifi_status = WIFI_STATUS_CONNECTED;
        } else if (strstr(line_buffer, "WIFI DISCONNECTED")) {
            esp->init_wifi_status = WIFI_STATUS_DISCONNECTED;
        }
			
			
        printf("[ESP8266] Processing packet: %s\r\n", current_ptr);

        // 处理 WiFi 连接状态
        if (strstr(current_ptr, "WIFI CONNECTED"))
            esp->init_wifi_status = WIFI_STATUS_CONNECTED;
				
				
				// 处理 MQTT 消息
        if (esp->mqtt_status == MQTT_STATUS_CONNECTED) {
            int link_id, data_length;
            char topic[ESP8266_MAX_TOPIC_SIZE] = {0};
            char message[ESP8266_MAX_MESSAGE_SIZE] = {0};

            if (sscanf(line_buffer, "+MQTTSUBRECV:%d,\"%127[^\"]\",%d,%511[^\r\n]",
                       &link_id, topic, &data_length, message) == 4) {
                printf("[ESP8266] MQTT message - Topic: %s, Message: %s\r\n", topic, message);
//                if (esp->mqtt_message_callback) {
//                    esp->mqtt_message_callback(topic, message);
//                }
												 
								// 根据主题进行处理
								if (strcmp(topic, "/thing/property/set") == 0) {      
										// 属性设置处理
										// 解析属性设置消息
										printf("收到属性设置消息: %s\r\n", message);
										
										// 解析JSON格式
										if(strstr(message, "mode") != NULL){
												printf("收到模式设置消息: %s\r\n", message); // {"mode": "1"}  {"mode": "0"}
												// 解析模式设置消息
												char *mode_start = strstr(message, "\"mode\"");
												if(mode_start){
														int8_t json_mode = -1;
														if(sscanf(mode_start, "\"mode\":\"%d\"", &json_mode) == 1){
																printf("解析到模式值: %d\r\n", json_mode);
																System_SetMode(json_mode);          // 设置模式
														}
												}
										}
									}

									if(strstr(message, "rain_one_day") != NULL){
											printf("收到降雨概率设置消息: %s\r\n", message); // {"rain_one_day": "10"}  {"rain_one_day": "0"}
											// 解析降雨概率设置消息
											char *rain_one_day_start = strstr(message, "\"rain_one_day\"");
											if(rain_one_day_start){
													int8_t json_rain_one_day = -1;
													if(sscanf(rain_one_day_start, "\"rain_one_day\":\"%d\"", &json_rain_one_day) == 1){
															printf("解析到降雨概率值: %d\r\n", json_rain_one_day);
															System_SetRainProbability(json_rain_one_day, 0);          // 设置降雨概率
													}
											}
									}

									if(strstr(message, "rain_two_day") != NULL){
											printf("收到降雨概率设置消息: %s\r\n", message); // {"rain_two_day": "10"}  {"rain_two_day": "0"}
											// 解析降雨概率设置消息
											char *rain_two_day_start = strstr(message, "\"rain_two_day\"");
											if(rain_two_day_start){
													int8_t json_rain_two_day = -1;  
													if(sscanf(rain_two_day_start, "\"rain_two_day\":\"%d\"", &json_rain_two_day) == 1){
															printf("解析到降雨概率值: %d\r\n", json_rain_two_day);
															System_SetRainProbability(0, json_rain_two_day);          // 设置降雨概率
													}
											}
									}
									

									else if (strcmp(topic, "/thing/property/get_reply") == 0) {      
											// 属性查询响应处理
									} else if (strcmp(topic, "/thing/property/post_reply") == 0) {
											// 设备属性上报响应处理

									}
												 
												 
												 
            }
            // 处理其他 MQTT 状态消息
            else if (strstr(line_buffer, "+MQTTCONNECTED")) {
                esp->mqtt_status = MQTT_STATUS_CONNECTED;
            } else if (strstr(line_buffer, "+MQTTDISCONNECTED")) {
                esp->mqtt_status = MQTT_STATUS_DISCONNECTED;
            }
        }
				
				
       

				// 检查预期响应
        if (strstr(line_buffer, expected_response)) {
            esp->expected_response_received = 1;
            printf("[ESP8266] Expected response received: %s\r\n", expected_response);
        }

				current_ptr = line_end + 2; // 跳过 "\r\n"
    }

		// 处理完毕后清空缓冲区或保留未处理数据
    if (current_ptr >= (char *)esp->rx_buffer + esp->rx_index) {
        esp->rx_index = 0;
        memset(esp->rx_buffer, 0, ESP8266_MAX_BUFFER_SIZE);
    }
		
		esp->response_received = 0; // 重置标志
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
 *                 "{\\\"key1\\\":\\\"%d\\\"\\,\\\"key2\\\":\\\"%s\\\"}"
 * @param json_str 存放生成 JSON 字符串的缓冲区指针
 * @param ...      格式字符串对应的参数列表
 */
void JSON_Template_Pack	(const char *fmt, char *json_str, ...)
{
    va_list args;
    va_start(args, json_str);
    vsprintf(json_str, fmt, args);
    va_end(args);
}

