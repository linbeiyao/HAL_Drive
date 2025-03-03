#ifndef _ESP8266_H_
#define _ESP8266_H_

//#include "cJSON.h"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

void ESP8266_Init(void);                                    //ESP8266初始化

void ESP8266_Clear(void);                                   //清除ESP8266缓存

void ESP8266_SendData(unsigned char *data, unsigned short len);  //ESP8266发送数据

_Bool ESP8266_SendCmd(char *cmd, char *res);                //ESP8266发送命令

unsigned char *ESP8266_GetIPD(unsigned short timeOut);      //获取IPD数据
unsigned char* ESP8266_Receive(unsigned short timeOut);     //接收数据

void SetPassThrough(void);                                  //设置透传模式
void RstPassThrough(void);                                  //重置透传模式
//cJSON* ESP8266_Commit(char* url);                        //ESP8266提交数据

#endif
