#ifndef __AT24C02_H
#define __AT24C02_H


void AT24C02_Init(void);
uint8_t AT24C02_ReadOneByte(uint16_t addr);
void AT24C02_WriteOneByte(uint16_t addr, uint8_t data);
void AT24C02_WriteLenByte(uint16_t addr, uint32_t data, uint8_t len);
uint32_t AT24C02_ReadLenByte(uint16_t addr, uint8_t len);
uint8_t AT24C02_Check(void);
void AT24C02_Write(uint16_t addr, uint8_t *pBuffer, uint16_t num);
void AT24C02_Read(uint16_t addr, uint8_t *pBuffer, uint16_t num);


#endif
