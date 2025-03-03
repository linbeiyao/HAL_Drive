#include "main.h"
#include "esp8266.h"
#include "usart.h"
#include "uart_user.h"

#include <string.h>
#include <stdio.h>
//#include "cJSON.h"

/* WIFI���붨�� */
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Channel_2.4G\",\"password\"\r\n"
#define MAXBUFFERSIZE 1024

uint8_t aRxBuffer;                            // �����жϻ��� ÿ�ν���һ���ֽ�
unsigned char esp8266_buf[MAXBUFFERSIZE];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_Clear(void)
{
	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;
		
	if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0;							//��0���ռ���
			
		return REV_OK;								//���ؽ�����ɱ�־
	}
		
	esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ
	
	return REV_WAIT;								//���ؽ���δ��ɱ�־
}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����		
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	
	unsigned char timeOut = 200;

	Usart_SendString(&huart2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeOut--)
	{
		if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
		{
			if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
			{
				//printf("buf:%s\r\n",(const char *)esp8266_buf);
				ESP8266_Clear();									//��ջ���
				return 0;
			}
		}
		HAL_Delay(10);
	}
	
	return 1;
}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[256];
	
	ESP8266_Clear();								//��ս��ջ���
	
	//�ȷ���Ҫ�������ݵ�ָ����׼��
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������
	if(!ESP8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
	{
		//��Ȼ׼����ϼ��ɿ�ʼ��������
		Usart_SendString(&huart2, data, len);		//�����豸������������
	}

}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;
	
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//����������
		{
			ptrIPD = strstr((char *)esp8266_buf, "{");				//������{��ͷ
			if(ptrIPD == NULL)											//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
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
		
		HAL_Delay(5);													//��ʱ�ȴ�
	} while(timeOut--);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��

}

//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================

void ESP8266_Init(void)
{
	HAL_UART_MspInit(&huart2);
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);   //���������ж�
	
	ESP8266_Clear();	// ������ջ�����
	
	/* ����ATָ�� */
	printf("1. AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"))
		HAL_Delay(500);
	
	/* ���� */
	printf("2. RST\r\n");
	while(ESP8266_SendCmd("AT+RST\r\n", "OK"))
		HAL_Delay(500);
	
	/* ����stationģʽ */
	printf("3. CWMODE\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(500);
	
	/* ����DHCP�����Զ���ȡip */
	printf("4. AT+CWDHCP\r\n");
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(500);
	
	/* ����WIFI */
	printf("5. CWJAP\r\n");
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "WIFI GOT IP"))
		HAL_Delay(500);
	
	printf("6. ESP8266 Init OK\r\n");
	
//	while(ESP8266_SendCmd(MQTTUSERCFG, "OK"))
//		HAL_Delay(500);
}


//==========================================================
//	�������ƣ�	RstPassThrough
//
//	�������ܣ�	����͸��
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void SetPassThrough(void)
{
	ESP8266_Clear();	// ��ս����ڴ�
	
	/* ����͸��ģʽ */
	printf( "AT+CIPMODE=1\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"))
		HAL_Delay(500);
	
	/* ͸��ģʽ�´������� */
	printf( "AT+CIPSEND\r\n");
	while(ESP8266_SendCmd("AT+CIPSEND\r\n", ">"))
		HAL_Delay(500);
}
//==========================================================
//	�������ƣ�	RstPassThrough
//
//	�������ܣ�	�ر�͸��
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void RstPassThrough(void)
{
	ESP8266_Clear();	// ��ս����ڴ�
	
	/* �˳�͸��ģʽû��/r/n */
	Usart_SendString(&huart2, (unsigned char *)"+++", strlen("+++"));
	HAL_Delay(100);
	printf("11.AT+CIPSEND=0\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=0\r\n", "OK"))
		HAL_Delay(500);
}

//==========================================================
//	�������ƣ�	ESP8266_Commit
//
//	�������ܣ�	�������ݲ��������������ص�json����
//
//	��ڲ�����	char* url��GET + http��ַ
//
//	���ز�����	cJSON* 
//
//	˵����		
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
//		if(ESP8266_WaitRecive() == REV_OK)							//����յ�����
//		{
////			if(strstr((const char *)esp8266_buf, "{\"results\"") != NULL)		//����������ؼ���
////			{
////				//printf("buf:%s\r\n",(const char *)esp8266_buf);
////				
////				//ESP8266_Clear();									//��ջ���
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
//	ESP8266_Clear();									//��ջ���
//	return cjson_root_tmp;			// ����json���ݵĸ�ͷ
//}

unsigned char* ESP8266_Receive(unsigned short timeOut)
{
//	unsigned char timeOut = 3;
	char *ptrIPD = NULL;
	do
	{
		if(ESP8266_WaitRecive() == REV_OK)								//����������
		{
			ptrIPD = strstr((char *)esp8266_buf, "{");				//������{��ͷ
			if(ptrIPD == NULL)											//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
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
		
		HAL_Delay(5);													//��ʱ�ȴ�
	} while(timeOut--);
	
	return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��
}






/* ����2�жϻص����� */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)	/* �ж��Ƿ�ΪUSART2 */
	{
		if(esp8266_cnt >= MAXBUFFERSIZE - 1) /* ����ж� */
		{
			esp8266_cnt = 0;
			memset(esp8266_buf, 0x00, sizeof(esp8266_buf));			
		}
		else
		{
			esp8266_buf[esp8266_cnt++] = aRxBuffer;		/* ��������ת�� */
		}
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&aRxBuffer, 1);   // �ٴο��������ж�
	}
}








