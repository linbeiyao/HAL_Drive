#include "esp8266.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "uart_user.h"
#include "stm32f1xx_hal.h"

/* 定义USART2，如果在stm32f1xx_hal.h中未定义 */
#ifndef USART2
#define USART2 ((USART_TypeDef *) USART2_BASE)
#endif

/* 全局变量定义 */
uint8_t aRxBuffer;                            // 接收中断缓存
unsigned char esp8266_buf[MAXBUFFERSIZE];     // ESP8266接收缓冲区
unsigned short esp8266_cnt = 0;               // 接收计数器
unsigned short esp8266_cntPre = 0;            // 上一次接收计数器的值

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
    if(esp8266_cnt == 0)                     // 如果接收计数为0，说明没有数据
        return REV_WAIT;
        
    if(esp8266_cnt == esp8266_cntPre)       // 如果这次的值和上次相同，说明接收完毕
    {
        esp8266_cnt = 0;                     // 清0接收计数
        return REV_OK;                       // 返回接收完成标志
    }
        
    esp8266_cntPre = esp8266_cnt;           // 置为相同
    return REV_WAIT;                         // 返回接收未完成标志
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

    // 直接使用HAL库函数发送
    HAL_UART_Transmit(&huart2, (unsigned char *)cmd, strlen((const char *)cmd), 1000);
    
    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)                      // 如果收到数据
        {
            if(strstr((const char *)esp8266_buf, res) != NULL)  // 如果检索到关键词
            {
                ESP8266_Clear();                                 // 清空缓存
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
    char cmdBuf[32];
    
    ESP8266_Clear();                                 // 清空接收缓存
    
    sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);      // 发送AT+CIPSEND命令
    if(!ESP8266_SendCmd(cmdBuf, ">"))               // 收到>时可以发送数据
    {
        // 直接使用HAL库函数发送
        HAL_UART_Transmit(&huart2, data, len, 1000); // 发送数据
    }
}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x为数据长度，yyy为数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
    char *ptrIPD = NULL;
    
    do
    {
        if(ESP8266_WaitRecive() == REV_OK)                      // 如果接收完成
        {
            ptrIPD = strstr((char *)esp8266_buf, "{");          // 搜索"{"头
            if(ptrIPD == NULL)                                  // 如果没有找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
            {
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
        
        HAL_Delay(5);                                          // 延时等待
    } while(timeOut--);
    
    return NULL;                                               // 超时还未找到，返回空指针
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
    // 确保串口2已经初始化
    HAL_UART_MspInit(&huart2);
    HAL_UART_Receive_IT(&huart2, &aRxBuffer, 1);   // 开启接收中断
    
    ESP8266_Clear();    // 清空接收缓存区
    
    printf("\r\n正在初始化ESP8266...\r\n");
		HAL_UART_Transmit(&huart2,(const uint8_t *)"ATE0\r\n",sizeof("ATE0\r\n"),1000);
    
    // 测试AT指令
    printf("1. 测试AT指令...\r\n");
    while(ESP8266_SendCmd("AT\r\n", "OK"))
    {
        HAL_Delay(500);
        printf("重试AT指令...\r\n");
    }
		
    while(ESP8266_SendCmd("ATE0\r\n", "OK"))
    {
        HAL_Delay(500);
    }
    
    // 重启模块
    printf("2. 重启ESP8266...\r\n");
    while(ESP8266_SendCmd("AT+RST\r\n", "OK"))
    {
        HAL_Delay(500);
        printf("重试重启...\r\n");
    }
    HAL_Delay(1000);    // 等待重启完成
    
    // 设置工作模式为station
    printf("3. 设置工作模式...\r\n");
    while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
    {
        HAL_Delay(500);
        printf("重试设置模式...\r\n");
    }
    
    // 设置DHCP
    printf("4. 设置DHCP...\r\n");
    while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
    {
        HAL_Delay(500);
        printf("重试设置DHCP...\r\n");
    }
    
    // 连接到WiFi
    printf("5. 连接WiFi...\r\n");
    while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "WIFI GOT IP"))
    {
        HAL_Delay(500);
        printf("重试连接WiFi...\r\n");
    }

    printf("ESP8266初始化完成！\r\n");
}

//==========================================================
//	函数名称：	ESP8266_UART_RxHandler
//	函数功能：	串口接收中断处理
//==========================================================
void ESP8266_UART_RxHandler(UART_HandleTypeDef *huart)
{
//		static char message_ring[5][512];
//		static uint8_t ring_index = 0;
	
//	
//		if(ring_index < 5){
//			memcpy(message_ring[ring_index] ,esp8266_buf,512);
//			ring_index++;
//		}
//		if(ring_index >= 5){
//			ring_index = 0;
//		}
	
    if(huart->Instance == USART2)
    {
        if(esp8266_cnt >= MAXBUFFERSIZE - 1)    // 防止缓冲区溢出
        {
            esp8266_cnt = 0;
            memset(esp8266_buf, 0, sizeof(esp8266_buf));
        }
        else
        {
            esp8266_buf[esp8266_cnt++] = aRxBuffer;   // 将接收到的数据存入缓冲区
					
					
						
					
						
					
					
        }
				
        
        // 重新使能接收中断
        HAL_UART_Receive_IT(huart, &aRxBuffer, 1);
    }
}


unsigned char* ESP8266_Receive(unsigned short timeOut)
{
//	unsigned char timeOut = 3;
	char *ptrIPD = NULL;
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "{");				//搜索"{"头
			if(ptrIPD == NULL)											//如果没有找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
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


//==========================================================
//	函数名称：	SetPassThrough
//
//	函数功能：	设置透传
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void SetPassThrough(void)
{
	ESP8266_Clear();	// 清空串口内存
	
	/* 设置透传模式 */
	printf( "AT+CIPMODE=1\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"))
		HAL_Delay(500);
	
	/* 透传模式下开始发送 */
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
	ESP8266_Clear();	// 清空串口内存
	
	/* 退出透传模式没有/r/n */
	Usart_SendString(&huart2, (unsigned char *)"+++", strlen("+++"));
	HAL_Delay(100);
	printf("11.AT+CIPSEND=0\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=0\r\n", "OK"))
		HAL_Delay(500);
}








