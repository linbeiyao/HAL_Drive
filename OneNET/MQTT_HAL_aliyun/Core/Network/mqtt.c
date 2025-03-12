#include "mqtt.h"
#include "esp8266.h"
#include "stm32f1xx_it.h"
#include <stdio.h>

/* MQTT DEBUG */
#define MQTT_DEBUG 	1

/** MQTT ���Ӳ������� **/
/* �ͻ���ID PS:������ƹ���֮����Ҫ�ڶ���ǰ��\\ */
#define Client_ID 		 "stm32_intelligent_lock|securemode=2\\,signmethod=hmacsha1\\,timestamp=1716281808068|"

/* Broker������ַ */
#define Broker_Address 	 "iot-06z00h6z06dknsn.mqtt.iothub.aliyuncs.com"

/* �û��� */
#define User_Name 		 "stm32_intelligent_lock&j13nl5xV77p"

/* �û����� */
#define Password 		 "26F6B6BAEEDC0DD83337480E447732324D7B7BA5"

/* �������� */
#define Sub_Topic        "/sys/j13nl5xV77p/stm32_intelligent_lock/thing/service/property/set"

/* �������� */
#define Pub_Topic        "/sys/j13nl5xV77p/stm32_intelligent_lock/thing/event/property/post"

/* �����ĸ�ʽ */
#define JSON_FORMAT      "{\\\"params\\\":{\\\"LEDSwitch\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

//#define JSON_FORMAT      "{\"params\":{\"LEDSwitch\":%d},\"version\":\"1.0.0\"}"



/**
  * @brief  mqtt���Ӱ�����������ƽ̨
  * @param	void
  * 
  * @retval void
  */
void mqtt_connect(void)
{
	/* MQTT�û����� */
#if MQTT_DEBUG
	printf("1. MQTT USER CONFIG\r\n");
#endif
	while(ESP8266_SendCmd((char *)"AT+MQTTUSERCFG=0,1,\""Client_ID"\",\""User_Name"\",\""Password"\",0,0,\"\"\r\n", "OK"))
		HAL_Delay(500);
	
	/* MQTT Broker���� */
#if MQTT_DEBUG
	printf("2.CONNECT MQTT BROKER\r\n");
#endif
	while(ESP8266_SendCmd((char *)"AT+MQTTCONN=0,\""Broker_Address"\",1883,0\r\n", "OK"))
		HAL_Delay(500); 
	
	/* �������� */
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
  * @brief  mqtt��������
  * @param	void
  * 
  * @retval void
  */
void mqtt_pub(void)
{
	unsigned char msg_buf[256];
	
	/* ����JSON_FORMAT��ʽ�����Ӧ���ݼ��� */
	sprintf((char *)msg_buf, "AT+MQTTPUB=0,\""Pub_Topic"\",\""JSON_FORMAT"\",0,0\r\n", 1);
	
	while(ESP8266_SendCmd((char *)msg_buf, "OK"));
	
#if MQTT_DEBUG	
	printf("mqtt pub succeed");	
#endif
}

/**
  * @brief  mqtt��������
  * @param	void
  * 
  * @retval void
  */
void mqtt_sub(unsigned char* dataPtr)
{
	
}



