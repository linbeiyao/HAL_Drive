#include "main.h"
#include "esp8266.h"
#include "usart.h"
#include "uart_user.h"
#include "weather.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 天气API的URL模板
#define WEATHER_API_URL "api.seniverse.com"
#define WEATHER_API_PATH "/v3/weather/daily.json?key=%s&location=%s&language=zh-Hans&unit=c&start=0&days=3"
#define WEATHER_API_KEY "YOUR_API_KEY" // 替换为你的API密钥

// 全局变量，存储解析后的天气数据
static uint8_t today_rain_probability = 0;
static uint8_t tomorrow_rain_probability = 0;

// 解析天气数据的函数
static void parse_weather_data(const char *data);

/**
 * @brief 初始化天气模块
 * @param None
 * @return None
 */
void Weather_Init(void)
{
    // 初始化天气数据
    today_rain_probability = 0;
    tomorrow_rain_probability = 0;
}

/**
 * @brief 获取今天的降雨概率
 * @param None
 * @return 今天的降雨概率(0-100)
 */
uint8_t Weather_GetTodayRainProbability(void)
{
    return today_rain_probability;
}

/**
 * @brief 获取明天的降雨概率
 * @param None
 * @return 明天的降雨概率(0-100)
 */
uint8_t Weather_GetTomorrowRainProbability(void)
{
    return tomorrow_rain_probability;
}

/**
 * @brief 更新天气数据
 * @param location 城市名称，如"beijing"
 * @return 0-成功，1-失败
 */
uint8_t Weather_Update(const char *location)
{
    char cmd[256];
    char url[256];
    
    // 清空ESP8266缓存
    ESP8266_Clear();
    
    // 建立TCP连接
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",80\r\n", WEATHER_API_URL);
    if(ESP8266_SendCmd(cmd, "OK"))
    {
        printf("TCP连接失败\r\n");
        return 1;
    }
    
    // 构建HTTP请求
    sprintf(url, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", 
            WEATHER_API_PATH, WEATHER_API_URL);
    
    // 替换API密钥和位置
    char *key_pos = strstr(url, "key=%s");
    if(key_pos)
    {
        sprintf(key_pos, "key=%s", WEATHER_API_KEY);
    }
    
    char *location_pos = strstr(url, "&location=%s");
    if(location_pos)
    {
        sprintf(location_pos, "&location=%s", location);
    }
    
    // 发送HTTP请求
    sprintf(cmd, "AT+CIPSEND=%d\r\n", strlen(url));
    if(ESP8266_SendCmd(cmd, ">"))
    {
        printf("发送请求失败\r\n");
        return 1;
    }
    
    // 发送HTTP请求内容
    Usart_SendString(&huart2, (unsigned char *)url, strlen(url));
    
    // 等待接收数据
    unsigned char timeOut = 200;
    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)
        {
            // 查找JSON数据开始的位置
            char *json_start = strstr((char *)esp8266_buf, "{\"results\"");
            if(json_start)
            {
                // 解析天气数据
                parse_weather_data(json_start);
                ESP8266_Clear();
                return 0;
            }
        }
        HAL_Delay(10);
    }
    
    // 关闭TCP连接
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
    
    return 1;
}

/**
 * @brief 解析天气数据
 * @param data JSON格式的天气数据
 * @return None
 */
static void parse_weather_data(const char *data)
{
    // 简单解析，实际项目中应使用JSON解析库
    char *daily = strstr(data, "\"daily\"");
    if(!daily) return;
    
    // 查找今天的降雨概率
    char *today = strstr(daily, "\"precip\"");
    if(today)
    {
        today += 9; // 跳过"precip":"
        float precip = atof(today);
        today_rain_probability = (uint8_t)(precip * 100); // 转换为百分比
    }
    
    // 查找明天的降雨概率
    char *tomorrow = strstr(today + 10, "\"precip\"");
    if(tomorrow)
    {
        tomorrow += 9; // 跳过"precip":"
        float precip = atof(tomorrow);
        tomorrow_rain_probability = (uint8_t)(precip * 100); // 转换为百分比
    }
}

/**
 * @brief 模拟天气数据(用于测试)
 * @param today_prob 今天的降雨概率
 * @param tomorrow_prob 明天的降雨概率
 * @return None
 */
void Weather_SimulateData(uint8_t today_prob, uint8_t tomorrow_prob)
{
    today_rain_probability = today_prob;
    tomorrow_rain_probability = tomorrow_prob;
} 