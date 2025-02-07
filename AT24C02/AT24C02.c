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

//    while (HAL_I2C_IsDeviceReady(hi2c, AT24C02_ADDR << 1, 1, AT24C02_TIMEOUT) != HAL_OK)
//    {
//        printf("AT24C02_Init: Device not ready, retrying...\r\n");
//        HAL_Delay(AT24C02_DELAY_MS);
//    }
//    printf("AT24C02_Init: Device is ready\r\n");
		
		
		if(HAL_I2C_IsDeviceReady(hi2c, AT24C02_ADDR << 1, 10, AT24C02_TIMEOUT) == HAL_OK){
			printf("AT24C02 Init OK!!\r\n");
			return;
		}
		
		printf("AT24C02 ERR!!\r\n");
		

   // uint8_t writeData = 0xFF;

    // AT24C02_Write(0xFF, 0xAB);  
    // AT24C02_Read(0xFF, &writeData);
    // printf("AT24C02_Init: Read data = 0x%02X\n", writeData);



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
    return HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE, 0XA0, (uint8_t)memAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, AT24C02_TIMEOUT);
}

// 读操作
HAL_StatusTypeDef AT24C02_Read(uint16_t memAddress, uint8_t *data)
{
#if AT24C02_DEBUG
    printf("AT24C02_Read: memAddress=0x%04X\n", memAddress);
#endif

    // 使用 HAL_I2C_Mem_Read 读取数据
    HAL_Delay(AT24C02_DELAY_MS);
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE, 0xA1, (uint8_t)memAddress, I2C_MEMADD_SIZE_8BIT, data, 1, AT24C02_TIMEOUT);

#if AT24C02_DEBUG
    if (status == HAL_OK) {
        printf("AT24C02_Read: Received data=0x%02X\n", *data);
    } else {
        printf("AT24C02_Read: Failed to receive data\n");
    }
#endif

    return status;
}


// 向 EEPROM 写入字节数组
HAL_StatusTypeDef AT24C02_Write_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status = HAL_OK;

    // 循环写入每个字节
    for (uint16_t i = 0; i < length; i++)
    {
        // 调用 AT24C02_Write 函数写入单字节数据
        status = AT24C02_Write(memAddress + i, data[i]);
        if (status != HAL_OK)
        {
            // 如果写入失败，立即返回错误
            return status;
        }

        // 等待写入完成（根据需要调整延时）
        HAL_Delay(AT24C02_DELAY_MS);
    }

    return status;
}


// 从 EEPROM 读取字节数组
HAL_StatusTypeDef AT24C02_Read_Array(uint16_t memAddress, uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status = HAL_OK;

    // 检查内存地址是否超出8位范围（AT24C02只有256字节）
    if (memAddress > 0xFF)
    {
        return HAL_ERROR;
    }

    // 使用正确的设备地址和8位内存地址大小进行读取
    status = HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE,
                              0xA1,                          // AT24C02读地址（假设A2A1A0=000）
                              memAddress,                    // 直接传递uint16_t，HAL库根据I2C_MEMADD_SIZE_8BIT处理低8位
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              length,
                              AT24C02_TIMEOUT);

    return status;
}

HAL_StatusTypeDef fAT24C02_Read_256Bytes(uint16_t memAddress, uint8_t *data)
{
    HAL_StatusTypeDef status = HAL_OK;

    for (uint16_t i = 0; i < 256; i++)
    {
        status = AT24C02_Read(memAddress + i, &data[i]);
        if (status != HAL_OK)
        {
            printf("AT24C02_Read_256Bytes: Failed to read at address 0x%04X\n", memAddress + i);
            return status;
        }
    }

    // 每行显示16个字节
    for (uint16_t i = 0; i < 256; i += 16)
    {
        printf("Address 0x%04X: ", memAddress + i);
        for (uint16_t j = 0; j < 16; j++)
        {
            printf("%02X ", data[i + j]);
        }
        printf("\n");
    }

    return status;
}

	
		




// 假设 EEPROM 总容量为 256 字节
#define EEPROM_SIZE 256

void Test_AT24C02(void)
{
    HAL_StatusTypeDef status;
    uint8_t writeData = 0xAB;        // 要写入EEPROM的单字节数据
    uint8_t readData = 0x00;         // 用于存储从EEPROM读取的单字节数据
    uint16_t memAddress = 0x10;      // 单字节测试时使用的 EEPROM 地址
	
		HAL_Delay(20);

    // 1. 单字节写入/读取测试
    printf("----- Byte Test -----\r\n");
    printf("Writing data to EEPROM at address 0x%04X...\r\n", memAddress);
    if (AT24C02_Write(memAddress, writeData) == HAL_OK)
    {
        printf("Data written successfully.\r\n");
    }
    else
    {
        printf("Failed to write data.\r\n");
    }

    printf("Reading data from EEPROM at address 0x%04X...\r\n", memAddress);
    if (AT24C02_Read(memAddress, &readData) == HAL_OK)
    {
        printf("Data read successfully: 0x%02X\r\n", readData);
    }
    else
    {
        printf("Failed to read data.\r\n");
    }

    if (readData == writeData)
    {
        printf("Data verification successful. Read data matches written data.\r\n");
    }
    else
    {
        printf("Data verification failed. Read data does not match written data.\r\n");
    }

    // 2. 数组写入/读取测试
    printf("\r\n----- Arr Test -----\r\n");
    #define TEST_ARRAY_SIZE 16
    uint8_t testArray[TEST_ARRAY_SIZE];
    uint8_t readArray[TEST_ARRAY_SIZE];
    memAddress = 0x20;  // 数组测试使用的起始地址

    // 初始化测试数据（例如依次填入 0,1,2,...）
    for (uint16_t i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        testArray[i] = (uint8_t)i+1;
    }

    printf("Writing array data to EEPROM starting at address 0x%04X...\r\n", memAddress);
    status = AT24C02_Write_Array(memAddress, testArray, TEST_ARRAY_SIZE);
    if (status == HAL_OK)
    {
        printf("Array data written successfully.\r\n");
    }
    else
    {
        printf("Failed to write array data. Status = %d\r\n", status);
    }

    // 清空 readArray 数组以便验证读取的数据
    memset(readArray, 0, TEST_ARRAY_SIZE);

    printf("Reading array data from EEPROM starting at address 0x%04X...\r\n", memAddress);
    status = AT24C02_Read_Array(memAddress, readArray, TEST_ARRAY_SIZE);
    if (status == HAL_OK)
    {
        printf("Array data read successfully.\r\n");
    }
    else
    {
        printf("Failed to read array data. Status = %d\r\n", status);
    }

    // 验证数组数据
    uint8_t arrayVerified = 1;
    for (uint16_t i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        if (testArray[i] != readArray[i])
        {
            arrayVerified = 0;
            printf("Mismatch at index %d: wrote 0x%02X, read 0x%02X\r\n", i, testArray[i], readArray[i]);
            break;
        }
    }
    if (arrayVerified)
    {
        printf("Array verification successful. Read data matches written data.\r\n");
    }
    else
    {
        printf("Array verification failed.\r\n");
    }

//    // 3. 清空整个 EEPROM（写入 0x00 到所有地址）
//    printf("\r\n----- Clear EEPROM -----\r\n");
//    // 每次写入 16 字节（你也可以根据 EEPROM 页写特性调整块大小）
//    uint8_t clearData[TEST_ARRAY_SIZE];
//    memset(clearData, 0x00, TEST_ARRAY_SIZE);
//    for (memAddress = 0; memAddress < EEPROM_SIZE; memAddress += TEST_ARRAY_SIZE)
//    {
//        status = AT24C02_Write_Array(memAddress, clearData, TEST_ARRAY_SIZE);
//        if (status != HAL_OK)
//        {
//            // printf("Failed to clear EEPROM at address 0x%04X, status = %d\r\n", memAddress, status);
//        }
//        else
//        {
//            // 可选：打印进度
//            // printf("Cleared EEPROM from 0x%04X to 0x%04X\r\n", memAddress, memAddress + TEST_ARRAY_SIZE - 1);
//        }
//    }
//    printf("EEPROM memory cleared.\r\n");
		
		
printf("\r\n----- Persistence Test -----\r\n");
uint8_t readdata[1], writedata[1] = {0x66};
uint8_t Persistence_status;

// 1. 尝试读取数据
Persistence_status = HAL_I2C_Mem_Read(&AT24C02_I2C_HANDLE,0xA1,0x30,I2C_MEMADD_SIZE_8BIT,readdata,1,AT24C02_TIMEOUT);
if (Persistence_status != HAL_OK) {
    printf("Read failed: %d\r\n", Persistence_status);
    return; // 根据实际需求处理错误
}

// 2. 检查数据有效性
if (readdata[0] == 0x66) {
    printf("Persistence ok\r\n");
} else {
    // 3. 数据无效，执行写入
    Persistence_status = HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE,0xA0,0x30,I2C_MEMADD_SIZE_8BIT,writedata,1,AT24C02_TIMEOUT);
    if (Persistence_status != HAL_OK) {
        printf("Write failed: %d\r\n", Persistence_status);
        return; // 处理写入失败
    }
    HAL_Delay(5); // 等待EEPROM写入完成
    printf("Data written, verify on next boot\r\n");
	}
}



void Test_AT24C02_Base(void){
	  // AT24C02_Init(&hi2c2);       // 使用I2C2初始化EEPROM


  // // 配置PC13为输出模式用于状态反馈
  // GPIO_InitTypeDef GPIO_InitStruct = {0};
  // GPIO_InitStruct.Pin = GPIO_PIN_13;
  // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

 #define AT24C02ADDR      (0x50)
 #define AT24C02_WRITE    (AT24C02ADDR << 1)
 #define AT24C02_READ     ((AT24C02ADDR << 1) | 0x01)

 #define BUFFER_SIZE      256
 #define BLOCK_SIZE       8
 #define BLOCK_COUNT      (BUFFER_SIZE / BLOCK_SIZE)

 uint8_t WriteBuffer[BUFFER_SIZE];
 uint8_t ReadBuffer[BUFFER_SIZE];

 // 初始化写缓冲区，填充数据0~255
 for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
     WriteBuffer[i] = i;
 }

 // 分块写入EEPROM，每块写入8个字节
 for (uint16_t j = 0; j < BLOCK_COUNT; j++) {
     if (HAL_I2C_Mem_Write(&hi2c2, AT24C02_WRITE, j * BLOCK_SIZE, I2C_MEMADD_SIZE_8BIT,
                            &WriteBuffer[j * BLOCK_SIZE], BLOCK_SIZE, 1000) == HAL_OK) {
         printf("write ok\r\n");
         // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
     } else {
         printf("write error\r\n");

         // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
     }


     HAL_Delay(20);
 }

 // 读取EEPROM中全部数据，以验证写入情况
 if (HAL_I2C_Mem_Read(&hi2c2, AT24C02_READ, 0, I2C_MEMADD_SIZE_8BIT, ReadBuffer, BUFFER_SIZE, 100) != HAL_OK) {
     // 读取失败，设置错误指示
     printf("read error\r\n");
     //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
 } else {
     printf("read ok\r\n");
     for (uint16_t i = 0; i < BUFFER_SIZE; i++) {
       printf("%d ", ReadBuffer[i]);
     }
     printf("\r\n");
     // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

 }

 // 无限循环闪烁LED，作为程序运行状态的反馈（约400ms切换一次）
 while (1) {
     HAL_Delay(400);
     // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
 }
}


// 清空指定区域的数据
uint8_t AT24C02_Clear(uint8_t beginaddr,uint8_t endaddr){
	
	uint8_t memsize = endaddr - beginaddr;
	uint8_t zero[] = {0x00};
	uint8_t  status;
	
	status = HAL_I2C_Mem_Write(&AT24C02_I2C_HANDLE,0xa0,beginaddr,I2C_MEMADD_SIZE_8BIT,zero,memsize,AT24C02_TIMEOUT);
	
	if(status == HAL_OK){
		printf("at24 clear ok\r\n");
	}
	else {
		printf("at24 clear fail\r\n");
	}
}









