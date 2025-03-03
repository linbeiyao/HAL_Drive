#include <string.h>
#include <stdio.h>
#include "as608.h"
#include "key.h"
#include "usart.h"
#include "uart_user.h"

#define AS608_DEBUG 0	/* DEBUG 调试 */

uint32_t AS608Addr = 0XFFFFFFFF; 			/* 默认芯片地址 */
uint8_t USART3_RX_BUF[USART3_MAX_RECV_LEN];	/* USART3接收缓冲 */
uint8_t USART3_RX_STA = 0;						/* 串口是否接收到数据 */

char str2[6] = {0};

/* 口令验证 */
uint8_t Get_Device_Code[10] ={0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};

/**
  * @brief  串口发送一个字节
  * @param	uint8_t data 数据
  * 
  * @retval 发送成功返回0，失败返回1
  */
static uint8_t UART_SendData(uint8_t data)
{
	/* 阻塞发送 */
	if(HAL_UART_Transmit(&AS608_UART, &data, 1, 0xff) == HAL_OK)
		return 0;
	return 1;
}

/**
  * @brief  串口发送包头
  * @param	void
  * 
  * @retval void
  */
static void SendHead(void)
{
	/* 发送包头前清空数据 */
	memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));
	UART_SendData(0xEF);
	UART_SendData(0x01);
}

/**
  * @brief  串口发送芯片地址
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
  * @brief  串口发送包标识
  * @param	uint8_t flag 包标识 
			01：命令包
			02：数据包
			03：结束包
  * 
  * @retval void
  */
static void SendFlag(uint8_t flag)
{
	UART_SendData(flag);
}

/**
  * @brief  串口发送包长度
  * @param	int length 包长度
  * 
  * @retval void
  */
static void SendLength(short length)
{
	UART_SendData(length >> 8);
	UART_SendData(length);
}

/**
  * @brief  串口发送指令码
  * @param	uint8_t cmd 指令
  * 
  * @retval void
  */
static void SendCmd(uint8_t cmd)
{
	UART_SendData(cmd);
}

/**
  * @brief  串口发送校验和
  * @param	uint8_t cmd 指令
  * 
  * @retval void
  */
static void SendCheck(uint16_t check)
{
	UART_SendData(check >> 8);
	UART_SendData(check);
}

/**
  * @brief  检查模块是否连接
  * @param	void
  * 
  * @retval 模块连接返回0，失败返回1
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
	// 修改必须放在包头下面，因为发送包头会清空
	USART3_RX_BUF[9] = 1;
	HAL_UART_Receive(&AS608_UART, USART3_RX_BUF, 12, 200);	//串口三阻塞接收12个数据
	
#if AS608_DEBUG
	printf("AS608_Check:RX_BUF:%s\n", USART3_RX_BUF);
#endif
	
	//HAL_Delay(200);		// 等待200ms
	if(USART3_RX_BUF[9] == 0)
		return 0;
 
  return 1;
}

/**
  * @brief  指纹模块AS608初始化
  * @param	void
  * 
  * @retval 成功返回0，失败返回1
  */
uint8_t AS608_Init(void)
{
//	// 设置uart3接收中断
//	HAL_UART_Receive_IT(&AS608_UART, USART3_RX_BUF, sizeof(USART3_RX_BUF));		// 接收数据，且产生中断
//	// 使能空闲中断
//	__HAL_UART_ENABLE_IT(&AS608_UART, UART_IT_IDLE);
	
	return AS608_Check();
}

/** 
  * @brief  判断中断接收的数组有没有应答包
  * @param	uint16_t waittime 等待中断接收数据的时间(ms)
  * 
  * @retval 数据包首地址
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
	
	/* 阻塞接收 */
	HAL_UART_Receive(&AS608_UART, (uint8_t *)USART3_RX_BUF, USART3_MAX_RECV_LEN, waitTime);
	
#if AS608_DEBUG
	printf("JudgeStr:RX_BUF:%s\n", USART3_RX_BUF);
#endif
	
	if(!memcmp(str, USART3_RX_BUF, 7))
	{
		/* 在USART3_RX_BUF中搜索str，返回在USART3_RX_BUF中的str开头首地址*/
		/* 例如：USART3_RX_BUF: abhelloabcd str: hello */
		/* 则data: helloabcd */
		data = strstr((const char *)USART3_RX_BUF, (const char *)str);
		if(data)
			return (uint8_t*)data;	
	}
	
	/* 中断接收，需要修改中断服务函数，比较麻烦 */
//	USART3_RX_STA = 0;
//	while(--waitTime)
//	{
//		HAL_Delay(1);
//		if(USART3_RX_STA) 	// 接收到一次数据
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
  * @brief  PS_GetImage 录入图像 
			探测手指，探测到后录入指纹图像存于ImageBuffer
  * @param	void
  * 
  * @retval 模块返回确认字
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
  * @brief  生成特征 PS_GenChar
			将 ImageBuffer 中的原始图像生成指纹特征文件存于 CharBuffer1 或 CharBuffer2
  * @param	BufferID(特征缓冲区号)
  * 
  * @retval 模块返回确认字
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
  * @brief  精确比对两枚指纹特征 PS_Match精确比对CharBuffer1 与CharBuffer2 中的特征文件  
  * @param	BufferID(特征缓冲区号)
  * 
  * @retval 模块返回确认字 比对得分
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
  * @brief  搜索指纹 PS_Search 以CharBuffer1 或CharBuffer2 中的特征文件搜索整个
			或部分指纹库。若搜索到，则返回页码。
  * @param	BufferID 缓冲区号
  * @param	StartPage  
  * @param	PageNum
  * 
  * @retval 模块返回确认字 页码（相配指纹模板）
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
  * @brief  合并特征（生成模板）PS_RegModel 将CharBuffer1 与CharBuffer2 中的特征文件合并生成
			模板，结果存于CharBuffer1 与CharBuffer2。
  * @param	void
  * 
  * @retval 模块返回确认字
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
  * @brief  储存模板 PS_StoreChar 将 CharBuffer1 或 CharBuffer2 
			中的模板文件存到 PageID 号flash数据库位置。
  * @param	BufferID
  * @param	PageID
  * 
  * @retval 模块返回确认字
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
  * @brief  删除模板 PS_DeletChar 
			删除flash数据库中指定ID号开始的N个指纹模板。
  * @param	PageID 指纹库模板号
  * @param	N 删除的模板个数
  * 
  * @retval 模块返回确认字
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
  * @brief  清空指纹库 PS_Empty 
			删除flash数据库中所有指纹模板
  * @param	void
  * 
  * @retval 模块返回确认字
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
  * @brief  写系统寄存器 PS_WriteRegr 
			写模块寄存器
  * @param	RegNum 寄存器序号 4/5/6
  * @param	DATA 删除的模板个数
  * 
  * @retval 模块返回确认字
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
		printf("\r\n设置参数成功！");
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  读系统基本参数 PS_ReadSysPara
			读取模块的基本参数（波特率，包大小等)
  * @param	void
  * 
  * @retval 模块返回确认字 + 基本参数（16bytes）
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
		printf("\r\n模块最大指纹容量=%d", p->PS_max);
		printf("\r\n对比等级=%d", p->PS_level);
		printf("\r\n地址=%x", p->PS_addr);
		printf("\r\n波特率=%d", p->PS_N * 9600);
	}
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  设置模块地址 PS_SetAddr
			设置模块地址
  * @param	PS_addr
  * 
  * @retval 模块返回确认字
  */
uint8_t PS_SetAddr(uint32_t PS_addr)
{
	uint16_t temp;
	uint8_t  ensure;
	uint8_t  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
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
	AS608Addr = PS_addr; //发送完指令，更换地址
	data = JudgeStr(2000);
	if(data)
		ensure = data[9];
	else
		ensure = 0xff;
	AS608Addr = PS_addr;
	if(ensure == 0x00)
		printf("\r\n设置地址成功！");
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  模块内部为用户开辟了256bytes的FLASH空间用于存用户记事本,
			该记事本逻辑上被分成 16 个页
  * @param	NotePageNum(0~15)
  * @param	Byte32(要写入内容，32个字节)
  * 
  * @retval 模块返回确认字
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
  * @brief  读记事 PS_ReadNotepad
			读取FLASH用户区的128bytes数据
  * @param	NotePageNum(0~15)
  * 
  * @retval 模块返回确认字 + 用户信息
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
  * @brief  高速搜索PS_HighSpeedSearch
		以 CharBuffer1或CharBuffer2中的特征文件高速搜索整个或部分指纹库。
		若搜索到，则返回页码,该指令对于的确存在于指纹库中，且登录时质量
		很好的指纹，会很快给出搜索结果。
  * @param	BufferID
  * @param	StartPage(起始页)
  * @param	PageNum（页数）
  * 
  * @retval 模块返回确认字 + 用户信息
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
  * @brief  读有效模板个数 PS_ValidTempleteNum
			读有效模板个数
  * @param	void
  * 
  * @retval 模块返回确认字 + 有效模板个数ValidN
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
		printf("\r\n有效指纹个数=%d", (data[10] << 8) + data[11]);
	}
	else
		printf("\r\n%s", EnsureMessage(ensure));
	
	return ensure;
}

/** 
  * @brief  与AS608握手 PS_HandShake
			模块返新地址（正确地址）
  * @param	PS_Addr 地址指针
  * 
  * @retval 模块返回确认字
  */
uint8_t PS_HandShake(uint32_t *PS_Addr)
{
	SendHead();
	SendAddr();
	UART_SendData(0X01);
	UART_SendData(0X00);
	UART_SendData(0X00);
	HAL_Delay(200);
	if(USART3_RX_STA & 0X8000) //接收到数据
	{
		//判断是不是模块返回的应答包
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
  * @brief  模块应答包确认码信息解析
			解析确认码错误信息返回信息
  * @param	PS_Addr 地址指针
  * 
  * @retval 模块返回确认字 
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
			p = " 数据包接收错误 ";
			break;
		case  0x02:
			p = "传感器上没有手指";
			break;
		case  0x03:
			p = "录入指纹图像失败";
			break;
		case  0x04:
			p = " 指纹太干或太淡 ";
			break;
		case  0x05:
			p = " 指纹太湿或太糊 ";
			break;
		case  0x06:
			p = "  指纹图像太乱  ";
			break;
		case  0x07:
			p = " 指纹特征点太少 ";
			break;
		case  0x08:
			p = "  指纹不匹配    ";
			break;
		case  0x09:
			p = " 没有搜索到指纹 ";
			break;
		case  0x0a:
			p = "   特征合并失败 ";
			break;
		case  0x0b:
			p = "地址序号超出范围";
			break;
		case  0x10:
			p = "  删除模板失败  ";
			break;
		case  0x11:
			p = " 清空指纹库失败 ";
			break;
		case  0x15:
			p = "缓冲区内无有效图";
			break;
		case  0x18:
			p = " 读写FLASH出错  ";
			break;
		case  0x19:
			p = "   未定义错误   ";
			break;
		case  0x1a:
			p = "  无效寄存器号  ";
			break;
		case  0x1b:
			p = " 寄存器内容错误 ";
			break;
		case  0x1c:
			p = " 记事本页码错误 ";
			break;
		case  0x1f:
			p = "    指纹库满    ";
			break;
		case  0x20:
			p = "    地址错误    ";
			break;
		default :
			p = " 返回确认码有误 ";
			break;
	}
	return p;
}
 
/** 
  * @brief  显示确认码错误信息
  * @param	ensure q确认码
  * 
  * @retval void
  */
void ShowErrMessage(uint8_t ensure)
{
	printf("%s\r\n",EnsureMessage(ensure));
}
 
/** 
  * @brief  录指纹
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
				printf("请按手指\r\n");
				ensure = PS_GetImage();
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer1); // 生成特征
					if(ensure == 0x00)
					{	
						printf("指纹正常\r\n");
						i = 0;
						processnum = 1; // 跳到第二步
					}
					else 
						ShowErrMessage(ensure);
				}
				else 
					ShowErrMessage(ensure);
				break;
			case 1:
				i++;
				printf("请再按一次\r\n");
				ensure = PS_GetImage();
				if(ensure == 0x00)
				{
					ensure = PS_GenChar(CharBuffer2); //生成特征
					if(ensure == 0x00)
					{
						printf("指纹正常\r\n");
						i = 0;
						processnum = 2; //跳到第三步
					}
					else 
						ShowErrMessage(ensure);
				}
				else 
					 ShowErrMessage(ensure);
				break;
			case 2:
				printf("对比两次指纹\r\n");
				ensure = PS_Match();
				if(ensure == 0x00)
				{
					printf("对比成功\r\n");
					processnum = 3; //跳到第四步
				}
				else
				{
					printf("对比失败\r\n");
					ShowErrMessage(ensure);
					i = 0;
					processnum = 0; //跳回第一步
				}
				HAL_Delay(500);
				break;

				case 3:
					printf("生成指纹模板\r\n");
					HAL_Delay(500);
					ensure = PS_RegModel();
					if(ensure == 0x00)
					{
						printf("生成指纹模板成功\r\n");
						processnum = 4; //跳到第五步
					}
					else
					{
						processnum = 0;
						ShowErrMessage(ensure);
					}
					HAL_Delay(1000);
					break;

				case 4:
					printf("默认选择ID为1 \r\n");
					ID_NUM = 1;
					
		  ensure = PS_StoreChar(CharBuffer2, ID_NUM); //储存模板
		  if(ensure == 0x00)
		  {
				printf("录入指纹成功\r\n");
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
	if(i == 10) //超过5次没有按手指则退出
	{
	  break;
	}
	}
}
 
/** 
  * @brief  刷指纹
  * @param	void
  * 
  * @retval void
  */
SysPara AS608Para;//指纹模块AS608参数
void press_FR(void)
{
  SearchResult seach;
  uint8_t ensure;
  char str[20];
  while(1)
  {
    ensure = PS_GetImage();
    if(ensure == 0x00) //获取图像成功
    {
		printf("获取图像成功\n");
		ensure = PS_GenChar(CharBuffer1);
		if(ensure == 0x00) //生成特征成功
		{
			printf("生成特征成功\n");
			ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
			if(ensure == 0x00) //搜索成功
			{
				printf("指纹验证成功");
				sprintf(str, " ID:%d 得分:%d ", seach.pageID, seach.mathscore);
				printf("%s\r\n",str);
				HAL_Delay(1500);
				HAL_Delay(1500);
			}
			else
			{
				printf("验证失败\r\n");
				HAL_Delay(1500);
			}
      }
      else	{};
			printf("请按手指\r\n");
    }	
	else
		printf("获取图像失败\n");	
  }
}
 
/** 
  * @brief  删除单个指纹
  * @param	void
  * 
  * @retval void
  */
void Del_FR(void)
{
	uint8_t  ensure;
	uint16_t ID_NUM = 0;
	printf("单个删除指纹开始，默认删除ID为1");
	ID_NUM = 1;
	ensure = PS_DeletChar(ID_NUM, 1); //删除单个指纹
	if(ensure == 0)
	{
		printf("删除指纹成功 \r\n");
	}
	else
		ShowErrMessage(ensure);
	HAL_Delay(1500); 
}

/** 
  * @brief  清空指纹库
  * @param	void
  * 
  * @retval void
  */
void Del_FR_Lib(void)
{
	uint8_t  ensure;
	printf("删除指纹库开始\r\n");
	ensure = PS_Empty(); //清空指纹库
	if(ensure == 0)
	{
		printf("清空指纹库成功\r\n");
	}
	else
		ShowErrMessage(ensure);
	HAL_Delay(1500);
}
