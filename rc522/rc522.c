// Mifare RC522 RFID读卡器 13.56 MHz
// STM32F103 RFID RC522 SPI1 / UART / USB / Keil HAL / 2017 vk.com/zz555

// 引脚定义
// PA0  - (OUT) LED2
// PA1  - (IN)  BTN1
// PA4  - (OUT) SPI1_NSS (Soft)
// PA5  - (OUT) SPI1_SCK
// PA6  - (IN)  SPI1_MISO (Master In)
// PA7  - (OUT) SPI1_MOSI (Master Out)
// PA9  - (OUT) TX UART1 (RX-RS232)
// PA10 - (IN)  RX UART1 (TX-RS232)
// PA11 - (OUT) USB_DM
// PA12 - (OUT) USB_DP
// PA13 - (IN)  SWDIO
// PA14 - (IN)  SWDCLK
// PC13 - (OUT) LED1

// MFRC522与STM32F103引脚连接说明
// CS (SDA)     PA4                 SPI1_NSS    芯片选择引脚
// SCK          PA5                 SPI1_SCK    串行时钟引脚
// MOSI         PA7                 SPI1_MOSI   主设备输出从设备输入引脚
// MISO         PA6                 SPI1_MISO   主设备输入从设备输出引脚
// IRQ          -                   中断引脚（未使用）
// GND          GND                 接地
// RST          3.3V                复位引脚（3.3V供电）
// VCC          3.3V                电源3.3V

#include "stm32f1xx_hal.h"
#include "rc522.h"

extern SPI_HandleTypeDef hspi1;

/**
  * @brief  通过SPI发送一个字节并接收一个字节
  * @param  data 要发送的字节
  * @retval 接收到的字节
  */
uint8_t SPI1SendByte(uint8_t data)
{
    unsigned char writeCommand[1];
    unsigned char readValue[1];

    writeCommand[0] = data;
    // 同时发送和接收一个字节数据
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&writeCommand, (uint8_t *)&readValue, 1, 10);
    return readValue[0];
}

/**
  * @brief  向MFRC522的某个寄存器写入一个字节
  * @param  address 目标寄存器地址（写寄存器时，最低位为0）
  * @param  value   写入的值
  */
void SPI1_WriteReg(uint8_t address, uint8_t value)
{
    cs_reset();                // 片选拉低，使能SPI通信
    SPI1SendByte(address);     // 发送寄存器地址
    SPI1SendByte(value);       // 发送要写入的值
    cs_set();                  // 片选拉高，结束通信
}

/**
  * @brief  从MFRC522的某个寄存器读取一个字节
  * @param  address 目标寄存器地址（读寄存器时，最低位为1）
  * @retval 读取到的字节
  */
uint8_t SPI1_ReadReg(uint8_t address)
{
    uint8_t val;

    cs_reset();                 // 片选拉低
    SPI1SendByte(address);      // 发送寄存器地址（读模式）
    val = SPI1SendByte(0x00);   // 读取寄存器的值
    cs_set();                   // 片选拉高
    return val;
}

/**
  * @brief  向MFRC522写一个寄存器
  * @param  addr 要写的寄存器地址（只保留高6位，最低位清零）
  * @param  val  要写入的值
  */
void MFRC522_WriteRegister(uint8_t addr, uint8_t val)
{
    // 写寄存器时，格式: 0XXXXXX0(低2位)
    addr = (addr << 1) & 0x7E;
    SPI1_WriteReg(addr, val);
}

/**
  * @brief  从MFRC522读一个寄存器
  * @param  addr 要读取的寄存器地址（只保留高6位，最低位置1）
  * @retval 读取到的寄存器值
  */
uint8_t MFRC522_ReadRegister(uint8_t addr)
{
    uint8_t val;
    // 读寄存器时，格式: 0XXXXXX1
    addr = ((addr << 1) & 0x7E) | 0x80;
    val = SPI1_ReadReg(addr);
    return val;
}

/**
  * @brief  检测并读取卡片序列号
  * @param  id  用于存储卡片类型和卡片序列号
  * @retval 状态值，MI_OK表示成功，否则表示失败
  */
uint8_t MFRC522_Check(uint8_t *id)
{
    uint8_t status;
    // 请求寻卡，获取卡片类型
    status = MFRC522_Request(PICC_REQIDL, id);
    if (status == MI_OK)
    {
        // 如果检测到卡片，进行防冲突处理，获取4字节卡号
        status = MFRC522_Anticoll(id);
    }
    // 让卡片进入休眠
    MFRC522_Halt();
    return status;
}

/**
  * @brief  对比两个卡号（5字节）是否相同
  * @param  CardID    读取到的卡号
  * @param  CompareID 用于比较的卡号
  * @retval MI_OK表示相同，MI_ERR表示不同
  */
uint8_t MFRC522_Compare(uint8_t *CardID, uint8_t *CompareID)
{
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        if (CardID[i] != CompareID[i])
            return MI_ERR;
    }
    return MI_OK;
}

/**
  * @brief  在指定寄存器的原有值上置位（将mask对应的位设置为1）
  * @param  reg  寄存器地址
  * @param  mask 需要置位的位掩码
  */
void MFRC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) | mask);
}

/**
  * @brief  在指定寄存器的原有值上清位（将mask对应的位清零）
  * @param  reg  寄存器地址
  * @param  mask 需要清零的位掩码
  */
void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) & (~mask));
}

/**
  * @brief  请求寻卡（如寻Mifare卡），获取卡片类型
  * @param  reqMode 寻卡模式（如PICC_REQIDL）
  * @param  TagType 用于存储返回的卡类型(2字节)
  * @retval 状态值，MI_OK表示成功，否则表示失败
  */
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
    uint8_t status;
    uint16_t backBits; // 接收到的数据位数

    // TxLastBits = BitFramingReg[2..0] = 7
    MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);

    TagType[0] = reqMode;
    // 发送寻卡指令，并接收卡片应答
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    // 期望收到 0x10 = 16位 如果不等，说明寻卡失败
    if ((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }
    return status;
}

/**
  * @brief  与卡片进行数据交互的核心函数（发送并接收数据）
  * @param  command   命令字（PCD_AUTHENT 或 PCD_TRANSCEIVE 等）
  * @param  sendData  要发送的数据
  * @param  sendLen   要发送的数据长度
  * @param  backData  用于存储接收的数据
  * @param  backLen   用于存储接收位数
  * @retval 状态值，MI_OK表示成功，否则表示失败
  */
uint8_t MFRC522_ToCard(uint8_t command,
                       uint8_t *sendData,
                       uint8_t sendLen,
                       uint8_t *backData,
                       uint16_t *backLen)
{
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    // 根据命令设置中断使能位和等待的中断标志
    switch (command)
    {
    case PCD_AUTHENT:
    {
        irqEn = 0x12;   // 认证时要启用Tx和Rx中断
        waitIRq = 0x10; // 等待认证完成后触发的中断
        break;
    }
    case PCD_TRANSCEIVE:
    {
        irqEn = 0x77;   // 同时启用多个中断
        waitIRq = 0x30; // 等待发送完成或接收完成
        break;
    }
    default:
        break;
    }

    // 配置中断使能寄存器
    MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
    // 清除CommIrqReg的中断标志
    MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
    // 发送前先清FIFO
    MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);
    // 置空当前命令寄存器
    MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

    // 将数据写入FIFO
    for (i = 0; i < sendLen; i++)
    {
        MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);
    }

    // 执行命令
    MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
    if (command == PCD_TRANSCEIVE)
    {
        // StartSend=1，开始发送数据
        MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);
    }

    // 等待接收数据完成(或认证完成等)
    i = 2000; // 超时计数，避免死循环，根据系统时钟和卡的等待要求
    do
    {
        // 轮询中断标志CommIrqReg[0..7]
        // TxIRq、RxIRq、IdleIRq、HiAlertIRq、LoAlertIRq、ErrIRq、TimerIRq
        n = MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    // 停止发送
    MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);

    if (i != 0)
    {
        // 检查错误标志寄存器
        if (!(MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) // 0x1B = 00011011b
        {
            status = MI_OK;

            // 判断是否没有卡片
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }

            // 如果是发送并接收命令
            if (command == PCD_TRANSCEIVE)
            {
                // FIFO中保存的数据长度
                n = MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);
                // 最后接收到的位数
                lastBits = MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;

                if (lastBits)
                {
                    *backLen = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *backLen = n * 8;
                }

                if (n == 0) n = 1;
                if (n > MFRC522_MAX_LEN) n = MFRC522_MAX_LEN;

                // 将FIFO中的数据读到backData
                for (i = 0; i < n; i++)
                {
                    backData[i] = MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }
    return status;
}

/**
  * @brief  防冲突检测，读取卡片4字节序列号
  * @param  serNum 用于存储序列号（至少5字节，最后一字节为校验）
  * @retval 状态值，MI_OK表示成功，否则表示失败
  */
uint8_t MFRC522_Anticoll(uint8_t *serNum)
{
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint16_t unLen;

    // TxLastBists = BitFramingReg[2..0] = 0
    MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);

    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    // 发送防冲突命令并接收返回的卡序列号
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    if (status == MI_OK)
    {
        // 校验卡号的最后一字节(异或校验)
        for (i = 0; i < 4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i])
        {
            status = MI_ERR;
        }
    }
    return status;
}

/**
  * @brief  计算CRC校验
  * @param  pIndata  输入数据缓冲区
  * @param  len      输入数据长度
  * @param  pOutData 用于存储CRC计算结果(2字节)
  */
void MFRC522_CalculateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
    uint8_t i, n;

    // 先清除CRC中断标志
    MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);
    // 清FIFO指针
    MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);

    // 将数据写入FIFO
    for (i = 0; i < len; i++)
    {
        MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata + i));
    }
    // 启动CRC计算
    MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

    // 等待计算完成
    i = 0xFF;
    do
    {
        n = MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
        i--;
    } while ((i != 0) && !(n & 0x04));

    // 读取CRC计算结果
    pOutData[0] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
    pOutData[1] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

/**
  * @brief  选定卡片
  * @param  serNum 卡片的序列号（5字节，最后一字节为校验）
  * @retval 返回卡容量大小，0表示失败
  */
uint8_t MFRC522_SelectTag(uint8_t *serNum)
{
    uint8_t i;
    uint8_t status;
    uint8_t size;
    uint16_t recvBits;
    uint8_t buffer[9];

    // 命令字：0x93 = PICC_SElECTTAG, 0x70 表示所有字节都用于CRC
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;

    // 序列号4字节+校验1字节
    for (i = 0; i < 5; i++)
    {
        buffer[i + 2] = *(serNum + i);
    }

    // 计算CRC并放到 buffer[7..8]
    MFRC522_CalculateCRC(buffer, 7, &buffer[7]);

    // 发送选择卡命令
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    if ((status == MI_OK) && (recvBits == 0x18))
    {
        // 如果成功，buffer[0]中存储了卡容量大小
        size = buffer[0];
    }
    else
    {
        size = 0;
    }
    return size;
}

/**
  * @brief  验证卡片指定扇区密码
  * @param  authMode  验证模式(如PICC_AUTHENT1A或PICC_AUTHENT1B)
  * @param  BlockAddr 块地址
  * @param  Sectorkey 密码(6字节)
  * @param  serNum    卡号(4字节)
  * @retval MI_OK表示验证通过，否则表示验证失败
  */
uint8_t MFRC522_Auth(uint8_t authMode,
                     uint8_t BlockAddr,
                     uint8_t *Sectorkey,
                     uint8_t *serNum)
{
    uint8_t status;
    uint16_t recvBits;
    uint8_t i;
    uint8_t buff[12];

    // 拼接验证命令块地址 + 扇区密钥 + 卡片序列号
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i = 0; i < 6; i++)
    {
        buff[i + 2] = *(Sectorkey + i);
    }
    for (i = 0; i < 4; i++)
    {
        buff[i + 8] = *(serNum + i);
    }

    // 发送认证命令
    status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
    // 验证结果可通过读取Status2Reg的第3位(Status2Reg[3])判断：1表示认证成功
    if ((status != MI_OK) || (!(MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08)))
    {
        status = MI_ERR;
    }
    return status;
}

/**
  * @brief  读取指定块的数据（16字节）
  * @param  blockAddr 块地址
  * @param  recvData  用于存储读取结果的缓冲区（至少16字节）
  * @retval MI_OK表示成功，否则表示失败
  */
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData)
{
    uint8_t status;
    uint16_t unLen;

    // 发送读块命令
    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    // 计算CRC并附在后面
    MFRC522_CalculateCRC(recvData, 2, &recvData[2]);

    // 发送读取命令并接收返回数据
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
    // 如果返回的数据长度不是 0x90(16字节数据 + CRC等)，则视为失败
    if ((status != MI_OK) || (unLen != 0x90))
    {
        status = MI_ERR;
    }
    return status;
}

/**
  * @brief  向指定块写入16字节数据
  * @param  blockAddr 块地址
  * @param  writeData 要写入的数据(16字节)
  * @retval MI_OK表示成功，否则表示失败
  */
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData)
{
    uint8_t status;
    uint16_t recvBits;
    uint8_t i;
    uint8_t buff[18];

    // 发送写块命令
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    // 计算CRC并附在后面
    MFRC522_CalculateCRC(buff, 2, &buff[2]);

    // 发送写命令
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
    // 正常情况下，写命令应收到 4位应答，并且buff[0]的低4位是 0x0A
    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    // 如果第一步正确，再写16字节数据
    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            buff[i] = *(writeData + i);
        }
        // 再次计算CRC并附在后面
        MFRC522_CalculateCRC(buff, 16, &buff[16]);
        // 发送16字节数据
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

        // 同理，写入后应收到4位应答，并且buff[0]的低4位是 0x0A
        if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}

/**
  * @brief  初始化MFRC522
  *         重置RC522并对寄存器进行配置，同时开启天线
  */
void MFRC522_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // 片选信号线 拉低 表示选中
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);   // 复位信号线 拉高 表示正常工作


    MFRC522_Reset();

    // 定时器模式设置
    MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
    // 定时器分频系数
    MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);
    // 定时器低8位
    MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);
    // 定时器高8位
    MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);

    // 射频配置寄存器，0x70 = 最大增益48dB
    MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70);
    // 自动发送设置
    MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
    // 模式寄存器，0x3D表示CRC初始值0x6363
    MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);

    // 打开天线
    MFRC522_AntennaOn();
}

/**
  * @brief  复位RC522
  */
void MFRC522_Reset(void)
{
    // 写入PCD_RESETPHASE命令
    MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

/**
  * @brief  打开天线输出
  */
void MFRC522_AntennaOn(void)
{
    uint8_t temp;

    temp = MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
    // 如果天线位未打开，则置位
    if (!(temp & 0x03))
    {
        MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
    }
}

/**
  * @brief  关闭天线输出
  */
void MFRC522_AntennaOff(void)
{
    // 清除天线位
    MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

/**
  * @brief  让卡片进入休眠状态
  */
void MFRC522_Halt(void)
{
    uint16_t unLen;
    uint8_t buff[4];

    buff[0] = PICC_HALT;
    buff[1] = 0;
    // 计算CRC
    MFRC522_CalculateCRC(buff, 2, &buff[2]);
    // 发送命令让卡片休眠
    MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}



// 测试函数：初始化RFID模块并检测卡片
void RFID_Test(void)
{
    uint8_t status;
    uint8_t cardID[5];  // 存放卡片序列号（4字节序号+1字节校验）

    // 初始化RC522模块
    // MFRC522_Init();

    // 提示信息，可通过LED或UART通知用户系统已就绪
    printf("[RFID] Test start\r\n");

    while(1)
    {
        // 检测卡片并获取卡片序列号
        status = MFRC522_Check(cardID);
        if(status == MI_OK)
        {
            // 检测到卡片，打印卡片序列号
            printf("[RFID] Card detected!\r\n");
            printf("[RFID] Card ID: %02X %02X %02X %02X %02X\r\n",
                    cardID[0], cardID[1], cardID[2], cardID[3], cardID[4]);
            printf("[RFID] Wait 1s...\r\n");

            // 延时一段时间以避免连续多次读取同一张卡
            HAL_Delay(1000);
        }
        else
        {
            // 未检测到卡片，稍微延时后继续检测
            printf("[RFID] No card, err:%d\r\n", status);
            HAL_Delay(100);
        }
    }
}