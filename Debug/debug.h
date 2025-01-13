// debug.h

#ifndef DEBUG_H
#define DEBUG_H

#include "stm32f4xx_hal.h"

// 定义调试等级
#define DEBUG_LEVEL_NONE   0
#define DEBUG_LEVEL_ERROR  1
#define DEBUG_LEVEL_WARN   2
#define DEBUG_LEVEL_INFO   3
#define DEBUG_LEVEL_DEBUG  4

// 设置当前调试等级
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_DEBUG
#endif

// 定义输出函数，例如通过 UART
// extern UART_HandleTypeDef huart1;

// 可变参数宏，用于格式化输出
#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
#define DEBUG_ERROR(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "ERROR: " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_ERROR(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARN
#define DEBUG_WARN(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "WARN: " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_WARN(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define DEBUG_INFO(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "INFO: " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_INFO(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DEBUG_DEBUG(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "DEBUG: " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_DEBUG(fmt, ...) do {} while (0)
#endif

#endif // DEBUG_H

