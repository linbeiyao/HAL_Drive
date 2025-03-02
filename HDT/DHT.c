#include "DHT.h"

#define lineDown() 		HAL_GPIO_WritePin(sensor->DHT_Port, sensor->DHT_Pin, GPIO_PIN_RESET)
#define lineUp()		HAL_GPIO_WritePin(sensor->DHT_Port, sensor->DHT_Pin, GPIO_PIN_SET)
#define getLine()		(HAL_GPIO_ReadPin(sensor->DHT_Port, sensor->DHT_Pin) == GPIO_PIN_SET)
#define Delay(d)		HAL_Delay(d)

static void goToOutput(DHT_sensor *sensor) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 默认情况下，线路上的电平为高
  lineUp();

  // 将端口配置为输出模式
  GPIO_InitStruct.Pin = sensor->DHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; 	// 开漏输出
  GPIO_InitStruct.Pull = sensor->pullUp;		// 上拉

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
  HAL_GPIO_Init(sensor->DHT_Port, &GPIO_InitStruct);
}

static void goToInput(DHT_sensor *sensor) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 将端口配置为输入模式
  GPIO_InitStruct.Pin = sensor->DHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = sensor->pullUp;		// 上拉
  HAL_GPIO_Init(sensor->DHT_Port, &GPIO_InitStruct);
}

DHT_data DHT_getData(DHT_sensor *sensor) {
	DHT_data data = {-128.0f, -128.0f};
	
	#if DHT_POLLING_CONTROL == 1
	/* 限制传感器的轮询频率 */
	// 根据传感器类型定义轮询间隔
	uint16_t pollingInterval;
	if (sensor->type == DHT11) {
		pollingInterval = DHT_POLLING_INTERVAL_DHT11;
	} else {
		pollingInterval = DHT_POLLING_INTERVAL_DHT22;
	}

	// 如果间隔太短，则返回最后一个成功的值
	if ((HAL_GetTick() - sensor->lastPollingTime < pollingInterval) && sensor->lastPollingTime != 0) {
		data.hum = sensor->lastHum;
		data.temp = sensor->lastTemp;
		return data;
	}
	sensor->lastPollingTime = HAL_GetTick()+1;
	#endif

	/* 请求传感器数据 */
	// 将引脚设置为输出模式
	goToOutput(sensor);
	// 将数据线拉低18毫秒
	lineDown();
	Delay(18);
	// 将数据线拉高，将端口设置为输入模式
	lineUp();
	goToInput(sensor);


	#ifdef DHT_IRQ_CONTROL
	// 禁用中断，以防止干扰数据处理
	__disable_irq();
	#endif
	/* 等待传感器响应 */
	uint16_t timeout = 0;
	// 等待电平下降
	while(getLine()) {
		timeout++;
		if (timeout > DHT_TIMEOUT) {
			#ifdef DHT_IRQ_CONTROL
			__enable_irq();
			#endif
			// 如果传感器没有响应，则传感器肯定不存在
			// 重置最后一个成功的值，以避免假值
			sensor->lastHum = -128.0f;
			sensor->lastTemp = -128.0f;

			return data;
		}
	}
	timeout = 0;
	// 等待电平上升
	while(!getLine()) {
		timeout++;
		if (timeout > DHT_TIMEOUT) {
			#ifdef DHT_IRQ_CONTROL
			__enable_irq();
			#endif
			// 如果传感器没有响应，则传感器肯定不存在
			// 重置最后一个成功的值，以避免假值
			sensor->lastHum = -128.0f;
			sensor->lastTemp = -128.0f;

			return data;
		}
	}
	timeout = 0;
	// 等待电平下降
	while(getLine()) {
		timeout++;
		if (timeout > DHT_TIMEOUT) {
			#ifdef DHT_IRQ_CONTROL
			__enable_irq();
			#endif
			return data;
		}
	}
	
	/* 读取传感器响应 */
	uint8_t rawData[5] = {0,0,0,0,0};
	for(uint8_t a = 0; a < 5; a++) {
		for(uint8_t b = 7; b != 255; b--) {
			uint16_t hT = 0, lT = 0;
			// 当电平为低时，递增变量 lT
			while(!getLine() && lT != 65535) lT++;
			// 当电平为高时，递增变量 hT
			timeout = 0;
			while(getLine()&& hT != 65535) hT++;
			// 如果 hT 大于 lT，则接收到一个1
			if(hT > lT) rawData[a] |= (1<<b);
		}
	}

    #ifdef DHT_IRQ_CONTROL
	// 接收数据后启用中断
	__enable_irq();
    #endif

	/* 检查数据完整性 */
	if((uint8_t)(rawData[0] + rawData[1] + rawData[2] + rawData[3]) == rawData[4]) {
		// 如果校验和匹配，则转换并返回接收到的值
		if (sensor->type == DHT22) {
			data.hum = (float)(((uint16_t)rawData[0]<<8) | rawData[1])*0.1f;
			// 检查温度是否为负数
			if(!(rawData[2] & (1<<7))) {
				data.temp = (float)(((uint16_t)rawData[2]<<8) | rawData[3])*0.1f;
			}	else {
				rawData[2] &= ~(1<<7);
				data.temp = (float)(((uint16_t)rawData[2]<<8) | rawData[3])*-0.1f;
			}
		}
		if (sensor->type == DHT11) {
			data.hum = (float)rawData[0];
			data.temp = (float)rawData[2];
		}
	}
	
	#if DHT_POLLING_CONTROL == 1
	sensor->lastHum = data.hum;
	sensor->lastTemp = data.temp;
	#endif

	return data;	
}

