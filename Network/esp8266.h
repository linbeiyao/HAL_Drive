#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f1xx_hal.h"

/**
 * @file esp8266.h
 * @brief ESP8266 WiFi模块驱动头文件
 * 
 * @硬件连接配置:
 * ESP8266 --- STM32
 * VCC    --- 3.3V (注意：需要能提供至少500mA电流)
 * GND    --- GND
 * TX     --- PA3 (STM32的USART2_RX)
 * RX     --- PA2 (STM32的USART2_TX)
 * CH_PD  --- 3.3V (ESP8266使能引脚，必须接高电平)
 * RST    --- 可选连接到STM32的GPIO，用于硬件复位
 * 
 * @软件配置:
 * 1. 在STM32 CubeMX中配置USART2:
 *    - 波特率: 115200
 *    - 数据位: 8
 *    - 停止位: 1
 *    - 校验位: 无
 *    - 流控制: 无
 *    - 开启接收中断
 * 
 * 2. 修改WiFi连接信息:
 *    - 修改ESP8266_WIFI_INFO宏定义中的WiFi名称和密码
 *    - 默认格式: "AT+CWJAP=\"WiFi名称\",\"WiFi密码\"\r\n"
 * 
 * @使用流程:
 * 1. 调用ESP8266_Init()初始化模块
 * 2. 等待初始化完成(会自动连接WiFi)
 * 3. 使用ESP8266_SendData()发送数据
 * 4. 使用ESP8266_Receive()接收数据
 * 
 * @故障排除:
 * 1. 如果初始化失败，检查:
 *    - 电源是否稳定且电流足够
 *    - 串口连接是否正确
 *    - WiFi名称和密码是否正确
 * 2. 如果数据发送/接收失败，检查:
 *    - WiFi连接状态
 *    - 网络服务器是否可达
 */

//#include "cJSON.h"

/* 返回值定义 */
#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

/* 缓冲区大小定义 */
#define MAXBUFFERSIZE 1024  //最大缓冲区大小

/* WIFI连接定义 */
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"2\",\"\"\r\n"

/* 全局变量声明 */
extern uint8_t aRxBuffer;                            // 接收中断缓存
extern unsigned char esp8266_buf[MAXBUFFERSIZE];     // ESP8266接收缓冲区
extern unsigned short esp8266_cnt;                   // 接收计数器
extern unsigned short esp8266_cntPre;                // 上一次接收计数器



/* 函数声明 */
void ESP8266_Init(void);                                    //ESP8266初始化
void ESP8266_Clear(void);                                   //清除ESP8266缓存
void ESP8266_SendData(unsigned char *data, unsigned short len);  //ESP8266发送数据
_Bool ESP8266_SendCmd(char *cmd, char *res);                //ESP8266发送命令

unsigned char *ESP8266_GetIPD(unsigned short timeOut);      //获取IPD数据
unsigned char* ESP8266_Receive(unsigned short timeOut);     //接收数据

void ESP8266_UART_RxHandler(UART_HandleTypeDef *huart);     //ESP8266串口接收处理函数


void SetPassThrough(void);                                  //设置透传模式
void RstPassThrough(void);                                  //重置透传模式


#endif /* __ESP8266_H */
