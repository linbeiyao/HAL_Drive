#include "RC522.h"
#include <stdio.h>

/*
	M1卡分为16个扇区，每个扇区由四个块（块0、块1、块2、块3）组成
	将16个扇区的64个块按绝对地址编号为：0~63
	第0个扇区的块0（即绝对地址0块），用于存放厂商代码，已经固化不可更改 
	每个扇区的块0、块1、块2为数据块，可用于存放数据
	每个扇区的块3为控制块（绝对地址为:块3、块7、块11.....）包括密码A，存取控制、密码B等
*/
/*******************************
	*连线说明：
	*1--片选SDA/CS/NSS  <----->PA4
	*2--SCK  <----->PA5
	*3--MOSI <----->PA7
	*4--MISO <----->PA6
	*5--IRQ 悬空
	*6--GND <----->GND
	*7--RST <----->PC5
	*8--VCC <----->VCC
************************************/

// 软件SPI暂时还没调试成功。
/* 选择SPI驱动方式,默认使用硬件SPI,不能两个都同时取消注释！*/
#define RC522_USE_HW_SPI		// 硬件SPI
//#define RC522_USE_SW_SPI		// 软件SPI

#ifdef RC522_USE_HW_SPI 
	extern SPI_HandleTypeDef hspi1;		// HAL库使用，指定SPI接口
#else RC522_USE_SW_SPI	   
	#define          RC522_SCK_0()             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)
	#define          RC522_SCK_1()             HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET)
	#define          RC522_MOSI_0()            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET)
	#define          RC522_MOSI_1()            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET)
	#define          RC522_MISO_GET()          HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)
#endif

/* RST */
#define RC522_RST_HIGH HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET)
#define RC522_RST_LOW HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET)

/* 片选SDA/CS/NSS */
#define RC522_ENABLE HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define RC522_DISABLE HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

#define fac_us 72   //时钟频率，单位MHZ

/* 全局变量 */
unsigned char CT[2];		//卡类型
unsigned char SN[4]; 		//卡号
unsigned char DATA[16];			//存放数据
unsigned char RFID[16];			//存放RFID
unsigned char status;
uint8_t KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t KEY_B[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
// 0x08 就是2扇区0区块（即第9块）
unsigned char addr=0x08;

// 显示卡的卡号，以十六进制显示
static void ShowID(uint8_t *p)
{
    uint8_t num[9];
    uint8_t i;

    for(i=0; i<4; i++)
    {
        num[i*2] = p[i] / 16;
        num[i*2] > 9 ? (num[i*2] += '7') : (num[i*2] += '0');
        num[i*2+1] = p[i] % 16;
        num[i*2+1] > 9 ? (num[i*2+1] += '7') : (num[i*2+1] += '0');
    }
    num[8] = 0;
    printf("ID>>>%s\r\n", num);
}


/*微秒级延时函数*/
void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt = 0;
	uint32_t reload = SysTick->LOAD;			//LOAD的值
	ticks = nus * fac_us; 						//需要的节拍数
	told = SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told) tcnt += told - tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt += reload - tnow + told;
			told = tnow;
			if(tcnt >= ticks) break;			//时间超过/等于要延迟的时间,则退出.
		}
	}
}

#ifdef RC522_USE_HW_SPI 
static int32_t HW_SPI_WriteNBytes(SPI_TypeDef* SPIx, uint8_t *p_TxData,uint32_t sendDataNum)
{
	uint8_t retry=0;
	while(sendDataNum--){
		while((SPIx->SR&SPI_FLAG_TXE)==0)//等待发送区空
		{
			retry++;
			if(retry>20000)return -1;
		}
		SPIx->DR=*p_TxData++;//发送一个byte
		retry=0;
		while((SPIx->SR&SPI_FLAG_RXNE)==0)//等待接收完一个byte
		{
			SPIx->SR = SPIx->SR;
			retry++;
			if(retry>20000)return -1;
		}
		SPIx->DR;
	}
	return 0;
}
static int32_t HW_SPI_ReadNBytes(SPI_TypeDef* SPIx, uint8_t *p_RxData,uint32_t readDataNum)
{
	uint8_t retry=0;
	while(readDataNum--){
		SPIx->DR = 0xFF;
		while(!(SPIx->SR&SPI_FLAG_TXE)){
			retry++;
			if(retry>20000)return -1;
		}
		retry = 0;
		while(!(SPIx->SR&SPI_FLAG_RXNE)){
			retry++;
			if(retry>20000)return -1;
		}
		*p_RxData++ = SPIx->DR;
	}
	return 0;
}
#endif
/*
 * 函数名：SPI_RC522_SendByte
 * 描述  ：向RC522发送1 Byte 数据
 * 输入  ：byte，要发送的数据
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */

#ifdef RC522_USE_SW_SPI
static void SW_SPI_RC522_SendByte (uint8_t byte)
{
    uint8_t counter;

    for(counter=0; counter<8; counter++)
    {
        if (byte & 0x80)                                                                                           
            RC522_MOSI_1();
        else
            RC522_MOSI_0();

        delay_us(20);
        RC522_SCK_0 ();
        delay_us(20);
        RC522_SCK_1();
        delay_us(20);

        byte <<= 1;
    }
	RC522_SCK_0 ();
}

/*
 * 函数名：SPI_RC522_ReadByte
 * 描述  ：从RC522发送1 Byte 数据
 * 输入  ：无
 * 返回  : RC522返回的数据
 * 调用  ：内部调用
 */
static uint8_t SW_SPI_RC522_ReadByte ( void )
{
    uint8_t counter;
    uint8_t SPI_Data;

    for(counter=0; counter<8; counter++)
    {
        SPI_Data <<= 1;

        RC522_SCK_0 ();

        delay_us(20);

        if (RC522_MISO_GET() == 1)
            SPI_Data |= 0x01;

        delay_us(20);

        RC522_SCK_1 ();

        delay_us(20);
    }
//	printf("****%c****",SPI_Data);
	RC522_SCK_0();
    return SPI_Data;
}
#endif

void RC522_Init(void)
{
	RC522_ENABLE;
	HAL_SPI_Transmit(&hspi1, (uint8_t *)0xaa, sizeof((uint8_t *)0xaa), 0xFF);//启动传输
	RC522_DISABLE;
    HAL_Delay(50);
	PcdReset();//复位RC522读卡器
	HAL_Delay(10);
	PcdReset();//复位RC522读卡器
	HAL_Delay(10);
	PcdAntennaOff();//关闭天线发射
	HAL_Delay(10);
    PcdAntennaOn();//开启天线发射

	printf("RFID-MFRC522 初始化完成\r\nFindCard Starting ...\r\n");  //测试引脚初始化完成
}

//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);  //清RC522寄存位
    WriteRawRC(BitFramingReg, 0x07); //写RC623寄存器
    SetBitMask(TxControlReg, 0x03);  //置RC522寄存位

    ucComMF522Buf[0] = req_code;
	printf("aaa\r\n");
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
	printf("bbb\r\n");
    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }
    return status;
}
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);
    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }
    SetBitMask(CollReg, 0x80);
    return status;
}
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6] ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);
    ClearBitMask(Status2Reg, 0x08);
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }
    return status;
}
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }
    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }
    return status;
}
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          p [OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }
    return status;
}
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          p [IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }
    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}
//功    能：扣款和充值
//参数说明: dd_mode[IN]：命令字
//               0xC0 = 扣款
//               0xC1 = 充值
//          addr[IN]：钱包地址
//          pValue[IN]：4字节增(减)值，低位在前
//返    回: 成功返回MI_OK
char PcdValue(unsigned char dd_mode, unsigned char addr, unsigned char *pValue)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }
    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pValue + i);
        }
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}
//功    能：备份钱包
//参数说明: sourceaddr[IN]：源地址
//          goaladdr[IN]：目标地址
//返    回: 成功返回MI_OK
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }
    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }
    if (status != MI_OK)
    {
        return MI_ERR;
    }
    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);
    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }
    return status;
}
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
char PcdHalt(void)
{
    // char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}
//用MF522计算CRC16函数
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}
//功    能：复位RC522
//返    回: 成功返回MI_OK
char PcdReset(void)
{
	RC522_RST_HIGH;
    delay_us(10);
    RC522_RST_LOW;
    HAL_Delay(60);
    RC522_RST_HIGH;
    delay_us(500);
    WriteRawRC(CommandReg, PCD_RESETPHASE);
    HAL_Delay(2);

    WriteRawRC(ModeReg, 0x3D);		// 定义发送和接收常用模式 和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);	 //16位定时器低位 
    WriteRawRC(TReloadRegH, 0);		 //16位定时器高位
    WriteRawRC(TModeReg, 0x8D);			//定义内部定时器的设置
    WriteRawRC(TPrescalerReg, 0x3E);	 //设置定时器分频系数 
    WriteRawRC(TxAutoReg, 0x40);		//调制发送信号为100%ASK 

    ClearBitMask(TestPinEnReg, 0x80);	
    WriteRawRC(TxAutoReg, 0x40);	

    return MI_OK;
}
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char ucAddr;
    unsigned char ucResult = 0;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;
    HAL_Delay(1);
    RC522_ENABLE;
#ifdef RC522_USE_HW_SPI
    //HW_SPI_WriteNBytes(SPI1, &ucAddr, 1);  //向总线写多个数据
    //HW_SPI_ReadNBytes(SPI1, &ucResult,1);
	HAL_SPI_Transmit(&hspi1, &ucAddr, 1, 0xff);   	//向总线写多个数据
	HAL_SPI_Receive(&hspi1, &ucResult, 1, 0xff);	//向总线读多个数据
#else 
	SW_SPI_RC522_SendByte(ucAddr);
	ucResult = SW_SPI_RC522_ReadByte();
#endif

    RC522_DISABLE;
    return ucResult;
}
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
void WriteRawRC(unsigned char Address, unsigned char value)
{
    unsigned char ucAddr;
    uint8_t write_buffer[2] = {0};
    ucAddr = ((Address << 1) & 0x7E);
    write_buffer[0] = ucAddr;
    write_buffer[1] = value;
    HAL_Delay(1);
    RC522_ENABLE;
#ifdef RC522_USE_HW_SPI
    //HW_SPI_WriteNBytes(SPI1, write_buffer, 2);
	HAL_SPI_Transmit(&hspi1, write_buffer, 2, 0xff);
#else	
	SW_SPI_RC522_SendByte(write_buffer[0]);
	SW_SPI_RC522_SendByte(write_buffer[1]);
#endif
    RC522_DISABLE;
}
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);        //读RC632寄存器
    WriteRawRC(reg, tmp | mask); // set bit mask
}
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pIn [IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOut [OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
    case PCD_AUTHENT:	//Mifare认证
        irqEn = 0x12;	//允许错误中断请求ErrIEn 允许空闲中断IdleIEn
        waitFor = 0x10;	//认证寻卡等待时候 查询空闲中断标志位
        break;
    case PCD_TRANSCEIVE:	//接收发送 发送接收
        irqEn = 0x77;		//允许TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        waitFor = 0x30;		//寻卡等待时候 查询接收中断标志位与 空闲中断标志位
        break;	
    default:
        break;
    }
    WriteRawRC(ComIEnReg, irqEn | 0x80);	//IRqInv置位管脚IRQ与Status1Reg的IRq位的值相反
    ClearBitMask(ComIrqReg, 0x80);			//Set1该位清零时，CommIRqReg的屏蔽位清零
    WriteRawRC(CommandReg, PCD_IDLE);		//写空闲命令
    SetBitMask(FIFOLevelReg, 0x80);			
    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    WriteRawRC(CommandReg, Command);
    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }
    //i = 800; // 原来
	i = 1000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);
    if (i != 0)
    {
        if (!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }
    SetBitMask(ControlReg, 0x80); // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}
//开启天线
//每次启动或关闭天险发射之间应至少有1ms的间隔
void PcdAntennaOn(void)
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}
//关闭天线
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}

void RC522_Config(unsigned char Card_Type)
{
    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(ModeReg, 0x3D);
    WriteRawRC(RxSelReg, 0x86);
    WriteRawRC(RFCfgReg, 0x7F);
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    HAL_Delay(5);
    PcdAntennaOn();
}


// 测试程序0，完成addr读写读
void RC522_Handle(void)
{
    uint8_t i = 0;
    status = PcdRequest(PICC_REQALL, CT);	//寻天线区内全部卡
    //printf("\r\nstatus>>>>>>%d\r\n", status);


    if (status == MI_OK)	// 寻卡成功
    {
        status = MI_ERR;
        status = PcdAnticoll(SN);	// 防冲撞 获得UID 存入SN
    }

    if (status == MI_OK)	// 防冲撞成功
    {
        status = MI_ERR;
        ShowID(SN); 		// 串口打印卡的ID号 UID
        status = PcdSelect(SN);		// 选定卡片
    }
	
    if(status == MI_OK)//选卡成功
    {
        status = MI_ERR;
        // 验证A密钥 块地址 密码 SN
        // 注意：此处的块地址0x0B即2扇区3区块，可以替换成变量addr，
		// 此块地址只需要指向某一扇区就可以了，即2扇区为0x08-0x0B这个范围都有效
		// 且只能对验证过的扇区进行读写操作
        status = PcdAuthState(KEYA, 0x0B, KEY_A, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(A) success\r\n");
        }
        else
        {
            printf("PcdAuthState(A) failed\r\n");
        }
        // 验证B密钥 块地址 密码 SN  0x0B即2扇区3区块，可以替换成变量addr
        status = PcdAuthState(KEYB, 0x0B, KEY_B, SN);
        if(status == MI_OK)//验证成功
        {
            printf("PcdAuthState(B) success\r\n");
        }
        else
        {
            printf("PcdAuthState(B) failed\r\n");
        }
    }

    if(status == MI_OK)//验证成功
    {
        status = MI_ERR;
        // 读取M1卡一块数据 块地址 读取的数据 注意：因为上面验证的扇区是2扇区，所以只能对2扇区的数据进行读写，即0x08-0x0B这个范围，超出范围读取失败。
        status = PcdRead(addr, DATA);
        if(status == MI_OK)//读卡成功
        {
            // printf("RFID:%s\r\n", RFID);
            printf("DATA:");
            for(i = 0; i < 16; i++)
            {
                printf("%02x", DATA[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("PcdRead() failed\r\n");
        }
    }

//    if(status == MI_OK)//读卡成功
//    {
//        status = MI_ERR;
//        printf("Write the card after 1 second. Do not move the card!!!\r\n");
//        HAL_Delay(1000);
//        // status = PcdWrite(addr, DATA0);
//        // 写数据到M1卡一块
//        status = PcdWrite(addr, DATA1);
//        if(status == MI_OK)//写卡成功
//        {
//            printf("PcdWrite() success\r\n");
//        }
//        else
//        {
//            printf("PcdWrite() failed\r\n");
//            HAL_Delay(3000);
//        }
//    }

//    if(status == MI_OK)//写卡成功
//    {
//        status = MI_ERR;
//        // 读取M1卡一块数据 块地址 读取的数据
//        status = PcdRead(addr, DATA);
//        if(status == MI_OK)//读卡成功
//        {
//            // printf("DATA:%s\r\n", DATA);
//            printf("DATA:");
//            for(i = 0; i < 16; i++)
//            {
//                printf("%02x", DATA[i]);
//            }
//            printf("\r\n");
//        }
//        else
//        {
//            printf("PcdRead() failed\r\n");
//        }
//    }

//    if(status == MI_OK)//读卡成功
//    {
//        status = MI_ERR;
//        printf("RC522_Handle() run finished after 1 second!\r\n");
//        HAL_Delay(1000);
//    }
}






