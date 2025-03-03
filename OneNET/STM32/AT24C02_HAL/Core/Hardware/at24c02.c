#include "main.h"
#include "at24c02.h"
#include "iic.h"


/* ģ��IIC ���õ������ű�ǩ�������£�����iic.c�ļ����޸��¶�Ӧ����
	I2C1_SCL
	I2C1_SDA
*/

/**
 * @brief AT24C02��ʼ��
 * @param ��
 * @return ��
 */
void AT24C02_Init(void)
{
	I2C_Init();
}

/**
 * @brief AT24C02��ָ����ַ����һ���ֽ�����
 * @param Addr ��ַ 0x00~0xFF
 * @return ����������
 */
uint8_t AT24C02_ReadOneByte(uint16_t addr)
{
	uint8_t tmp;
	I2C_Start();
	I2C_Send_Byte(0xA0 + ((addr/256)<<1));	// ����дģʽ
	I2C_Wait_Ack();
	
	I2C_Send_Byte(addr%256);	
	I2C_Wait_Ack();
	
	I2C_Start();
	I2C_Send_Byte(0xA1);	// ���ö�ģʽ
	I2C_Wait_Ack();
	tmp = I2C_Read_Byte(0);	// �������ݲ����ͷ�Ӧ���ź�
	I2C_Stop();
	return tmp;	
}

/**
 * @brief AT24C02��ָ����ַд��һ���ֽ�����
 * @param addr ��ַ 0x00~0xFF
 * @param data д���һ�ֽ�����
 * @return ��
 */
void AT24C02_WriteOneByte(uint16_t addr, uint8_t data)
{
	I2C_Start();
	I2C_Send_Byte(0xA0);
	I2C_Wait_Ack();
	I2C_Send_Byte(addr);	// ����д���ַ
	I2C_Wait_Ack();
	I2C_Send_Byte(data);	// ����д������
	I2C_Wait_Ack();
	I2C_Stop();
	HAL_Delay(10);	// �ȴ�д�����
}

/**
 * @brief AT24C02��ָ����ַд�볤��λlen������
 * @param addr ��ʼд��ĵ�ַ
 * @param data д������ 
 * @param len д�����ݵĳ���
 * @return ��
 */
void AT24C02_WriteLenByte(uint16_t addr, uint32_t data, uint8_t len)
{
	uint8_t i;
	for(i = 0; i < len; i++)
	{
		AT24C02_WriteOneByte(addr + i, (data >> (i*8)) & 0xff);
	}
}

/**
 * @brief AT24C02��ָ����ַ��ʼ��������ΪLen������
 * @param addr ��ʼд��ĵ�ַ
 * @param data д������ 
 * @param len д�����ݵĳ���
 * @return ��
 */
uint32_t AT24C02_ReadLenByte(uint16_t addr, uint8_t len)
{
	uint8_t i;
	uint32_t tmp = 0;
	for(i = 0; i < len; i++)
	{
		tmp <<= 8;
		tmp += AT24C02_ReadOneByte(addr + len - i - 1);
	}
	return tmp;
}

 /**
 * @brief ���AT24C02�Ƿ����� �����һ����ַ(255)���洢��־��
 * @param ��
 * @return 1��ʧ�ܣ�0���ɹ�
 */
uint8_t AT24C02_Check(void)
{
	uint8_t tmp;
	tmp = AT24C02_ReadOneByte(255);
	if(tmp == 0x55)
		return 0;
	else  // �ų���һ�γ�ʼ�������
	{
		AT24C02_WriteOneByte(255, 0x55);
		tmp = AT24C02_ReadOneByte(255);
		//printf("%d", tmp);
		if(tmp == 0x55)
			return 0;
	}
	return 1;	
}

 /**
 * @brief AT24C02�����ָ����ַ��ʼд��ָ������������
 * @param addr д��ĵ�ַ 0~255
 * @return ��
 */
void AT24C02_Write(uint16_t addr, uint8_t *pBuffer, uint16_t num)
{
	while(num--)
	{
		AT24C02_WriteOneByte(addr++, *pBuffer++);
	}
}

 /**
 * @brief AT24C02�����ָ����ַ��ʼ����ָ������������
 * @param addr д��ĵ�ַ 0~255
 * @return ��
 */
void AT24C02_Read(uint16_t addr, uint8_t *pBuffer, uint16_t num)
{
	while(num)
	{
		*pBuffer++ = AT24C02_ReadOneByte(addr++);
		num--;
	}
}
