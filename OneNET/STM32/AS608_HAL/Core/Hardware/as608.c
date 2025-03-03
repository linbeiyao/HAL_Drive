#include <string.h>
#include <stdio.h>
#include "as608.h"
#include "key.h"
#include "usart.h"
#include "uart_user.h"

#define AS608_DEBUG 0	/* DEBUG ���� */

uint32_t AS608Addr = 0XFFFFFFFF; 			/* Ĭ��оƬ��ַ */
uint8_t USART3_RX_BUF[USART3_MAX_RECV_LEN];	/* USART3���ջ��� */
uint8_t USART3_RX_STA = 0;						/* �����Ƿ���յ����� */

char str2[6] = {0};

/* ������֤ */
uint8_t Get_Device_Code[10] ={0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};

/**
  * @brief  ���ڷ���һ���ֽ�
  * @param	uint8_t data ����
  * 
  * @retval ���ͳɹ�����0��ʧ�ܷ���1
  */
static uint8_t UART_SendData(uint8_t data)
{
	/* �������� */
	if(HAL_UART_Transmit(&AS608_UART, &data, 1, 0xff) == HAL_OK)
		return 0;
	return 1;
}

/**
  * @brief  ���ڷ��Ͱ�ͷ
  * @param	void
  * 
  * @retval void
  */
static void SendHead(void)
{
	/* ���Ͱ�ͷǰ������� */
	memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));
	UART_SendData(0xEF);
	UART_SendData(0x01);
}

/**
  * @brief  ���ڷ���оƬ��ַ
  * @param	void
  * 
  * @retval void
  */
static void SendAddr(void)
{
	UART_SendData(AS608Addr >> 24);
	UART_SendData(AS608Addr >> 16);
	UART_SendData(AS608Addr >> 8);
	UART_SendData(AS608Addr);
}

/**
  * @brief  ���ڷ��Ͱ���ʶ
  * @param	uint8_t flag ����ʶ 
			01�������
			02�����ݰ�
			03��������
  * 
  * @retval void
  */
static void SendFlag(uint8_t flag)
{
	UART_SendData(flag);
}

/**
  * @brief  ���ڷ��Ͱ�����
  * @param	int length ������
  * 
  * @retval void
  */
static void SendLength(short length)
{
	UART_SendData(length >> 8);
	UART_SendData(length);
}

/**
  * @brief  ���ڷ���ָ����
  * @param	uint8_t cmd ָ��
  * 
  * @retval void
  */
static void SendCmd(uint8_t cmd)
{
	UART_SendData(cmd);
}

/**
  * @brief  ���ڷ���У���
  * @param	uint8_t cmd ָ��
  * 
  * @retval void
  */
static void SendCheck(uint16_t check)
{
	UART_SendData(check >> 8);
	UART_SendData(check);
}

/**
  * @brief  ���ģ���Ƿ�����
  * @param	void
  * 
  * @retval ģ�����ӷ���0��ʧ�ܷ���1
  */
static uint8_t AS608_Check(void)
{
	SendHead();
	SendAddr();
	for(int i = 0; i < 10; i++)
	{	
#if AS608_DEBUG
	if(!UART_SendData(Get_Device_Code[i]))
			printf("%d,ok\n", i);
#else
	UART_SendData(Get_Device_Code[i]);
#endif	
	}
	// �޸ı�����ڰ�ͷ���棬��Ϊ���Ͱ�ͷ�����
	USART3_RX_BUF[9] = 1;
	HAL_UART_Receive(&AS608_UART, USART3_RX_BUF, 12, 200);	//��������������12������
	
#if AS608_DEBUG
	printf("AS608_Check:RX_BUF:%s\n", USART3_RX_BUF);
#endif
	
	//HAL_Delay(200);		// �ȴ�200ms
	if(USART3_RX_BUF[9] == 0)
		return 0;
 
  return 1;
}

/**
  * @brief  ָ��ģ��AS608��ʼ��
  * @param	void
  * 
  * @retval �ɹ�����0��ʧ�ܷ���1
  */
uint8_t AS608_Init(void)
{
//	// ����uart3�����ж�
//	HAL_UART_Receive_IT(&AS608_UART, USART3_RX_BUF, sizeof(USART3_RX_BUF));		// �������ݣ��Ҳ����ж�
//	// ʹ�ܿ����ж�
//	__HAL_UART_ENABLE_IT(&AS608_UART, UART_IT_IDLE);
	
	return AS608_Check();
}

/** 
  * @brief  �ж��жϽ��յ�������û��Ӧ���
  * @param	uint16_t waittime �ȴ��жϽ������ݵ�ʱ��(ms)
  * 
  * @retval ���ݰ��׵�ַ
  */
static uint8_t *JudgeStr(uint16_t waitTime)
{
	char *data;
	uint8_t str[8];
	str[0] = 0xEF;
	str[1] = 0x01;
	str[2] = AS608Addr >> 24;
	str[3] = AS608Addr >> 16;
	str[4] = AS608Addr >> 8;
	str[5] = AS608Addr;
	str[6] = 0x07;
	str[7] = '\0';
	
	/* �������� */
	HAL_UART_Receive(&AS608_UART, (uint8_t *)USART3_RX_BUF, USART3_MAX_RECV_LEN, waitTime);
	
#if AS608_DEBUG
	printf("JudgeStr:RX_BUF:%s\n", USART3_RX_BUF);
#endif
	
	if(!memcmp(str, USART3_RX_BUF, 7))
	{
		/* ��USART3_RX_BUF������str��������USART3_RX_BUF�е�str��ͷ�׵�ַ*/
		/* ���磺USART3_RX_BUF: abhelloabcd str: hello */
		/* ��data: helloabcd */
		data = strstr((const char *)USART3_RX_BUF, (const char *)str);
		if(data)
			return (uint8_t*)data;	
	}
	
	/* �жϽ��գ���Ҫ�޸��жϷ��������Ƚ��鷳 */
//	USART3_RX_STA = 0;
//	while(--waitTime)
//	{
//		HAL_Delay(1);
//		if(USART3_RX_STA) 	// ���յ�һ������
//		{
//			printf("RXSTA\n");
//			USART3_RX_STA = 0;
//			data = strstr((const char *)USART3_RX_BUF, (const char *)str);
//			if(data)
//				return (uint8_t*)data;
//		}
//	}
//	printf("errops\n");
	return 0;
}

/** 
  * @brief  PS_GetImage ¼��ͼ�� 
			̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer
  * @param	void
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_GetImage(void)
{
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x03);
	SendCmd(0x01);
	SendCheck(0x05);
	
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  �������� PS_GenChar
			�� ImageBuffer �е�ԭʼͼ������ָ�������ļ����� CharBuffer1 �� CharBuffer2
  * @param	BufferID(������������)
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_GenChar(uint8_t BufferID)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x04);
	SendCmd(0x02);
	UART_SendData(BufferID);
	tmp = 0x01 + 0x04 + 0x02 + BufferID;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ��ȷ�ȶ���öָ������ PS_Match��ȷ�ȶ�CharBuffer1 ��CharBuffer2 �е������ļ�  
  * @param	BufferID(������������)
  * 
  * @retval ģ�鷵��ȷ���� �ȶԵ÷�
  */
uint8_t PS_Match(void)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x03);
	SendCmd(0x03);
	tmp = 0x01 + 0x03 + 0x03;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ����ָ�� PS_Search ��CharBuffer1 ��CharBuffer2 �е������ļ���������
			�򲿷�ָ�ƿ⡣�����������򷵻�ҳ�롣
  * @param	BufferID ��������
  * @param	StartPage  
  * @param	PageNum
  * 
  * @retval ģ�鷵��ȷ���� ҳ�루����ָ��ģ�壩
  */
uint8_t PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x08);
	SendCmd(0x04);
	
	UART_SendData(BufferID);
	UART_SendData(StartPage >> 8);
	UART_SendData(StartPage);
	UART_SendData(PageNum >> 8);
	UART_SendData(PageNum);

	tmp = 0x01 + 0x08 + 0x04 + BufferID + 
		  + (StartPage >> 8) + (uint8_t)StartPage
		  + (PageNum >> 8) + (uint8_t)PageNum;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
	{
		ensure = data[9];	
		p->pageID   = (data[10] << 8) + data[11];
		p->mathscore = (data[12] << 8) + data[13];
	}
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  �ϲ�����������ģ�壩PS_RegModel ��CharBuffer1 ��CharBuffer2 �е������ļ��ϲ�����
			ģ�壬�������CharBuffer1 ��CharBuffer2��
  * @param	void
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_RegModel(void)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x03);
	SendCmd(0x05);
	tmp = 0x01 + 0x03 + 0x05;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ����ģ�� PS_StoreChar �� CharBuffer1 �� CharBuffer2 
			�е�ģ���ļ��浽 PageID ��flash���ݿ�λ�á�
  * @param	BufferID
  * @param	PageID
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_StoreChar(uint8_t BufferID, uint16_t PageID)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x06);
	SendCmd(0x06);
	UART_SendData(BufferID);
	UART_SendData(PageID >> 8);
	UART_SendData(PageID);
	tmp = 0x01 + 0x06 + 0x06 + BufferID
		  + (PageID >> 8) + (uint8_t)PageID;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ɾ��ģ�� PS_DeletChar 
			ɾ��flash���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ�塣
  * @param	PageID ָ�ƿ�ģ���
  * @param	N ɾ����ģ�����
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_DeletChar(uint16_t PageID, uint16_t N)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x07);
	SendCmd(0x0C);
	UART_SendData(PageID >> 8);
	UART_SendData(PageID);
	UART_SendData(N >> 8);
	UART_SendData(N);	
	tmp = 0x01 + 0x07 + 0x0C
		  + (PageID >> 8) + (uint8_t)PageID
		  + (N >> 8) + (uint8_t)N;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ���ָ�ƿ� PS_Empty 
			ɾ��flash���ݿ�������ָ��ģ��
  * @param	void
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_Empty(void)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x03);
	SendCmd(0x0D);	
	tmp = 0x01 + 0x03 + 0x0D;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  дϵͳ�Ĵ��� PS_WriteRegr 
			дģ��Ĵ���
  * @param	RegNum �Ĵ������ 4/5/6
  * @param	DATA ɾ����ģ�����
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_WriteReg(uint8_t RegNum, uint8_t DATA)
{
	uint16_t tmp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x05);
	SendCmd(0x0E);
	UART_SendData(RegNum);
	UART_SendData(DATA);

	tmp = 0x01 + 0x07 + 0x0C + RegNum + DATA;
	SendCheck(tmp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	if(ensure == 0)
		printf("\r\n���ò����ɹ���");
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  ��ϵͳ�������� PS_ReadSysPara
			��ȡģ��Ļ��������������ʣ�����С��)
  * @param	void
  * 
  * @retval ģ�鷵��ȷ���� + ����������16bytes��
  */
uint8_t PS_ReadSysPara(SysPara *p)
{
	uint16_t temp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x03);
	SendCmd(0x0F);
	temp = 0x01 + 0x03 + 0x0F;
	SendCheck(temp);
	data = JudgeStr(1000);
	if(data)
	{
		ensure = data[9];
		p->PS_max = (data[14] << 8) + data[15];
		p->PS_level = data[17];
		p->PS_addr = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
		p->PS_size = data[23];
		p->PS_N = data[25];
	}
	else
		ensure = 0xff;
	if(ensure == 0x00)
	{
		printf("\r\nģ�����ָ������=%d", p->PS_max);
		printf("\r\n�Աȵȼ�=%d", p->PS_level);
		printf("\r\n��ַ=%x", p->PS_addr);
		printf("\r\n������=%d", p->PS_N * 9600);
	}
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  ����ģ���ַ PS_SetAddr
			����ģ���ַ
  * @param	PS_addr
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_SetAddr(uint32_t PS_addr)
{
	uint16_t temp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
	SendLength(0x07);
	SendCmd(0x15);
	UART_SendData(PS_addr >> 24);
	UART_SendData(PS_addr >> 16);
	UART_SendData(PS_addr >> 8);
	UART_SendData(PS_addr);
	temp = 0x01 + 0x07 + 0x15
		 + (uint8_t)(PS_addr >> 24) + (uint8_t)(PS_addr >> 16)
		 + (uint8_t)(PS_addr >> 8) + (uint8_t)PS_addr;
	SendCheck(temp);
	AS608Addr = PS_addr; //������ָ�������ַ
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	AS608Addr = PS_addr;
	if(ensure == 0x00)
		printf("\r\n���õ�ַ�ɹ���");
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  ģ���ڲ�Ϊ�û�������256bytes��FLASH�ռ����ڴ��û����±�,
			�ü��±��߼��ϱ��ֳ� 16 ��ҳ
  * @param	NotePageNum(0~15)
  * @param	Byte32(Ҫд�����ݣ�32���ֽ�)
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_WriteNotepad(uint8_t NotePageNum, uint8_t *Byte32)
{
	uint16_t temp;
	uint8_t  ensure, i;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(36);
	SendCmd(0x18);
	UART_SendData(NotePageNum);
	for(i = 0; i < 32; i++)
	{
		UART_SendData(Byte32[i]);
		temp += Byte32[i];
	}
	temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ������ PS_ReadNotepad
			��ȡFLASH�û�����128bytes����
  * @param	NotePageNum(0~15)
  * 
  * @retval ģ�鷵��ȷ���� + �û���Ϣ
  */
uint8_t PS_ReadNotepad(uint8_t NotePageNum, uint8_t *Byte32)
{
	uint16_t temp;
	uint8_t  ensure, i;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x04);
	SendCmd(0x19);
	UART_SendData(NotePageNum);
	temp = 0x01 + 0x04 + 0x19 + NotePageNum;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		for(i = 0; i < 32; i++)
		{
			Byte32[i] = data[10 + i];
		}
	}
	else
		ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ��������PS_HighSpeedSearch
		�� CharBuffer1��CharBuffer2�е������ļ��������������򲿷�ָ�ƿ⡣
		�����������򷵻�ҳ��,��ָ����ڵ�ȷ������ָ�ƿ��У��ҵ�¼ʱ����
		�ܺõ�ָ�ƣ���ܿ�������������
  * @param	BufferID
  * @param	StartPage(��ʼҳ)
  * @param	PageNum��ҳ����
  * 
  * @retval ģ�鷵��ȷ���� + �û���Ϣ
  */
uint8_t PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
	uint16_t temp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x08);
	SendCmd(0x1b);
	UART_SendData(BufferID);
	UART_SendData(StartPage >> 8);
	UART_SendData(StartPage);
	UART_SendData(PageNum >> 8);
	UART_SendData(PageNum);
	temp = 0x01 + 0x08 + 0x1b + BufferID
		 + (StartPage >> 8) + (uint8_t)StartPage
		 + (PageNum >> 8) + (uint8_t)PageNum;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		p->pageID 	= (data[10] << 8) + data[11];
		p->mathscore = (data[12] << 8) + data[13];
	}
	else
	ensure = 0xff;
	
	return ensure;
}

/** 
  * @brief  ����Чģ����� PS_ValidTempleteNum
			����Чģ�����
  * @param	void
  * 
  * @retval ģ�鷵��ȷ���� + ��Чģ�����ValidN
  */
uint8_t PS_ValidTempleteNum(uint16_t *ValidN)
{
	uint16_t temp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);
	SendLength(0x03);
	SendCmd(0x1d);
	temp = 0x01 + 0x03 + 0x1d;
	SendCheck(temp);
	data = JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		*ValidN = (data[10] << 8) + data[11];
	}
	else
		ensure = 0xff;

	if(ensure == 0x00)
	{
		printf("\r\n��Чָ�Ƹ���=%d", (data[10] << 8) + data[11]);
	}
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  ��AS608���� PS_HandShake
			ģ�鷵�µ�ַ����ȷ��ַ��
  * @param	PS_Addr ��ַָ��
  * 
  * @retval ģ�鷵��ȷ����
  */
uint8_t PS_HandShake(uint32_t *PS_Addr)
{
	SendHead();
	SendAddr();
	UART_SendData(0X01);
	UART_SendData(0X00);
	UART_SendData(0X00);
	HAL_Delay(200);
	if(USART3_RX_STA & 0X8000) //���յ�����
	{
		//�ж��ǲ���ģ�鷵�ص�Ӧ���
		if(USART3_RX_BUF[0] == 0XEF
		   && USART3_RX_BUF[1] == 0X01
		   && USART3_RX_BUF[6] == 0X07)
		{
			*PS_Addr = (USART3_RX_BUF[2] << 24) + (USART3_RX_BUF[3] << 16)
					 + (USART3_RX_BUF[4] << 8) + (USART3_RX_BUF[5]);
			USART3_RX_STA = 0;
			return 0;
		}
		USART3_RX_STA = 0;
	}
	return 1;
}

/** 
  * @brief  ģ��Ӧ���ȷ������Ϣ����
			����ȷ���������Ϣ������Ϣ
  * @param	PS_Addr ��ַָ��
  * 
  * @retval ģ�鷵��ȷ���� 
  */
const char *EnsureMessage(uint8_t ensure)
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p = "       OK       ";
			break;
		case  0x01:
			p = " ���ݰ����մ��� ";
			break;
		case  0x02:
			p = "��������û����ָ";
			break;
		case  0x03:
			p = "¼��ָ��ͼ��ʧ��";
			break;
		case  0x04:
			p = " ָ��̫�ɻ�̫�� ";
			break;
		case  0x05:
			p = " ָ��̫ʪ��̫�� ";
			break;
		case  0x06:
			p = "  ָ��ͼ��̫��  ";
			break;
		case  0x07:
			p = " ָ��������̫�� ";
			break;
		case  0x08:
			p = "  ָ�Ʋ�ƥ��    ";
			break;
		case  0x09:
			p = " û��������ָ�� ";
			break;
		case  0x0a:
			p = "   �����ϲ�ʧ�� ";
			break;
		case  0x0b:
			p = "��ַ��ų�����Χ";
			break;
		case  0x10:
			p = "  ɾ��ģ��ʧ��  ";
			break;
		case  0x11:
			p = " ���ָ�ƿ�ʧ�� ";
			break;
		case  0x15:
			p = "������������Чͼ";
			break;
		case  0x18:
			p = " ��дFLASH����  ";
			break;
		case  0x19:
			p = "   δ�������   ";
			break;
		case  0x1a:
			p = "  ��Ч�Ĵ�����  ";
			break;
		case  0x1b:
			p = " �Ĵ������ݴ��� ";
			break;
		case  0x1c:
			p = " ���±�ҳ����� ";
			break;
		case  0x1f:
			p = "    ָ�ƿ���    ";
			break;
		case  0x20:
			p = "    ��ַ����    ";
			break;
		default :
			p = " ����ȷ�������� ";
			break;
	}
	return p;
}
 
/** 
  * @brief  ��ʾȷ���������Ϣ
  * @param	ensure qȷ����
  * 
  * @retval void
  */
void ShowErrMessage(uint8_t ensure)
{
	printf("%s\r\n",EnsureMessage(ensure));
}
 
/** 
  * @brief  ¼ָ��
  * @param	void
  * 
  * @retval void
  */
void Add_FR(void)
{
	uint8_t i, ensure, processnum = 0;
	uint8_t ID_NUM = 0;
	while(1)
	{
		switch(processnum)
		{
			case 0:
				i++;
				printf("�밴��ָ\r\n");
				ensure = PS_GetImage();
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer1); // ��������
					if(ensure == 0x00)
					{	
						printf("ָ������\r\n");
						i = 0;
						processnum = 1; // �����ڶ���
					}
					else 
						ShowErrMessage(ensure);
				}
				else 
					ShowErrMessage(ensure);
				break;
			case 1:
				i++;
				printf("���ٰ�һ��\r\n");
				ensure = PS_GetImage();
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer2); //��������
					if(ensure == 0x00)
					{
						printf("ָ������\r\n");
						i = 0;
						processnum = 2; //����������
					}
					else 
						ShowErrMessage(ensure);
				}
				else 
					 ShowErrMessage(ensure);
				break;
			case 2:
				printf("�Ա�����ָ��\r\n");
				ensure = PS_Match();
				if(ensure == 0x00)
				{
					printf("�Աȳɹ�\r\n");
					processnum = 3; //�������Ĳ�
				}
				else
				{
					printf("�Ա�ʧ��\r\n");
					ShowErrMessage(ensure);
					i = 0;
					processnum = 0; //���ص�һ��
				}
				HAL_Delay(500);
				break;

				case 3:
					printf("����ָ��ģ��\r\n");
					HAL_Delay(500);
					ensure = PS_RegModel();
					if(ensure == 0x00)
					{
						printf("����ָ��ģ��ɹ�\r\n");
						processnum = 4; //�������岽
					}
					else
					{
						processnum = 0;
						ShowErrMessage(ensure);
					}
					HAL_Delay(1000);
					break;

				case 4:
					printf("Ĭ��ѡ��IDΪ1 \r\n");
					ID_NUM = 1;
					
		  ensure = PS_StoreChar(CharBuffer2, ID_NUM); //����ģ��
		  if(ensure == 0x00)
		  {
				printf("¼��ָ�Ƴɹ�\r\n");
				HAL_Delay(1500);
				return ;
		  }
		  else
		  {
				processnum = 0;
				ShowErrMessage(ensure);
		  }
		  break;
		}
		
	HAL_Delay(400);
	if(i == 10) //����5��û�а���ָ���˳�
	{
	  break;
	}
	}
}
 
/** 
  * @brief  ˢָ��
  * @param	void
  * 
  * @retval void
  */
SysPara AS608Para;//ָ��ģ��AS608����
void press_FR(void)
{
  SearchResult seach;
  uint8_t ensure;
  char str[20];
  while(1)
  {
    ensure = PS_GetImage();
    if(ensure == 0x00) //��ȡͼ��ɹ�
    {
		printf("��ȡͼ��ɹ�\n");
		ensure = PS_GenChar(CharBuffer1);
		if(ensure == 0x00) //���������ɹ�
		{
			printf("���������ɹ�\n");
			ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
			if(ensure == 0x00) //�����ɹ�
			{
				printf("ָ����֤�ɹ�");
				sprintf(str, " ID:%d �÷�:%d ", seach.pageID, seach.mathscore);
				printf("%s\r\n",str);
				HAL_Delay(1500);
				HAL_Delay(1500);
			}
			else
			{
				printf("��֤ʧ��\r\n");
				HAL_Delay(1500);
			}
      }
      else	{};
			printf("�밴��ָ\r\n");
    }	
	else
		printf("��ȡͼ��ʧ��\n");	
  }
}
 
/** 
  * @brief  ɾ������ָ��
  * @param	void
  * 
  * @retval void
  */
void Del_FR(void)
{
	uint8_t  ensure;
	uint16_t ID_NUM = 0;
	printf("����ɾ��ָ�ƿ�ʼ��Ĭ��ɾ��IDΪ1");
	ID_NUM = 1;
	ensure = PS_DeletChar(ID_NUM, 1); //ɾ������ָ��
	if(ensure == 0)
	{
		printf("ɾ��ָ�Ƴɹ� \r\n");
	}
	else
		ShowErrMessage(ensure);
	HAL_Delay(1500); 
}

/** 
  * @brief  ���ָ�ƿ�
  * @param	void
  * 
  * @retval void
  */
void Del_FR_Lib(void)
{
	uint8_t  ensure;
	printf("ɾ��ָ�ƿ⿪ʼ\r\n");
	ensure = PS_Empty(); //���ָ�ƿ�
	if(ensure == 0)
	{
		printf("���ָ�ƿ�ɹ�\r\n");
	}
	else
		ShowErrMessage(ensure);
	HAL_Delay(1500);
}
