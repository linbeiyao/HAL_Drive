#include "mqtt.h"
#include "esp8266.h"
#include "stm32f1xx_it.h"
#include <stdio.h>

/* MQTT 调试 */
#define MQTT_DEBUG 	1

/** MQTT 连接参数定义 **/
/* 客户端ID 注:如果有特殊字符需要在定义前加\\ */
#define Client_ID 		 "SoilMoisture_System"

/* Broker服务器地址 */
#define Broker_Address 	 "mqtts.heclouds.com"

/* 用户名 */
#define User_Name 		 "X15T01ZVAx"

/* 用户密码 */
#define Password 		 "version=2018-10-31&res=products%2FX15T01ZVAx%2Fdevices%2FSoilMoisture_System&et=1988121600&method=sha256&sign=IS8MliNutFuKugawuj7bnaHrCFrmLAgcmbXs1dIjazA%3D"

/* 订阅主题 */
#define Sub_Topic        "$sys/X15T01ZVAx/SoilMoisture_System/dp/post/json/+"

/* 发布主题 */
#define Pub_Topic        "$sys/X15T01ZVAx/SoilMoisture_System/dp/post/json"

/* 发送的格式 */
//#define JSON_FORMAT      "{\\\"params\\\":{\\\"LEDSwitch\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

#define JSON_FORMAT      "{\\\"id\\\":%d\\,\\\"dp\\\":{\\\"LOCKLOG\\\":[{\\\"v\\\":%d\\,}]\\,}}"

/**
  * @brief  MQTT连接OneNET
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
	
	/* MQTT Broker连接 */
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
  * @brief  MQTT发布消息
  * @param	void
  * 
  * @retval void
  */
void mqtt_pub(void)
{
	unsigned char msg_buf[256];
	
	/* 按照JSON_FORMAT格式填充对应数据进去 */
	sprintf((char *)msg_buf, "AT+MQTTPUB=0,\""Pub_Topic"\",\""JSON_FORMAT"\",0,0\r\n", 1, 1);
	
	while(ESP8266_SendCmd((char *)msg_buf, "OK"));
	
#if MQTT_DEBUG	
	printf("mqtt pub succeed");	
#endif
}

/**
  * @brief  MQTT订阅消息
  * @param	void
  * 
  * @retval void
  */
void mqtt_sub(unsigned char* dataPtr)
{
	
}

