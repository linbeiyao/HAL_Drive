#include "esp8266.h"




/**
 * @brief 回调函数
 * 
 * @param message 
 * @return 
 */
uint8_t ESP8266_CallBack(const char *message){
    if (g_wifi_status_need_parse_CWSTATE)
    {
        ESP8266_QueryWiFiStatus_CWSTATE_Callback(message);
    }
    else if (g_wifi_status_need_parse_CWJAP)
    {
        ESP8266_QueryWiFiStatus_CWJAP_Callback(message);
    }
    else 
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
void ESP8266_QueryWiFiStatus_CWSTATE_Callback(const char *response)
{
    // 解析响应字符串
    char *state_str = strstr(response, "+CWSTATE:");
    if (state_str == NULL)
    {
        return;
    }
    else
    {
        // 解析状态和SSID
        int state = 0;
        char ssid[32] = {0};
        
        // 使用sscanf解析状态和SSID
        if (sscanf(state_str, "+CWSTATE:%d,\"%[^\"]\"", &state, ssid) == 2)
        {
            // 根据状态进行相应处理
            switch (state)
            {
            case 0:
                printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:no connect\n");
                g_wifi_status_need_parse_CWSTATE =  0;
                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
                break;
            case 1:
                printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:connect AP but no get IP, SSID: %s\n", ssid);
                g_wifi_status_need_parse_CWSTATE =  0;
                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
                break;
            case 2:
                printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:connect AP and get IP, SSID: %s\n", ssid);
                g_wifi_status_need_parse_CWSTATE =  0;
                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,1);
                break;
            case 3:
                printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:connecting or reconnecting, SSID: %s\n", ssid);
                g_wifi_status_need_parse_CWSTATE =  0;
                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
                break;
            case 4:
                printf("[ESP8266_QueryWiFiStatus_Callback]  WiFi Status:disconnect\n");
                g_wifi_status_need_parse_CWSTATE =  0;
                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
                break;
            default:
                printf("[ESP8266_QueryWiFiStatus_Callback]  Unknown WiFi Status: %d\n", state);
                g_wifi_status_need_parse_CWSTATE =  0;
                FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
                break;
            }
        }
        else
        {
            printf("[ESP8266_QueryWiFiStatus_Callback]  Parse WiFi Status failed\n");
            g_wifi_status_need_parse_CWSTATE = 0;
            FruitVendingData_SetNetworkConnected(&g_fruitVendingData,0);
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
void ESP8266_QueryWiFiStatus_CWJAP_Callback(const char *response){
    // 解析响应字符串
    char *state_str = strstr(response, "+CWJAP:");
    if (state_str == NULL)
    {
        return;
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

    // 使用sscanf解析参数
    if (sscanf(state_str, "+CWJAP:%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d", ssid, bssid, &channel, &rssi, &pci_en, &reconn_interval, &listen_interval, &scan_mode, &pmf) == 9)
    {
        printf("[ESP8266_QueryWiFiStatus_CWJAP_Callback]  WiFi Status:connect AP and get IP, SSID: %s\n", ssid);
        g_wifi_status_need_parse_CWJAP = 0;
    }
    else
    {
        printf("[ESP8266_QueryWiFiStatus_CWJAP_Callback]  Parse WiFi Status failed\n");
        g_wifi_status_need_parse_CWJAP = 0;
    }  
}