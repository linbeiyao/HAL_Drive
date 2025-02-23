// at24c02.h

#ifndef __AT24C02_H
#define __AT24C02_H

/**
 * 在进行I2C通信时，主机发送启动信号后，再发送寻址信号。
 * AT24CXX EEPROM Board模块中默认为接地。
 * 写24C02的时候，从器件地址为10100000（0xA0）；
 * 读24C02的时候，从器件地址为10100001（0xA1）。
 * 
 * 片内地址寻址：
 * 芯片寻址可对内部256B中的任一个进行读/写操作，其寻址范围为00~FF，共256个寻址单位。
 * 
 * 24C02是一个2K Bit的串行EEPROM存储器（掉电不丢失），内部含有256个字节。
 */

#include "stm32f1xx_hal.h"  // 包含 STM32 HAL 库头文件

// 设备相关宏定义
#define AT24C02_ADDR        0x50    // AT24C02 I2C 写地址 (7位地址左移一位)
#define AT24C02_PAGE_SIZE   8       // AT24C02页大小为8字节
#define AT24C02_MAX_ADDR    0xFF    // 最大地址
#define AT24C02_SIZE        256     // 总容量（字节）

// 状态码定义
#define AT24C02_OK         0x00
#define AT24C02_ERROR      0x01
#define AT24C02_BUSY       0x02
#define AT24C02_TIMEOUT    0x03

// 基本功能函数
void AT24C02_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef AT24C02_Write(uint16_t memAddress, uint8_t data);
HAL_StatusTypeDef AT24C02_Read(uint16_t memAddress, uint8_t *data);

// 数组操作函数
HAL_StatusTypeDef AT24C02_Write_Array(uint16_t memAddress, uint8_t *data, uint16_t length);
HAL_StatusTypeDef AT24C02_Read_Array(uint16_t memAddress, uint8_t *data, uint16_t length);
HAL_StatusTypeDef AT24C02_Read_256Bytes(uint16_t memAddress, uint8_t *data);

// 内存管理函数
HAL_StatusTypeDef AT24C02_Clear(uint8_t beginaddr, uint8_t endaddr);
HAL_StatusTypeDef AT24C02_Fill(uint8_t beginaddr, uint8_t endaddr, uint8_t value);
HAL_StatusTypeDef AT24C02_Check_Address(uint16_t address);

// 测试函数
void Test_AT24C02(void);            // Test demo 函数            
void Test_AT24C02_Base(void);       // 使用底层库函数进行测试

#endif /* __AT24C02_H */
