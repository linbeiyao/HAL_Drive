// at24c02.c

#include "at24c02.h"
#include "stdint.h"

extern I2C_HandleTypeDef hi2c2;

// 宏定义
#define AT24C02_DELAY_MS 5
#define AT24C02_TIMEOUT 1000
#define AT24C02_I2C_HANDLE hi2c2            // 根据具体分配进行修改

// 初始化 I2C 设置
void AT24C02_Init(I2C_HandleTypeDef *hi2c)
{
    // 使用7位地址，HAL库会自动处理读/写位
    if (HAL_I2C_IsDeviceReady(hi2c, AT24C02_ADDR << 1, 10, AT24C02_TIMEOUT) != HAL_OK)
    {
        printf("AT24C02_Init: Device not ready\n");
    }

    uint8_t writeData = 0xFF;
    // if (HAL_I2C_Mem_Write(hi2c, AT24C02_ADDR << 1, 0xFF, I2C_MEMADD_SIZE_8BIT, &writeData, 1, AT24C02_TIMEOUT) != HAL_OK)
    // {
    //     printf("AT24C02_Init: Failed to write to EEPROM\n");
    // }

    // uint8_t readData;
    // if (HAL_I2C_Mem_Read(hi2c, AT24C02_ADDR << 1, 0xFF, I2C_MEMADD_SIZE_8BIT, &readData, 1, AT24C02_TIMEOUT) != HAL_OK)
    // {
    //     printf("AT24C02_Init: Failed to read from EEPROM\n");
    // }
    // else
    // {
    //     printf("AT24C02_Init: Read data = 0x%02X\n", readData);
    // }

    AT24C02_Write(0xFF, 0xAB);  
    AT24C02_Read(0xFF, &writeData);
    printf("AT24C02_Init: Read data = 0x%02X\n", writeData);

}

// 调试宏定义
#define AT24C02_DEBUG 1


// 写操作
HAL_StatusTypeDef AT24C02_Write(uint16_t memAddress, uint8_t data)
{
#if AT24C02_DEBUG
    printf("AT24C02_Write: memAddress=0x%04X, data=0x%02X\n", memAddress, data);
#endif

    // 使用 HAL_I2C_Mem_Write 写入数据
    HAL_Delay(AT24C02_DELAY_MS);
    return HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE, AT24C02_ADDR << 1, memAddress, I2C_MEMADD_SIZE_16BIT, &data, 1, AT24C02_TIMEOUT);
}

// 读操作
HAL_StatusTypeDef AT24C02_Read(uint16_t memAddress, uint8_t *data)
{
#if AT24C02_DEBUG
    printf("AT24C02_Read: memAddress=0x%04X\n", memAddress);
#endif

    // 使用 HAL_I2C_Mem_Read 读取数据
    HAL_Delay(AT24C02_DELAY_MS);
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE, AT24C02_ADDR << 1, memAddress, I2C_MEMADD_SIZE_16BIT, data, 1, AT24C02_TIMEOUT);

#if AT24C02_DEBUG
    if (status == HAL_OK) {
        printf("AT24C02_Read: Received data=0x%02X\n", *data);
    } else {
        printf("AT24C02_Read: Failed to receive data\n");
    }
#endif

    return status;
}

// 写入字节数组到 AT24C02
HAL_StatusTypeDef AT24C02_Write_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint16_t i;

    for (i = 0; i < length; i++)
    {
        // 使用 HAL_I2C_Mem_Write 写入单个字节
        status = HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE, AT24C02_ADDR << 1, memAddress, I2C_MEMADD_SIZE_16BIT, &data[i], 1, AT24C02_TIMEOUT);
        if (status != HAL_OK)
        {
            return status;
        }

        // 等待写入操作完成
        HAL_Delay(AT24C02_DELAY_MS);

        // 更新内存地址
        memAddress++;
    }

    return status;
}

// 从 AT24C02 读取字节数组
HAL_StatusTypeDef AT24C02_Read_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    // 使用 HAL_I2C_Mem_Read 读取多个字节
    HAL_Delay(AT24C02_DELAY_MS);
    return HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE, AT24C02_ADDR << 1, memAddress, I2C_MEMADD_SIZE_16BIT, data, length, AT24C02_TIMEOUT);
}

void Test_AT24C02() {
    uint8_t writeData = 0xAB;  // 要写入EEPROM的数据
    uint8_t readData = 0x00;   // 用于存储从EEPROM读取的数据
    uint16_t memAddress = 0x10; // EEPROM中的地址

    // 写入数据到EEPROM
    printf("Writing data to EEPROM...\r\n");
    if (AT24C02_Write(memAddress, writeData) == HAL_OK) {
        printf("Data written successfully.\r\n");
    } else {
        printf("Failed to write data.\r\n");
    }

    // 读取数据从EEPROM
    printf("Reading data from EEPROM...\r\n");
    if (AT24C02_Read(memAddress, &readData) == HAL_OK) {
        printf("Data read successfully: 0x%02X\r\n", readData);
    } else {
        printf("Failed to read data.\r\n");
    }

    // 验证数据
    if (readData == writeData) {
        printf("Data verification successful. Read data matches written data.\r\n");
    } else {
        printf("Data verification failed. Read data does not match written data.\r\n");
    }
}
