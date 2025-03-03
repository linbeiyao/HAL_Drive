#include "key.h"
#include <stdio.h>

/* �����б� */
key_gpio_t key_gpio[] = {
	{GPIOE, GPIO_PIN_4, 0},		// KEY0 ����Ϊ�͵�ƽ
	{GPIOE, GPIO_PIN_3, 0},		// KEY1 ����Ϊ�͵�ƽ
	{GPIOA, GPIO_PIN_0, 1},		// KEY_WP_UO ����Ϊ�ߵ�ƽ
};

#define KEY_NUM_MAX (sizeof(key_gpio)/sizeof(sizeof(key_gpio[0])))	// ��������

#define CONFIRM_TIME	20 		// ����ʱ��20ms
#define LONG_PRESS_TIME 500		// ����ʱ��500ms
#define SHORT_RELEASE_VALID	1	// 1���̰��ɿ�ʱ��Ч��0���̰�����ʱ��Ч
#define LONG_RELEASE_VALID	1	// 1�������ɿ�ʱ��Ч��0����������ʱ��Ч

key_param_t key_param[KEY_NUM_MAX];		/* ������а��������� */

/**
  * @brief  ��ȡ�����Ƿ���
  * @param	index��key_param�ṹ���Ӧ�����±�
  * @retval 1�����£�0��δ����
  */
static uint8_t read_key_state(uint8_t index)
{
	if(HAL_GPIO_ReadPin(key_gpio[index].port, key_gpio[index].pin) == key_gpio[index].pressed_state)
		return 1;
	return 0;
}

/**
  * @brief  ����ɨ��ÿ������״̬
  * @param	index��key_param�ṹ���Ӧ�����±�
  * @retval void
  */
void key_scan(void)
{
	static uint8_t index, isKeyPress;
	static uint32_t pressed_time;
	static key_state_t current_state;
	static key_event_t key_event;
	for(index = 0; index < KEY_NUM_MAX; index++)
	{
		isKeyPress = read_key_state(index);				// ��ȡ�����Ƿ���
		current_state = key_param[index].current_state;	// ��ȡ����״̬������״̬
		pressed_time  = key_param[index].pressed_time;	// ��ȡ�������µ�ʱ��
		key_event = key_param[index].key_event;			// ��ȡ�������¼�
		switch(current_state)
		{
			/* ״̬һ�����״̬ */
			case KEY_CHECK:		
				if(isKeyPress)	// ��������
				{
					current_state = KEY_CONFIRM; 	// ��������ȷ��
				}
				else	// �����ɿ�
				{
					pressed_time = 0;
				}				
				break;
			/* ״̬��������ȷ�� */
			case KEY_CONFIRM:		
				if(isKeyPress)	// ��Ȼ����
				{
					if(++pressed_time > CONFIRM_TIME)	// ����ʱ�� 
					{
						current_state = KEY_SHORT_PRESSED; 	// ����̰�״̬
						#if (!SHORT_RELEASE_VALID)	// �̰�����������Ч
							key_event = EVENT_SHORT_PRESSED;	// �̰��¼�
						#endif						
					}
				}
				else	// ��������ǰ�ɿ���(δ���� ������Ч)
				{
					current_state = KEY_CHECK;	// ������һ��״̬
				}		
				break;
			/* ״̬�����̰�״̬ */
			case KEY_SHORT_PRESSED:	
				if(isKeyPress)	// ��Ȼ����
				{
					if(++pressed_time > LONG_PRESS_TIME)	// ����ʱ��
					{
						current_state = KEY_LONG_PRESSED;
						#if (!SHORT_RELEASE_VALID)	// ��������������Ч
							key_event = EVENT_LONG_PRESSED;	// �����¼�
						#endif		
					}
				}
				else	// �����ɿ���
				{
					current_state = KEY_CHECK;			// ���ؼ��״̬
					#if (SHORT_RELEASE_VALID)	// �̰��ɿ���Ч
						key_event = EVENT_SHORT_PRESSED;	// �̰��¼�
					#endif	
				}
				break;
			/* ״̬�ģ�����״̬ */
			case KEY_LONG_PRESSED:	
				if(!isKeyPress)	// �����ɿ���
				{
					current_state = KEY_CHECK;			// ���ؼ��״̬
					#if (SHORT_RELEASE_VALID)	// �����ɿ���Ч
						key_event = EVENT_LONG_PRESSED;	// �����¼�
					#endif	
				}					
				break;
			default:
				current_state = KEY_CHECK;			// Ĭ�ϼ��״̬
				break;	
		}
		key_param[index].current_state = current_state;	// ����ṹ��
		key_param[index].pressed_time = pressed_time;	// ����ṹ��
		key_param[index].key_event = key_event;			// ����ṹ��
	}
}
	
/**
  * @brief  �������Ժ���
  * @param	void
  * @retval void
  */
void key_handle(void)
{
	key_scan();
	HAL_Delay(1);
	
	static uint8_t index;
	static key_event_t key_event;
	for(index = 0; index < KEY_NUM_MAX; index++)
	{
		key_event = key_param[index].key_event;
		//printf("key_event:%d\r\n", key_event);
		if(key_event != 0)	// ���¼�����
		{
			switch(index)
			{
				case 0:		// ����KEY0
					if(key_event == EVENT_SHORT_PRESSED)
					{
						printf("KEY0 SHORT PRESSED\r\n");
					}
					else if(key_event == EVENT_LONG_PRESSED)
					{
						printf("KEY0 LONG PRESSED\r\n");
					}
					break;
				case 1:		// ����KEY1
					if(key_event == EVENT_SHORT_PRESSED)
					{
						printf("KEY1 SHORT PRESSED\r\n");
					}
					else if(key_event == EVENT_LONG_PRESSED)
					{
						printf("KEY1 LONG PRESSED\r\n");
					}
					break;
				case 2:		// ����KEY_WP_UP
					if(key_event == EVENT_SHORT_PRESSED)
					{
						printf("KEY_WP_UP SHORT PRESSED\r\n");
					}
					else if(key_event == EVENT_LONG_PRESSED)
					{
						printf("KEY_WP_UP LONG PRESSED\r\n");
					}
					break;
				default:			
					break;
			}
			key_param[index].key_event = EVENT_NULL; 	// ����¼�
		}
	}
}












