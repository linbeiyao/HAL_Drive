// at24c02.c

#include "at24c02.h"
#include "stdint.h"

#include <stdio.h>

extern I2C_HandleTypeDef hi2c2;

// 调试相关宏定义
#ifndef AT24C02_DEBUG
#define AT24C02_DEBUG 1  // 默认开启调试信息
#endif

// 基础宏定义
#define AT24C02_DELAY_MS 5
#define AT24C02_TIMEOUT 1000
#define AT24C02_I2C_HANDLE hi2c2

// 宏定义
#define AT24C02_MAX_ADDR 0xFF

// 地址检查函数
HAL_StatusTypeDef AT24C02_Check_Address(uint16_t address)
{
    if (address > AT24C02_MAX_ADDR)
    {
        // printf("AT24C02: Invalid address 0x%04X\r\n", address);
        return HAL_ERROR;
    }
    return HAL_OK;
}

// 初始化 I2C 设置
void AT24C02_Init(I2C_HandleTypeDef *hi2c)
{
    if(HAL_I2C_IsDeviceReady(hi2c, AT24C02_ADDR << 1, 10, AT24C02_TIMEOUT) == HAL_OK)
    {
        // printf("AT24C02: 初始化成功\r\n");
        return;
    }
    // printf("AT24C02: 初始化失败\r\n");
}

// 写操作
HAL_StatusTypeDef AT24C02_Write(uint16_t memAddress, uint8_t data)
{
    if (AT24C02_Check_Address(memAddress) != HAL_OK)
    {
        return HAL_ERROR;
    }

#if AT24C02_DEBUG
    // printf("AT24C02_Write: 地址=0x%04X, 数据=0x%02X\n", memAddress, data);
#endif

    HAL_StatusTypeDef status;
    HAL_Delay(AT24C02_DELAY_MS);
    status = HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE, 0XA0, (uint8_t)memAddress, 
                              I2C_MEMADD_SIZE_8BIT, &data, 1, AT24C02_TIMEOUT);

    if (status != HAL_OK)
    {
        // printf("AT24C02_Write: 写入失败，状态=%d\r\n", status);
    }
    return status;
}

// 读操作
HAL_StatusTypeDef AT24C02_Read(uint16_t memAddress, uint8_t *data)
{
    if (AT24C02_Check_Address(memAddress) != HAL_OK)
    {
        return HAL_ERROR;
    }

#if AT24C02_DEBUG
    // printf("AT24C02_Read: memAddress=0x%04X\n", memAddress);
#endif

    HAL_StatusTypeDef status;
    HAL_Delay(AT24C02_DELAY_MS);
    status = HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE, 0xA1, (uint8_t)memAddress, 
                             I2C_MEMADD_SIZE_8BIT, data, 1, AT24C02_TIMEOUT);

#if AT24C02_DEBUG
    if (status == HAL_OK) {
        // printf("AT24C02_Read: Received data=0x%02X\n", *data);
    } else {
        // printf("AT24C02_Read: Failed to receive data\n");
    }
#endif

    return status;
}

// 填充指定区域为指定值
HAL_StatusTypeDef AT24C02_Fill(uint8_t beginaddr, uint8_t endaddr, uint8_t value)
{
    // 参数检查
    if (beginaddr > endaddr || endaddr > AT24C02_MAX_ADDR)
    {
        // printf("AT24C02_Fill: 无效的地址范围 0x%02X-0x%02X\r\n", beginaddr, endaddr);
        return HAL_ERROR;
    }

    uint8_t buffer[AT24C02_PAGE_SIZE];
    for (uint8_t i = 0; i < AT24C02_PAGE_SIZE; i++)
    {
        buffer[i] = value;
    }

    // 分页写入
    for (uint16_t addr = beginaddr; addr <= endaddr; addr += AT24C02_PAGE_SIZE)
    {
        uint16_t remain = endaddr - addr + 1;
        uint16_t write_size = (remain > AT24C02_PAGE_SIZE) ? AT24C02_PAGE_SIZE : remain;

        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE, 
                                                    0xA0, 
                                                    addr,
                                                    I2C_MEMADD_SIZE_8BIT,
                                                    buffer, 
                                                    write_size, 
                                                    AT24C02_TIMEOUT);
        
        if (status != HAL_OK)
        {
            // printf("AT24C02_Fill: 填充失败，地址=0x%02X，状态=%d\r\n", addr, status);
            return status;
        }
        HAL_Delay(AT24C02_DELAY_MS);
    }
    
    // printf("AT24C02_Fill: 填充成功，范围=0x%02X-0x%02X，值=0x%02X\r\n", beginaddr, endaddr, value);
    return HAL_OK;
}

// 清空指定区域的数据（使用Fill函数实现）
HAL_StatusTypeDef AT24C02_Clear(uint8_t beginaddr, uint8_t endaddr)
{
    return AT24C02_Fill(beginaddr, endaddr, 0x00);
}

/**
 * @brief 向EEPROM写入字节数组
 * @param memAddress 起始地址
 * @param data 数据指针
 * @param length 数据长度
 * @return 状态
 * 
 * return HAL_OK: 成功
 * return HAL_ERROR: 失败
 */
HAL_StatusTypeDef AT24C02_Write_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status = HAL_OK;

    // 检查参数
    if (data == NULL || (memAddress + length) > AT24C02_MAX_ADDR)
    {
        // printf("AT24C02_Write_Array: 参数无效\r\n");
        return HAL_ERROR;
    }

    // 循环写入每个字节
    for (uint16_t i = 0; i < length; i++)
    {
        status = AT24C02_Write(memAddress + i, data[i]);
        if (status != HAL_OK)
        {
            if(status == HAL_ERROR){
                // printf("AT24C02_Write_Array: 地址0x%04X写入失败\r\n", memAddress + i);
            }else if(status == HAL_BUSY){
                // printf("AT24C02_Write_Array: 地址0x%04X设备忙\r\n", memAddress + i);
            }else if(status == HAL_TIMEOUT){
                // printf("AT24C02_Write_Array: 地址0x%04X超时\r\n", memAddress + i);
            }
            return status;
        }
        HAL_Delay(AT24C02_DELAY_MS);
    }

    return status;
}

// 从 EEPROM 读取字节数组
HAL_StatusTypeDef AT24C02_Read_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    if (data == NULL || (memAddress + length) > AT24C02_MAX_ADDR)
    {
        // printf("AT24C02_Read_Array: 参数无效\r\n");
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE,
                              0xA1,
                              memAddress,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              length,
                              AT24C02_TIMEOUT);

    if (status != HAL_OK)
    {
        // printf("AT24C02_Read_Array: 读取失败，状态=%d\r\n", status);
    }

    return status;
}

HAL_StatusTypeDef AT24C02_Read_256Bytes(uint16_t memAddress, uint8_t *data)
{
    if (data == NULL)
    {
        // printf("AT24C02_Read_256Bytes: 参数无效\r\n");
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = AT24C02_Read_Array(memAddress, data, 256);
    
    if (status == HAL_OK)
    {
        // 每行显示16个字节
        for (uint16_t i = 0; i < 256; i += 16)
        {
            // printf("地址 0x%04X: ", memAddress + i);
            for (uint16_t j = 0; j < 16; j++)
            {
                // printf("%02X ", data[i + j]);
            }
            // printf("\r\n");
        }
    }

    return status;
}

void Test_AT24C02(void)
{
    // printf("\r\n----- AT24C02 测试开始 -----\r\n");
    
    // 单字节测试
    uint8_t writeData = 0xAA;
    uint8_t readData = 0x00;
    uint16_t testAddr = 0x00;

    // printf("写入单字节测试...\r\n");
    if (AT24C02_Write(testAddr, writeData) == HAL_OK)
    {
        if (AT24C02_Read(testAddr, &readData) == HAL_OK)
        {
            // printf("写入值: 0x%02X, 读取值: 0x%02X\r\n", writeData, readData);
            if (writeData == readData)
            {
                // printf("单字节测试成功！\r\n");
            }
            else
            {
                // printf("单字节测试失败：数据不匹配\r\n");
            }
        }
    }

    // 数组测试
    // printf("\r\n数组测试...\r\n");
    uint8_t testArray[16];
    uint8_t readArray[16];
    
    // 初始化测试数据
    for (uint8_t i = 0; i < 16; i++)
    {
        testArray[i] = i;
    }

    if (AT24C02_Write_Array(0x10, testArray, 16) == HAL_OK)
    {
        if (AT24C02_Read_Array(0x10, readArray, 16) == HAL_OK)
        {
            // printf("读取的数组数据：\r\n");
            for (uint8_t i = 0; i < 16; i++)
            {
                // printf("%02X ", readArray[i]);
            }
            // printf("\r\n");
        }
    }

    // printf("----- AT24C02 测试结束 -----\r\n");
}

void Test_AT24C02_Base(void)
{
    // printf("\r\n----- AT24C02 基础测试开始 -----\r\n");

    #define BUFFER_SIZE      256
    #define BLOCK_SIZE       8
    #define BLOCK_COUNT      (BUFFER_SIZE / BLOCK_SIZE)

    uint8_t WriteBuffer[BUFFER_SIZE];
    uint8_t ReadBuffer[BUFFER_SIZE];

    // 初始化写缓冲区
    for (uint16_t i = 0; i < BUFFER_SIZE; i++)
    {
        WriteBuffer[i] = i;
    }

    // 分块写入
    // printf("开始写入数据...\r\n");
    for (uint16_t j = 0; j < BLOCK_COUNT; j++)
    {
        if (HAL_I2C_Mem_Write(&hi2c2, 0xA0, j * BLOCK_SIZE, I2C_MEMADD_SIZE_8BIT,
                            &WriteBuffer[j * BLOCK_SIZE], BLOCK_SIZE, 1000) == HAL_OK)
        {
            // printf("块 %d 写入成功\r\n", j);
        }
        else
        {
            // printf("块 %d 写入失败\r\n", j);
        }
        HAL_Delay(20);
    }

    // 读取验证
    // printf("\r\n开始读取数据...\r\n");
    if (HAL_I2C_Mem_Read(&hi2c2, 0xA1, 0, I2C_MEMADD_SIZE_8BIT, ReadBuffer, BUFFER_SIZE, 1000) == HAL_OK)
    {
        // printf("读取成功，数据：\r\n");
        for (uint16_t i = 0; i < BUFFER_SIZE; i++)
        {
            // printf("%02X ", ReadBuffer[i]);
            // if ((i + 1) % 16 == 0) 
        }
    }
    else
    {
        // printf("读取失败\r\n");
    }

    // printf("\r\n----- AT24C02 基础测试结束 -----\r\n");
}









