#include "esp8266.h"




/**
 * @brief 回调函数
 * 
 * @param message 
 * @return 
 */
uint8_t ESP8266_CallBack(ESP8266_HandleTypeDef *esp,const char *message){
    if (g_wifi_status_need_parse_CWSTATE)
    {
        ESP8266_QueryWiFiStatus_CWSTATE_Callback(esp,message);
    }
    if (g_wifi_status_need_parse_CWJAP)
    {
        ESP8266_QueryWiFiStatus_CWJAP_Callback(esp,message);
    }
    if (g_mqtt_status_need_parse_MQTTCONN)
    {
        ESP8266_QueryMQTTStatus_Connect_Callback(esp,message);
    }
    
		if(!g_wifi_status_need_parse_CWSTATE && !g_wifi_status_need_parse_CWJAP && !g_mqtt_status_need_parse_MQTTCONN)
    {
        return 0;
    }
}





/**
 * @brief 查询WiFi连接状态回调函数
 * 
 * 解析AT+CWSTATE?命令的响应，格式如下：
 * +CWSTATE:<state>,<"ssid">
 * OK
 * 
 * @param response AT命令的响应字符串
 * 
 * <state>：当前WiFi连接状态
 *   0: 未连接
 *   1: 已连接AP但未获取IP
 *   2: 已连接AP并获取IP
 *   3: 正在连接或重连
 *   4: 已断开连接
 * 
 * <"ssid">：目标AP的SSID名称
 */
uint8_t ESP8266_QueryWiFiStatus_CWSTATE_Callback(ESP8266_HandleTypeDef *esp,const char *response)
{
    // 解析响应字符串
    char *state_str = strstr(response, "+CWSTATE:");
    if (state_str == NULL)
    {
        return 0;
    }
    else
    {
        // 解析状态和SSID
        int state = 0;
        char ssid[32] = {0};
        
        // 使用sscanf解析状态和SSID
        if (sscanf(state_str, "+CWSTATE:%d,\"%[^\"]\"", &state, ssid) == 2)
        {
            // 根据状态进行相应处理 自行进行修改
            switch (state)
            {
            case 0:
                // printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:no connect\n");
                g_wifi_status_need_parse_CWSTATE =  0;

                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
                esp->mqtt_status = MQTT_STATUS_DISCONNECTED;

                break;
            case 1:
                // printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:connect AP but no get IP, SSID: %s\n", ssid);
                g_wifi_status_need_parse_CWSTATE =  0;

                break;
            case 2:
                // printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:connect AP and get IP, SSID: %s\n", ssid);
                g_wifi_status_need_parse_CWSTATE =  0;

                break;
            case 3:
                // printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:connecting or reconnecting, SSID: %s\n", ssid);
                g_wifi_status_need_parse_CWSTATE =  0;

                break;
            case 4:
                // printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:disconnect\n");
                g_wifi_status_need_parse_CWSTATE =  0;

                break;
            default:
                // printf("[ESP8266_QueryWiFiStatus_Callback]  Unknown WiFi Status: %d\n", state);
                g_wifi_status_need_parse_CWSTATE =  0;

                break;
            }

            if(state == 2) // 连接成功
            {
                ESP8266_HandleTypeDef *esp = (ESP8266_HandleTypeDef*) get_esp_handle();
                esp->init_wifi_status = WIFI_STATUS_CONNECTED;
            }
            else
            {
                ESP8266_HandleTypeDef *esp = (ESP8266_HandleTypeDef*) get_esp_handle();
                esp->init_wifi_status = WIFI_STATUS_DISCONNECTED;
            }
            return 1;
        }
        else
        {
            // printf("[ESP8266_QueryWiFiStatus_Callback]  Parse WiFi Status failed\n");
            g_wifi_status_need_parse_CWSTATE = 0;

            return 0;
        }
    }
}

/**
 * @brief 查询与ESP Station连接的AP信息
 * 
 * 命令格式：AT+CWJAP?
 * 响应格式：+CWJAP:<ssid>,<bssid>,<channel>,<rssi>,<pci_en>,<reconn_interval>,<listen_interval>,<scan_mode>,<pmf>
 * 
 * 参数说明：
 * - ssid: 目标AP的SSID（需转义特殊字符,、"、\\）
 * - bssid: 目标AP的MAC地址（当多个AP有相同SSID时必填）
 * - channel: 信道号
 * - rssi: 信号强度
 * - pci_en: PCI认证模式
 *   - 0: 支持所有加密方式（包括OPEN和WEP）
 *   - 1: 仅支持WPA/WPA2加密
 * - reconn_interval: Wi-Fi重连间隔（单位：秒，默认1，范围[0,7200]）
 *   - 0: 不自动重连
 *   - [1,7200]: 定时重连间隔
 * - listen_interval: 监听AP beacon间隔（单位：AP beacon间隔，默认3，范围[1,100]）
 * - scan_mode: 扫描模式
 *   - 0: 快速扫描（连接第一个匹配AP）
 *   - 1: 全信道扫描（连接信号最强AP）
 * - pmf: 受保护的管理帧（PMF）配置
 *   - bit 0: 支持PMF（优先PMF模式连接）
 *   - bit 1: 需要PMF（拒绝不支持PMF的设备）
 * 
 * 错误码说明：
 * 1: 连接超时
 * 2: 密码错误
 * 3: 无法找到目标AP
 * 4: 连接失败
 * 其他: 未知错误
 */
uint8_t ESP8266_QueryWiFiStatus_CWJAP_Callback(ESP8266_HandleTypeDef *esp,const char *response){
    // 解析响应字符串
    char *state_str = strstr(response, "+CWJAP:");
    if (state_str == NULL)
    {
        return 0;
    }

		// 解析参数
		char ssid[32] = {0};
		char bssid[18] = {0};
		int channel = 0;
		int rssi = 0;
		int pci_en = 0;
		uint16_t reconn_interval = 0;
		uint8_t listen_interval = 0;
		uint8_t scan_mode = 0;
		uint8_t pmf = 0;

		// 使用sscanf解析参数，调整格式字符串以匹配变量类型
		if (sscanf(state_str, "+CWJAP:%[^,],%[^,],%d,%d,%d,%hu,%hhu,%hhu,%hhu", 
							 ssid, bssid, &channel, &rssi, &pci_en, &reconn_interval, 
							 &listen_interval, &scan_mode, &pmf) == 9)
		{
				// printf("[ESP8266_QueryWiFiStatus_CWJAP_Callback]  WiFi Status: connect AP and get IP, SSID: %s\n", ssid);
				g_wifi_status_need_parse_CWJAP = 0;
				return 1;  // 成功解析返回1
		}
		else
		{
				// printf("[ESP8266_QueryWiFiStatus_CWJAP_Callback]  Parse WiFi Status failed\n");
				g_wifi_status_need_parse_CWJAP = 0;
				return 0;  // 解析失败返回0
		}
}



/**
 * AT+MQTTCONN MQTT连接状态查询
 * 命令：AT+MQTTCONN?
 * 响应：+MQTTCONN:<LinkID>,<state>,<scheme><"host">,<port>,<"path">,<reconnect>
 * 
 * 参数说明：
 * <LinkID>：当前仅支持link ID 0
 * <host>：MQTT broker域名，最大128字节
 * <port>：MQTT broker端口，最大65535
 * <path>：资源路径，最大32字节
 * <reconnect>：0-不自动重连，1-自动重连
 * <state>：MQTT状态(0-未初始化,1-已设置USERCFG,2-已设置CONNCFG,
 *          3-已断开,4-已连接,5-已连接未订阅,6-已连接已订阅)
 * <scheme>：连接方案(1-MQTT over TCP,2-MQTT over TLS无校验,
 *          3-MQTT over TLS校验server,4-MQTT over TLS提供client,
 *          5-MQTT over TLS双向校验,6-MQTT over WebSocket TCP,
 *          7-MQTT over WebSocket TLS无校验,8-MQTT over WebSocket TLS校验server,
 *          9-MQTT over WebSocket TLS提供client,10-MQTT over WebSocket TLS双向校验)
 */

uint8_t ESP8266_QueryMQTTStatus_Connect_Callback(ESP8266_HandleTypeDef *esp,const char *response){
    // 检查响应字符串是否包含 "+MQTTCONN:"
    int i;
    for(i = 0; response[i] != '\0'; i++) {
        if(response[i] == '+' && 
           response[i+1] == 'M' &&
           response[i+2] == 'Q' &&
           response[i+3] == 'T' &&
           response[i+4] == 'T' &&
           response[i+5] == 'C' &&
           response[i+6] == 'O' &&
           response[i+7] == 'N' &&
           response[i+8] == 'N' &&
           response[i+9] == ':') {
            break;
        }

    }
    
    if(response[i] == '\0') {
        return 0;
    }
    
    const char *state_str = &response[i];

    // 解析参数
    uint8_t link_id = 0;
    char host[128] = {0};
    int port = 0;
    char path[32] = {0};
    int reconnect = 0;
    int state = 0;
    int scheme = 0;

    // 使用sscanf解析参数   
    if (sscanf(state_str, "+MQTTCONN:%hhu,%d,%d,\"%[^\"]\",%d,%32[^,],%d", &link_id, &state, &scheme, host, &port, path, &reconnect) == 7)
    {
        // printf("[ESP8266_QueryMQTTStatus_Connect_Callback]  MQTT Status:connect, host: %s, port: %d, path: %s, reconnect: %d\n", host, port, path, reconnect);
        g_mqtt_status_need_parse_MQTTCONN = 0;

        if(state == 4||state == 5||state == 6)
        {
            esp->mqtt_status = MQTT_STATUS_CONNECTED;
        }
        else
        {
            esp->mqtt_status = MQTT_STATUS_DISCONNECTED;
        }
        return 1;  
    }
    else
    {
        // printf("[ESP8266_QueryMQTTStatus_Connect_Callback]  Parse MQTT Status failed\n");

        g_mqtt_status_need_parse_MQTTCONN = 0;
        return 0;  
    }
}

