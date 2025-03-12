#include "mqtt.h"
#include "esp8266.h"
#include "stm32f1xx_it.h"
#include <stdio.h>

/* MQTT DEBUG */
#define MQTT_DEBUG 	1

/** MQTT 连接参数定义 **/
/* 客户端ID PS:这个复制过来之后需要在逗号前加\\ */
#define Client_ID 		 "stm32_intelligent_lock|securemode=2\\,signmethod=hmacsha1\\,timestamp=1716281808068|"

/* Broker域名地址 */
#define Broker_Address 	 "iot-06z00h6z06dknsn.mqtt.iothub.aliyuncs.com"

/* 用户名 */
#define User_Name 		 "stm32_intelligent_lock&j13nl5xV77p"

/* 用户密码 */
#define Password 		 "26F6B6BAEEDC0DD83337480E447732324D7B7BA5"

/* 订阅主题 */
#define Sub_Topic        "/sys/j13nl5xV77p/stm32_intelligent_lock/thing/service/property/set"

/* 发布主题 */
#define Pub_Topic        "/sys/j13nl5xV77p/stm32_intelligent_lock/thing/event/property/post"

/* 发布的格式 */
#define JSON_FORMAT      "{\\\"params\\\":{\\\"LEDSwitch\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

//#define JSON_FORMAT      "{\"params\":{\"LEDSwitch\":%d},\"version\":\"1.0.0\"}"



/**
  * @brief  mqtt连接阿里云物联网平台
  * @param	void
  * 
  * @retval void
  */
void mqtt_connect(void)
{
	/* MQTT用户配置 */
#if MQTT_DEBUG
	printf("1. MQTT USER CONFIG\r\n");
#endif
	while(ESP8266_SendCmd((char *)"AT+MQTTUSERCFG=0,1,\""Client_ID"\",\""User_Name"\",\""Password"\",0,0,\"\"\r\n", "OK"))
		HAL_Delay(500);
	
	/* MQTT Broker配置 */
#if MQTT_DEBUG
	printf("2.CONNECT MQTT BROKER\r\n");
#endif
	while(ESP8266_SendCmd((char *)"AT+MQTTCONN=0,\""Broker_Address"\",1883,0\r\n", "OK"))
		HAL_Delay(500); 
	
	/* 订阅主题 */
#if MQTT_DEBUG
	printf("3.SUBSCRIBE TOPIC\r\n");
#endif
	while(ESP8266_SendCmd((char *)"AT+MQTTSUB=0,\""Sub_Topic"\",0\r\n", "OK"))
		HAL_Delay(500); 
#if MQTT_DEBUG	
	printf("4.MQTT CONNECT SUCCEED");
#endif
}

/**
  * @brief  mqtt发布数据
  * @param	void
  * 
  * @retval void
  */
void mqtt_pub(void)
{
	unsigned char msg_buf[256];
	
	/* 根据JSON_FORMAT格式传入对应数据即可 */
	sprintf((char *)msg_buf, "AT+MQTTPUB=0,\""Pub_Topic"\",\""JSON_FORMAT"\",0,0\r\n", 1);
	
	while(ESP8266_SendCmd((char *)msg_buf, "OK"));
	
#if MQTT_DEBUG	
	printf("mqtt pub succeed");	
#endif
}

/**
  * @brief  mqtt订阅数据
  * @param	void
  * 
  * @retval void
  */
void mqtt_sub(unsigned char* dataPtr)
{
	
}



