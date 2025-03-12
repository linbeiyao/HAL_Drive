#include "main.h"
#include "esp8266.h"
#include "usart.h"
#include "uart_user.h"

#include <string.h>
#include <stdio.h>
//#include "cJSON.h"

/* WIFI密码定义 */
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Channel_2.4G\",\"password\"\r\n"
#define MAXBUFFERSIZE 1024

uint8_t aRxBuffer;                            // 接收中断缓冲 每次接收一个字节
unsigned char esp8266_buf[MAXBUFFERSIZE];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0;							//清0接收计数
			
		return REV_OK;								//返回接收完成标志
	}
		
	esp8266_cntPre = esp8266_cnt;					//置为相同
	
	return REV_WAIT;								//返回接收未完成标志
}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(&huart2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//如果检索到关键词
			{
				//printf("buf:%s\r\n",(const char *)esp8266_buf);
				ESP8266_Clear();									//清空缓存
				return 0;
			}
		}
		HAL_Delay(10);
	}
	
	return 1;
}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[256];
	
	ESP8266_Clear();								//清空接收缓存
	
	//先发送要发送数据的指令做准备
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//发送命令
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//收到‘>’时可以发送数据
	{
		//既然准备完毕即可开始发送数据
		Usart_SendString(&huart2, data, len);		//发送设备连接请求数据
	}

}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "{");				//搜索“{”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
				printf("error\r\n");
				return NULL;
			}
			else
			{
				ptrIPD = strstr((char *)esp8266_buf, "\"");							
				if(ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
				
			}
		}
		
		HAL_Delay(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================

void ESP8266_Init(void)
{
	HAL_UART_MspInit(&huart2);
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);   //开启接收中断
	
	ESP8266_Clear();	// 清除接收缓冲区
	
	/* 测试AT指令 */
	printf("1. AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		HAL_Delay(500);
	
	/* 重启 */
	printf("2. RST\r\n");
	while(ESP8266_SendCmd("AT+RST\r\n", "OK"))
		HAL_Delay(500);
	
	/* 设置station模式 */
	printf("3. CWMODE\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(500);
	
	/* 启动DHCP后续自动获取ip */
	printf("4. AT+CWDHCP\r\n");
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(500);
	
	/* 连接WIFI */
	printf("5. CWJAP\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "WIFI GOT IP"))
		HAL_Delay(500);
	
	printf("6. ESP8266 Init OK\r\n");
	
//	while(ESP8266_SendCmd(MQTTUSERCFG, "OK"))
//		HAL_Delay(500);
}


//==========================================================
//	函数名称：	RstPassThrough
//
//	函数功能：	开启透传
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void SetPassThrough(void)
{
	ESP8266_Clear();	// 清空接收内存
	
	/* 开启透传模式 */
	printf( "AT+CIPMODE=1\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"))
		HAL_Delay(500);
	
	/* 透传模式下传输数据 */
	printf( "AT+CIPSEND\r\n");
	while(ESP8266_SendCmd("AT+CIPSEND\r\n", ">"))
		HAL_Delay(500);
}
//==========================================================
//	函数名称：	RstPassThrough
//
//	函数功能：	关闭透传
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void RstPassThrough(void)
{
	ESP8266_Clear();	// 清空接收内存
	
	/* 退出透传模式没有/r/n */
	Usart_SendString(&huart2, (unsigned char *)"+++", strlen("+++"));
	HAL_Delay(100);
	printf("11.AT+CIPSEND=0\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=0\r\n", "OK"))
		HAL_Delay(500);
}

//==========================================================
//	函数名称：	ESP8266_Commit
//
//	函数功能：	发送数据并解析服务器返回的json数据
//
//	入口参数：	char* url：GET + http地址
//
//	返回参数：	cJSON* 
//
//	说明：		
//==========================================================
//cJSON* ESP8266_Commit(char* url)
//{

//	const char* message = NULL;	
//	unsigned char timeOut = 200;
//	cJSON* cjson_root_tmp;
//	
////	sprintf(url, "GET https://api.seniverse.com/v3/weather/now.json?key=SjGPDBNAcF8v_L8Sh&location=guangzhou&language=zh-Hans&unit=c\r\n\r\n");
//		//sprintf(url, "GET http://quan.suning.com/getSysTime.do\r\n\r\n");
//	printf("url:%s",url);
//	ESP8266_Clear();
//	Usart_SendString(USART2, (unsigned char*)url, strlen((const char*)url));
//	
//	while(timeOut--)
//	{
//		if(ESP8266_WaitRecive() == REV_OK)							//如果收到数据
//		{
////			if(strstr((const char *)esp8266_buf, "{\"results\"") != NULL)		//如果检索到关键词
////			{
////				//printf("buf:%s\r\n",(const char *)esp8266_buf);
////				
////				//ESP8266_Clear();									//清空缓存
////			}
//			message = (const char *)esp8266_buf;
//			//printf("BUF1:%s",(const char *)esp8266_buf);
//		}
//		
//		HAL_Delay(10);
//	}		
//	
//	
//	printf("message:%s",message);
//		
//	cjson_root_tmp = cJSON_Parse(message);
//	//printf("BUF2:%s",(const char *)esp8266_buf);
//	
//	if(cjson_root_tmp == 0)
//	{	
//		printf("cjson_root_tmp = 0");
//	}
//	
//	ESP8266_Clear();									//清空缓存
//	return cjson_root_tmp;			// 返回json数据的根头
//}

unsigned char* ESP8266_Receive(unsigned short timeOut)
{
//	unsigned char timeOut = 3;
	char *ptrIPD = NULL;
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "{");				//搜索“{”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
				printf("error\r\n");
				return NULL;
			}
			else
			{
				return (unsigned char *)(ptrIPD);
			}
		}
		
		HAL_Delay(5);													//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针
}






/* 串口2中断回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)	/* 判断是否为USART2 */
	{
		if(esp8266_cnt >= MAXBUFFERSIZE - 1) /* 溢出判断 */
		{
			esp8266_cnt = 0;
			memset(esp8266_buf, 0x00, sizeof(esp8266_buf));			
		}
		else
		{
			esp8266_buf[esp8266_cnt++] = aRxBuffer;		/* 接收数据转存 */
		}
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);   // 再次开启接收中断
	}
}








