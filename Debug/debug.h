// debug.h

#ifndef DEBUG_H
#define DEBUG_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

// 定义调试等级
#define DEBUG_LEVEL_NONE   0
#define DEBUG_LEVEL_ERROR  1
#define DEBUG_LEVEL_WARN   2
#define DEBUG_LEVEL_INFO   3
#define DEBUG_LEVEL_DEBUG  4
#define DEBUG_LEVEL_VERBOSE 5

// 设置当前调试等级
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_DEBUG
#endif

// 定义串口句柄
extern UART_HandleTypeDef huart1;

// 串口初始化函数
void Debug_Init(uint32_t baudrate);

// 发送原始数据函数
void Debug_Send(uint8_t *data, uint16_t size);

// 计时功能相关变量和函数
typedef struct {
    uint32_t start_time;
    uint32_t elapsed_time;
    char name[32];
} Debug_Timer_t;

void Debug_TimerStart(Debug_Timer_t *timer, const char *name);
void Debug_TimerStop(Debug_Timer_t *timer);
void Debug_TimerPrint(Debug_Timer_t *timer);

// 可变参数宏，用于格式化输出
#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
#define DEBUG_ERROR(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "[ERROR] " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_ERROR(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARN
#define DEBUG_WARN(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "[WARN] " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_WARN(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define DEBUG_INFO(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "[INFO] " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_INFO(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DEBUG_DEBUG(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "[DEBUG] " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_DEBUG(fmt, ...) do {} while (0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
#define DEBUG_VERBOSE(fmt, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), "[VERBOSE] " fmt "\r\n", ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); \
    } while (0)
#else
#define DEBUG_VERBOSE(fmt, ...) do {} while (0)
#endif

// 十六进制数据打印函数宏
#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define DEBUG_HEX_DUMP(desc, addr, len) Debug_HexDump(desc, addr, len)
void Debug_HexDump(const char *desc, const void *addr, int len);
#else
#define DEBUG_HEX_DUMP(desc, addr, len) do {} while (0)
#endif

// 输出当前函数名和行号
#define DEBUG_TRACE() DEBUG_DEBUG("TRACE: %s:%d", __FUNCTION__, __LINE__)

// 条件断言
#define DEBUG_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            DEBUG_ERROR("Assertion failed at %s:%d - %s", __FILE__, __LINE__, #condition); \
            while (1); \
        } \
    } while (0)

#endif // DEBUG_H

