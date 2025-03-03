#ifndef __KEY_H
#define __KEY_H

#include "gpio.h"

/* �������Ŷ��� */
typedef struct {
	GPIO_TypeDef* port;				// �˿ں�
	uint16_t 	  pin;				// ���ź�
	uint16_t	  pressed_state;	// ��������ʱ��״̬��0Ϊ�͵�ƽ��Ч��1Ϊ�ߵ�ƽ��Ч
}key_gpio_t;

/* ����״̬ */
typedef enum {
	KEY_CHECK = 0,		// �ͷ��ɿ�
	KEY_CONFIRM,		// ����ȷ��
	KEY_SHORT_PRESSED,	// �̰�	
	KEY_LONG_PRESSED,	// ����			
}key_state_t;

/* �����¼� */
typedef enum {
	EVENT_NULL = 0,				// ���¼�
	EVENT_SHORT_PRESSED,		// �̰��¼�
	EVENT_LONG_PRESSED,			// �����¼�
}key_event_t;

/* ÿ������������ */
typedef struct {
	key_state_t current_state;		// ������ǰ״̬
	uint32_t pressed_time;			// ����ʱ��
	key_event_t key_event;			// �����¼�
}key_param_t;

/* �ⲿ���� */


/* �������� */
void key_scan(void);
void key_handle(void);






#endif
