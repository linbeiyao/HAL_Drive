#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "main.h"

/**
 * @brief 初始化天气模块
 * @param None
 * @return None
 */
void Weather_Init(void);

/**
 * @brief 获取今天的降雨概率
 * @param None
 * @return 今天的降雨概率(0-100)
 */
uint8_t Weather_GetTodayRainProbability(void);

/**
 * @brief 获取明天的降雨概率
 * @param None
 * @return 明天的降雨概率(0-100)
 */
uint8_t Weather_GetTomorrowRainProbability(void);

/**
 * @brief 更新天气数据
 * @param location 城市名称，如"beijing"
 * @return 0-成功，1-失败
 */
uint8_t Weather_Update(const char *location);

/**
 * @brief 模拟天气数据(用于测试)
 * @param today_prob 今天的降雨概率
 * @param tomorrow_prob 明天的降雨概率
 * @return None
 */
void Weather_SimulateData(uint8_t today_prob, uint8_t tomorrow_prob);

#endif /* __WEATHER_H__ */ 