#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

/**
 * ESP8266 MQTT配置文件
 * ===================
 * 
 * 本文件包含了ESP8266 MQTT通信所需的配置参数,支持普通MQTT和OneNET平台。
 * 
 * 使用说明:
 * 1. 选择MQTT平台
 *    - 在SECELTMQTTPLATFORM宏中选择NORMAL或ONENET
 * 
 * 2. 配置WiFi连接
 *    - 修改WIFI_SSID为你的WiFi名称
 *    - 修改WIFI_PASSWORD为你的WiFi密码
 * 
 * 3. 普通MQTT平台配置
 *    - 修改MQTT_SERVER为你的MQTT服务器地址
 *    - 修改MQTT_PORT为服务器端口(默认1883)
 *    - 修改MQTT_CLIENT_ID为唯一的客户端标识
 *    - 根据需要修改用户名密码和主题
 * 
 * 4. OneNET平台配置
 *    - 在OneNET平台创建产品和设备
 *    - 将产品ID填入PRODUCT_ID
 *    - 将设备名称填入DEVICE_NAME
 *    - 将设备密钥填入DEVICE_KEY
 *    - 使用OneNET提供的工具生成Token并填入
 * 
 * 注意事项:
 * - 请妥善保管密钥等敏感信息
 * - 建议将实际的密钥配置放在单独的配置文件中
 * - 遗嘱消息用于设备离线通知
 */

#include "main.h"
#include "esp8266.h"

/* MQTT平台选择 */
#define NOMAL 1   // 普通MQTT平台
#define ONENET 2  // OneNET平台

// 选择使用的MQTT平台
#define SECELTMQTTPLATFORM ONENET

/* 通用配置 */
#define ESP8266_SYSNAME "SoilMoisture"  // 设备名称
#define WIFI_SSID "2"                   // WiFi名称
#define WIFI_PASSWORD ""                // WiFi密码
#define MQTT_SERVER "mqtts.heclouds.com"// MQTT服务器地址
#define MQTT_PORT 1883                  // MQTT服务器端口


// 遗嘱消息
#define MQTT_TOPIC_Will_Message ESP8266_SYSNAME ":offline"

/* 普通MQTT平台配置 */
#if (SECELTMQTTPLATFORM == NOMAL)

// 基本配置
#define MQTT_USERNAME ESP8266_SYSNAME   // MQTT用户名(使用设备名称)
#define MQTT_PASSWORD ESP8266_SYSNAME   // MQTT密码(使用设备名称)
#define MQTT_CLIENT_ID "1"              // MQTT客户端ID
#define MQTT_IP MQTT_SERVER             // MQTT服务器IP
#define MQTT_PORT_NUM "18083"           // MQTT Web管理端口

// MQTT主题
#define MQTT_TOPIC_Subscribe "/" ESP8266_SYSNAME "/control"  // 订阅主题
#define MQTT_TOPIC_Publish "/" ESP8266_SYSNAME "/data"       // 发布主题
#define MQTT_TOPIC_Will "/" ESP8266_SYSNAME "/Will"          // 遗嘱主题

/* OneNET平台配置 */
#elif (SECELTMQTTPLATFORM == ONENET)

/**
 * OneNET平台配置信息
 * =================
 * 
 * 配置步骤:
 * 1. 登录OneNET平台(https://open.iot.10086.cn/)
 * 2. 创建产品
 *    - 选择MQTT协议
 *    - 记录产品ID
 * 3. 创建设备
 *    - 选择认证方式
 *    - 记录设备ID和密钥
 * 4. 生成Token
 *    - 使用OneNET提供的工具
 *    - 设置合适的有效期
 * 
 * 地址：mqtts.heclouds.com
 * 端口：1883
 * 
 * 产品信息:
 * - 产品ID: X15T01ZVAx
 * - 设备ID: SoilMoisture_System
 * - 设备名称: SoilMoisture_System
 * 
 * 安全认证:
 * - 设备密钥: aHJCcnBRcFdEUU5zYksydkhsU0MyTXBna0c3R0R0czA=
 * 
 * Token信息:
 * - 使用OneNET-token工具生成   et 时间戳一定要在今天的日期之后
 * - Token值: version=2018-10-31&res=products%2FX15T01ZVAx%2Fdevices%2FSoilMoisture_System&et=1988121600&method=sha256&sign=IS8MliNutFuKugawuj7bnaHrCFrmLAgcmbXs1dIjazA%3D
 */

// 产品和设备配置
#define PRODUCT_ID "X15T01ZVAx"                                    // 产品ID
#define DEVICE_NAME "SoilMoisture_System"                         // 设备名称
#define DEVICE_KEY "VU5Pd0hqeWkxMW9qbXB6enRaU21wUWN4VnJta0FodGI=" // 设备密钥
#define TOKEN "version=2018-10-31&res=products%2FX15T01ZVAx%2Fdevices%2FSoilMoisture_System&et=1988121600&method=sha256&sign=IS8MliNutFuKugawuj7bnaHrCFrmLAgcmbXs1dIjazA%3D"
						// version=2018-10-31&res=products%2FX15T01ZVAx%2Fdevices%2FSoilMoisture_System&et=1988121600&method=sha256&sign=IS8MliNutFuKugawuj7bnaHrCFrmLAgcmbXs1dIjazA%3D
// 网络配置
#define MQTT_IP MQTT_SERVER
#define MQTT_CLIENT_ID DEVICE_NAME   // 客户端ID(使用设备名称)
#define MQTT_USERNAME PRODUCT_ID     // 用户名(使用产品ID)
#define MQTT_PASSWORD TOKEN     		// 密码 TOKEN

/**
 * MQTT配置命令说明
 * ==============
 * 
 * AT+MQTTUSERCFG命令格式:
 * AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,
 *                <cert_key_ID>,<CA_ID>,<"path">
 * 
 * 参数说明:
 * - LinkID: 连接ID(固定为0)
 * - scheme: 加密方式(1表示TCP)
 * - client_id: 客户端ID(使用设备名称)
 * - username: 用户名(使用产品ID)
 * - password: 密码(使用Token)
 * - cert_key_ID: 证书密钥ID(不使用为0)
 * - CA_ID: CA证书ID(不使用为0)
 * - path: 路径(不使用为空)
 */



/* MQTT 主题配置 */

// 设备属性上报主题
#define MQTT_TOPIC_PROPERTY_POST "$sys/" PRODUCT_ID "/" DEVICE_NAME "/thing/property/post"        // 上行数据发布主题
/**
 * 示例响应格式：
 * {
 *   "id": "123",                  // 消息ID
 *   "version": "1.0",             // 协议版本
 *   "params": {                   // 参数对象
 *     "Power": {                  // 电源状态
 *       "value": "on",            // 状态值
 *       "time": 1524448722123     // 时间戳
 *     },
 *     "WF": {                     // 其他参数
 *       "value": 23.6,            // 数值
 *       "time": 1524448722123     // 时间戳
 *     }
 *   }
 * }
 */
#define MQTT_TOPIC_PROPERTY_POST_REPLY "$sys/" PRODUCT_ID "/" DEVICE_NAME "/thing/property/post_reply" // 上行数据响应主题
/**
 * 示例响应格式：
 * {
 *   "id": "123",        // 消息ID，与请求ID对应
 *   "code": 200,        // 响应状态码
 *   "msg": "xxxx"       // 响应消息
 * }
 */
// 设备属性设置主题  
#define MQTT_TOPIC_PROPERTY_SET "$sys/" PRODUCT_ID "/" DEVICE_NAME "/thing/property/set"               // 属性设置主题
/**
 * 示例响应格式：
 * {
 *   "id": "123",
 *   "version": "1.0",
 *   "params": {
 *     "temperature": "30.5"
 *   }
 * }
 */
#define MQTT_TOPIC_PROPERTY_SET_REPLY "$sys/" PRODUCT_ID "/" DEVICE_NAME "/thing/property/set_reply"    // 属性设置响应主题
/**
 * 示例响应格式：
 * {
 *   "id": "123",
 *   "code": 200,
 *   "msg": "xxxx"
 * }
 */

// 设备属性查询主题
#define MQTT_TOPIC_PROPERTY_GET "$sys/" PRODUCT_ID "/" DEVICE_NAME "/thing/property/get"        // 下行数据发布主题
/**
 * 示例响应格式：
 * {
 *   "id": "123",
 *   "version": "1.0",
 *   "params": [
 *     "temperature",
 *     "humidity"
 *   ]
 * }
 */
#define MQTT_TOPIC_PROPERTY_GET_REPLY "$sys/" PRODUCT_ID "/" DEVICE_NAME "/thing/property/get_reply" // 下行数据响应主题
/**
 * 示例响应格式：
 * {
 *   "id": "123",
 *   "code": 200,
 *   "msg": "xxx",
 *   "data": {
 *     "temperature": 39.5,
 *     "humidity": 20
 *   }
 * }
 */



// MQTT主题(遗嘱)
#define MQTT_TOPIC_Will "$sys/" PRODUCT_ID "/" DEVICE_NAME "/status"

#endif






#endif

