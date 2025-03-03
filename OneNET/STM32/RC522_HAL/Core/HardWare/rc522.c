#include "RC522.h"
#include <stdio.h>

/*
	M1����Ϊ16��������ÿ���������ĸ��飨��0����1����2����3�����
	��16��������64���鰴���Ե�ַ���Ϊ��0~63
	��0�������Ŀ�0�������Ե�ַ0�飩�����ڴ�ų��̴��룬�Ѿ��̻����ɸ��� 
	ÿ�������Ŀ�0����1����2Ϊ���ݿ飬�����ڴ������
	ÿ�������Ŀ�3Ϊ���ƿ飨���Ե�ַΪ:��3����7����11.....����������A����ȡ���ơ�����B��
*/
/*******************************
	*����˵����
	*1--ƬѡSDA/CS/NSS  <----->PA4
	*2--SCK  <----->PA5
	*3--MOSI <----->PA7
	*4--MISO <----->PA6
	*5--IRQ ����
	*6--GND <----->GND
	*7--RST <----->PC5
	*8--VCC <----->VCC
************************************/

// ���SPI��ʱ��û���Գɹ���
/* ѡ��SPI������ʽ,Ĭ��ʹ��Ӳ��SPI,����������ͬʱȡ��ע�ͣ�*/
#define RC522_USE_HW_SPI		// Ӳ��SPI
//#define RC522_USE_SW_SPI		// ���SPI

#ifdef RC522_USE_HW_SPI 
	extern SPI_HandleTypeDef hspi1;		// HAL��ʹ�ã�ָ��SPI�ӿ�
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

/* ƬѡSDA/CS/NSS */
#define RC522_ENABLE HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define RC522_DISABLE HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

#define fac_us 72   //ʱ��Ƶ�ʣ���λMHZ

/* ȫ�ֱ��� */
unsigned char CT[2];		//������
unsigned char SN[4]; 		//����
unsigned char DATA[16];			//�������
unsigned char RFID[16];			//���RFID
unsigned char status;
uint8_t KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t KEY_B[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
// 0x08 ����2����0���飨����9�飩
unsigned char addr=0x08;

// ��ʾ���Ŀ��ţ���ʮ��������ʾ
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


/*΢�뼶��ʱ����*/
void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt = 0;
	uint32_t reload = SysTick->LOAD;			//LOAD��ֵ
	ticks = nus * fac_us; 						//��Ҫ�Ľ�����
	told = SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told) tcnt += told - tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt += reload - tnow + told;
			told = tnow;
			if(tcnt >= ticks) break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}
	}
}

#ifdef RC522_USE_HW_SPI 
static int32_t HW_SPI_WriteNBytes(SPI_TypeDef* SPIx, uint8_t *p_TxData,uint32_t sendDataNum)
{
	uint8_t retry=0;
	while(sendDataNum--){
		while((SPIx->SR&SPI_FLAG_TXE)==0)//�ȴ���������
		{
			retry++;
			if(retry>20000)return -1;
		}
		SPIx->DR=*p_TxData++;//����һ��byte
		retry=0;
		while((SPIx->SR&SPI_FLAG_RXNE)==0)//�ȴ�������һ��byte
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
 * ��������SPI_RC522_SendByte
 * ����  ����RC522����1 Byte ����
 * ����  ��byte��Ҫ���͵�����
 * ����  : RC522���ص�����
 * ����  ���ڲ�����
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
 * ��������SPI_RC522_ReadByte
 * ����  ����RC522����1 Byte ����
 * ����  ����
 * ����  : RC522���ص�����
 * ����  ���ڲ�����
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
	HAL_SPI_Transmit(&hspi1, (uint8_t *)0xaa, sizeof((uint8_t *)0xaa), 0xFF);//��������
	RC522_DISABLE;
    HAL_Delay(50);
	PcdReset();//��λRC522������
	HAL_Delay(10);
	PcdReset();//��λRC522������
	HAL_Delay(10);
	PcdAntennaOff();//�ر����߷���
	HAL_Delay(10);
    PcdAntennaOn();//�������߷���

	printf("RFID-MFRC522 ��ʼ�����\r\nFindCard Starting ...\r\n");  //�������ų�ʼ�����
}

//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);  //��RC522�Ĵ�λ
    WriteRawRC(BitFramingReg, 0x07); //дRC623�Ĵ���
    SetBitMask(TxControlReg, 0x03);  //��RC522�Ĵ�λ

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
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          p [OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          p [IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ��ۿ�ͳ�ֵ
//����˵��: dd_mode[IN]��������
//               0xC0 = �ۿ�
//               0xC1 = ��ֵ
//          addr[IN]��Ǯ����ַ
//          pValue[IN]��4�ֽ���(��)ֵ����λ��ǰ
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�����Ǯ��
//����˵��: sourceaddr[IN]��Դ��ַ
//          goaladdr[IN]��Ŀ���ַ
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
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
//��MF522����CRC16����
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
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
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

    WriteRawRC(ModeReg, 0x3D);		// ���巢�ͺͽ��ճ���ģʽ ��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL, 30);	 //16λ��ʱ����λ 
    WriteRawRC(TReloadRegH, 0);		 //16λ��ʱ����λ
    WriteRawRC(TModeReg, 0x8D);			//�����ڲ���ʱ��������
    WriteRawRC(TPrescalerReg, 0x3E);	 //���ö�ʱ����Ƶϵ�� 
    WriteRawRC(TxAutoReg, 0x40);		//���Ʒ����ź�Ϊ100%ASK 

    ClearBitMask(TestPinEnReg, 0x80);	
    WriteRawRC(TxAutoReg, 0x40);	

    return MI_OK;
}
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char ucAddr;
    unsigned char ucResult = 0;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;
    HAL_Delay(1);
    RC522_ENABLE;
#ifdef RC522_USE_HW_SPI
    //HW_SPI_WriteNBytes(SPI1, &ucAddr, 1);  //������д�������
    //HW_SPI_ReadNBytes(SPI1, &ucResult,1);
	HAL_SPI_Transmit(&hspi1, &ucAddr, 1, 0xff);   	//������д�������
	HAL_SPI_Receive(&hspi1, &ucResult, 1, 0xff);	//�����߶��������
#else 
	SW_SPI_RC522_SendByte(ucAddr);
	ucResult = SW_SPI_RC522_ReadByte();
#endif

    RC522_DISABLE;
    return ucResult;
}
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
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
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);        //��RC632�Ĵ���
    WriteRawRC(reg, tmp | mask); // set bit mask
}
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pIn [IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOut [OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
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
    case PCD_AUTHENT:	//Mifare��֤
        irqEn = 0x12;	//��������ж�����ErrIEn ��������ж�IdleIEn
        waitFor = 0x10;	//��֤Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ
        break;
    case PCD_TRANSCEIVE:	//���շ��� ���ͽ���
        irqEn = 0x77;		//����TxIEn RxIEn IdleIEn LoAlertIEn ErrIEn TimerIEn
        waitFor = 0x30;		//Ѱ���ȴ�ʱ�� ��ѯ�����жϱ�־λ�� �����жϱ�־λ
        break;	
    default:
        break;
    }
    WriteRawRC(ComIEnReg, irqEn | 0x80);	//IRqInv��λ�ܽ�IRQ��Status1Reg��IRqλ��ֵ�෴
    ClearBitMask(ComIrqReg, 0x80);			//Set1��λ����ʱ��CommIRqReg������λ����
    WriteRawRC(CommandReg, PCD_IDLE);		//д��������
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
    //i = 800; // ԭ��
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
//��������
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
void PcdAntennaOn(void)
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}
//�ر�����
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


// ���Գ���0�����addr��д��
void RC522_Handle(void)
{
    uint8_t i = 0;
    status = PcdRequest(PICC_REQALL, CT);	//Ѱ��������ȫ����
    //printf("\r\nstatus>>>>>>%d\r\n", status);


    if (status == MI_OK)	// Ѱ���ɹ�
    {
        status = MI_ERR;
        status = PcdAnticoll(SN);	// ����ײ ���UID ����SN
    }

    if (status == MI_OK)	// ����ײ�ɹ�
    {
        status = MI_ERR;
        ShowID(SN); 		// ���ڴ�ӡ����ID�� UID
        status = PcdSelect(SN);		// ѡ����Ƭ
    }
	
    if(status == MI_OK)//ѡ���ɹ�
    {
        status = MI_ERR;
        // ��֤A��Կ ���ַ ���� SN
        // ע�⣺�˴��Ŀ��ַ0x0B��2����3���飬�����滻�ɱ���addr��
		// �˿��ַֻ��Ҫָ��ĳһ�����Ϳ����ˣ���2����Ϊ0x08-0x0B�����Χ����Ч
		// ��ֻ�ܶ���֤�����������ж�д����
        status = PcdAuthState(KEYA, 0x0B, KEY_A, SN);
        if(status == MI_OK)//��֤�ɹ�
        {
            printf("PcdAuthState(A) success\r\n");
        }
        else
        {
            printf("PcdAuthState(A) failed\r\n");
        }
        // ��֤B��Կ ���ַ ���� SN  0x0B��2����3���飬�����滻�ɱ���addr
        status = PcdAuthState(KEYB, 0x0B, KEY_B, SN);
        if(status == MI_OK)//��֤�ɹ�
        {
            printf("PcdAuthState(B) success\r\n");
        }
        else
        {
            printf("PcdAuthState(B) failed\r\n");
        }
    }

    if(status == MI_OK)//��֤�ɹ�
    {
        status = MI_ERR;
        // ��ȡM1��һ������ ���ַ ��ȡ������ ע�⣺��Ϊ������֤��������2����������ֻ�ܶ�2���������ݽ��ж�д����0x08-0x0B�����Χ��������Χ��ȡʧ�ܡ�
        status = PcdRead(addr, DATA);
        if(status == MI_OK)//�����ɹ�
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

//    if(status == MI_OK)//�����ɹ�
//    {
//        status = MI_ERR;
//        printf("Write the card after 1 second. Do not move the card!!!\r\n");
//        HAL_Delay(1000);
//        // status = PcdWrite(addr, DATA0);
//        // д���ݵ�M1��һ��
//        status = PcdWrite(addr, DATA1);
//        if(status == MI_OK)//д���ɹ�
//        {
//            printf("PcdWrite() success\r\n");
//        }
//        else
//        {
//            printf("PcdWrite() failed\r\n");
//            HAL_Delay(3000);
//        }
//    }

//    if(status == MI_OK)//д���ɹ�
//    {
//        status = MI_ERR;
//        // ��ȡM1��һ������ ���ַ ��ȡ������
//        status = PcdRead(addr, DATA);
//        if(status == MI_OK)//�����ɹ�
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

//    if(status == MI_OK)//�����ɹ�
//    {
//        status = MI_ERR;
//        printf("RC522_Handle() run finished after 1 second!\r\n");
//        HAL_Delay(1000);
//    }
}






