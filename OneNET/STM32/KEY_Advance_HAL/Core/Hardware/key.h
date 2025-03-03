#ifndef __KEY_H
#define __KEY_H

#include "gpio.h"


/* ��Ӧ����ֵ */
typedef enum {
	KEY_NULL = 0,
	KEY_0,
	KEY_1,
	KEY_WK_UP,
}KEY_VALUE;

/* ����״̬ */
typedef enum {
	KEY_CHECK = 0,			// �������
	KEY_CONFIRM,			// ����ȷ��
	KEY_RELEASE,			// �����ɿ�
}KEY_STATE;

/* �������� */
typedef enum {
	NULL_KEY = 0,		// �ް���
	SHORT_KEY,			// �̰�
	LONG_KEY,			// ����
}KEY_TYPE;

/* ���� */
extern uint8_t g_KeyFlag; 
extern KEY_TYPE KeyType;
extern KEY_VALUE KeyValue;
/* ���� */
void Key_Scan(void);

//uint8_t Key_GetNum(void);

#endif
