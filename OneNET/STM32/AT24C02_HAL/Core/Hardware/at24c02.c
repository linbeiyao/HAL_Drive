#include "main.h"
#include "at24c02.h"
#include "iic.h"


/* 模拟IIC 将用到的引脚标签设置如下，并在iic.c文件中修改下对应引脚
	I2C1_SCL
	I2C1_SDA
*/

/**
 * @brief AT24C02初始化
 * @param 无
 * @return 无
 */
void AT24C02_Init(void)
{
	I2C_Init();
}

/**
 * @brief AT24C02从指定地址读出一个字节数据
 * @param Addr 地址 0x00~0xFF
 * @return 读到的数据
 */
uint8_t AT24C02_ReadOneByte(uint16_t addr)
{
	uint8_t tmp;
	I2C_Start();
	I2C_Send_Byte(0xA0 + ((addr/256)<<1));	// 设置写模式
	I2C_Wait_Ack();
	
	I2C_Send_Byte(addr%256);	
	I2C_Wait_Ack();
	
	I2C_Start();
	I2C_Send_Byte(0xA1);	// 设置读模式
	I2C_Wait_Ack();
	tmp = I2C_Read_Byte(0);	// 接收数据并发送非应答信号
	I2C_Stop();
	return tmp;	
}

/**
 * @brief AT24C02在指定地址写入一个字节数据
 * @param addr 地址 0x00~0xFF
 * @param data 写入的一字节数据
 * @return 无
 */
void AT24C02_WriteOneByte(uint16_t addr, uint8_t data)
{
	I2C_Start();
	I2C_Send_Byte(0xA0);
	I2C_Wait_Ack();
	I2C_Send_Byte(addr);	// 发送写入地址
	I2C_Wait_Ack();
	I2C_Send_Byte(data);	// 发送写入数据
	I2C_Wait_Ack();
	I2C_Stop();
	HAL_Delay(10);	// 等待写入完毕
}

/**
 * @brief AT24C02在指定地址写入长度位len的数据
 * @param addr 开始写入的地址
 * @param data 写入数据 
 * @param len 写入数据的长度
 * @return 无
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
 * @brief AT24C02在指定地址开始读出长度为Len的数据
 * @param addr 开始写入的地址
 * @param data 写入数据 
 * @param len 写入数据的长度
 * @return 无
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
 * @brief 检测AT24C02是否正常 将最后一个地址(255)来存储标志字
 * @param 无
 * @return 1：失败，0：成功
 */
uint8_t AT24C02_Check(void)
{
	uint8_t tmp;
	tmp = AT24C02_ReadOneByte(255);
	if(tmp == 0x55)
		return 0;
	else  // 排除第一次初始化的情况
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
 * @brief AT24C02里面的指定地址开始写入指定个数的数据
 * @param addr 写入的地址 0~255
 * @return 无
 */
void AT24C02_Write(uint16_t addr, uint8_t *pBuffer, uint16_t num)
{
	while(num--)
	{
		AT24C02_WriteOneByte(addr++, *pBuffer++);
	}
}

 /**
 * @brief AT24C02里面的指定地址开始读出指定个数的数据
 * @param addr 写入的地址 0~255
 * @return 无
 */
void AT24C02_Read(uint16_t addr, uint8_t *pBuffer, uint16_t num)
{
	while(num)
	{
		*pBuffer++ = AT24C02_ReadOneByte(addr++);
		num--;
	}
}
