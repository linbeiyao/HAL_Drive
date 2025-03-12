#ifndef _ESP8266_H_
#define _ESP8266_H_

//#include "cJSON.h"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

void ESP8266_Init(void);

void ESP8266_Clear(void);

void ESP8266_SendData(unsigned char *data, unsigned short len);

_Bool ESP8266_SendCmd(char *cmd, char *res);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);
unsigned char* ESP8266_Receive(unsigned short timeOut);

void SetPassThrough(void);
void RstPassThrough(void);
//cJSON* ESP8266_Commit(char* url);

#endif
