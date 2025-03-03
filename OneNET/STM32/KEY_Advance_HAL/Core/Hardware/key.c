#include "main.h"
#include "key.h"

/* ֧�ֳ��̰� 
   Ŀǰ�������⣺�޷����������ͬʱ����
*/

/* �����Ƿ�Ҫ�г��� ����ͬʱע�ͻ����*/
#define LongKeyEvent 1
//#define ShortKeyEvent 1

#define KEY0   	HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)		// ���µ͵�ƽ
#define KEY1 	HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)		// ���µ͵�ƽ
#define KEY_UP 	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)		// ���¸ߵ�ƽ

#define isKeyDown (!KEY0 || !KEY1 || KEY_UP)	// 1��ʾ���£�0��ʾû�а���

KEY_VALUE KeyValue = KEY_NULL;		 // ��ʼ������ֵ
KEY_STATE KeyState = KEY_CHECK;		 // ��ʼ������״̬Ϊ���״̬
KEY_TYPE  KeyType = NULL_KEY;		 // ��ʼ����������Ϊ��
uint8_t g_KeyFlag = 0;               // ������Ч��־��0�� ����ֵ��Ч�� 1������ֵ��Ч


/* ��������޳��� */
#if ShortKeyEvent
void Key_Scan(void)
{
	switch(KeyState)
	{
		case KEY_CHECK:
			if(isKeyDown)	// �Ƿ��а�������
			{
				KeyState = KEY_CONFIRM;	// ������һ��״̬
			}
			break;
		case KEY_CONFIRM:
			if(isKeyDown)	// �Ƿ�һֱ���� ����
			{
				KeyState = KEY_RELEASE;	// ������һ��״̬				
			}
			else
			{
				KeyState = KEY_CHECK;	// ������һ��״̬
			}			
			break;
		case KEY_RELEASE:
			if(!isKeyDown)	// �����ɿ�
			{
				KeyState = KEY_CHECK;	// ���ؿ�ʼ״̬
				g_KeyFlag = 1;
			}				
			break;
		default:
			break;		
	}	
}
/* ��������г��� */
#elif defined(LongKeyEvent)
void Key_Scan(void)
{
	static uint8_t count, isFirstEntry;
	switch(KeyState)
	{
		case KEY_CHECK:
			if(isKeyDown)	// �Ƿ��а�������
			{
				KeyState = KEY_CONFIRM;	// ������һ��״̬
				count = 0;
				isFirstEntry = 0;
			}
			break;
		case KEY_CONFIRM:
			if(isKeyDown)	// �Ƿ�һֱ���� ����
			{
				if(!isFirstEntry) 
					isFirstEntry = 1;
				count++;		
				/* �ж����ĸ��������µ� */
				if(!KEY0)
					KeyValue = KEY_0;
				else if(!KEY1)
					KeyValue = KEY_1;
				else if(KEY_UP)
					KeyValue = KEY_WK_UP;		
			}
			else
			{
				if(isFirstEntry)  	// ���ǵ�һ�ν��� 
				{
					KeyState = KEY_RELEASE;	// ������һ��״̬	
				}					
				else			
					KeyState = KEY_CHECK;	// ������һ��״̬
			}			
			break;
		case KEY_RELEASE:
			if(!isKeyDown)	// �����ɿ�
			{
				if(count > 80) // ��ʱ����80���ж�Ϊ����
				{
					KeyType = LONG_KEY;	// ��������Ϊ����
					count = 0;
					isFirstEntry = 0;
				}	
				else
				{
					KeyType = SHORT_KEY;	// ��������Ϊ�̰�
				}	
				KeyState = KEY_CHECK;	// ���ؿ�ʼ״̬
			}				
			break;
		default:
			break;		
	}	
}
#endif



