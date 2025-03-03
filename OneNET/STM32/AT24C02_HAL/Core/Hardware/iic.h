#ifndef __IIC_H
#define __IIC_H

void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_Wait_Ack(void);
void I2C_Ack(void);
void I2C_NAck(void);
void I2C_Send_Byte(uint8_t sendByte);
uint8_t I2C_Read_Byte(uint8_t ack);

#endif
