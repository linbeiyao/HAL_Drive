#ifndef DHT_H_
#define DHT_H_

#include "stm32f1xx_hal.h"

/* 设置 */
#define DHT_TIMEOUT 				10000	// 迭代次数，超过后函数将返回空值
#define DHT_POLLING_CONTROL			1		// 启用传感器轮询频率检查
#define DHT_POLLING_INTERVAL_DHT11	2000	// DHT11 的轮询间隔（数据手册中为 0.5 Hz）。可以设置为 1500，也能正常工作
#define DHT_POLLING_INTERVAL_DHT22	1000	// DHT22 的轮询间隔（数据手册中为 1 Hz）
#define DHT_IRQ_CONTROL						// 在与传感器交换数据时禁用中断

/* 传感器返回的数据结构 */
typedef struct {
	float hum;
	float temp;
} DHT_data;

/* 使用的传感器类型 */
typedef enum {
	DHT11,
	DHT22
} DHT_type;

/* 传感器对象结构 */
typedef struct {
	GPIO_TypeDef *DHT_Port;	// 传感器端口（GPIOA, GPIOB 等）
	uint16_t DHT_Pin;		// 传感器引脚号（GPIO_PIN_0, GPIO_PIN_1 等）
	DHT_type type;			// 传感器类型（DHT11 或 DHT22）
	uint8_t pullUp;			// 是否需要将数据线拉高到电源（GPIO_NOPULL - 不需要，GPIO_PULLUP - 需要）

	// 传感器轮询频率控制。无需填写这些值！
	#if DHT_POLLING_CONTROL == 1
	uint32_t lastPollingTime;	// 上次轮询传感器的时间
	float lastTemp;				// 上次读取的温度值
	float lastHum;				// 上次读取的湿度值
	#endif
} DHT_sensor;

#include "main.h"

/* 函数原型 */
DHT_data DHT_getData(DHT_sensor *sensor); // 从传感器获取数据

#endif




