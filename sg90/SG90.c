#include "SG90.h"

/**
 * @brief ����SG90����ĽǶȡ�
 * 
 * �ú������ݽǶȲ���������Ӧ�������ȣ�
 * Ȼ������TIM2��ʱ����ָ��ͨ���ϵ�PWM�ź������������ת��ָ���Ƕȡ�
 * 
 * @param angle Ҫ���õĶ���Ƕȣ���λΪ�ȣ�0-180�㣩��
 */
void SG90_SetAngle(uint8_t target_angle) {
    // ���ƶ���Ƕ��� 0 �� 180 ��֮��
    if (target_angle > 180) {
        target_angle = 180;
    } else if (target_angle < 0) {
        target_angle = 0;
    }

    // ��̬�������浱ǰ�Ƕ�
    static uint8_t current_angle = 0;

    // ���㲽���ͷ���
    int step = (target_angle > current_angle) ? 1 : -1;

    // ʹ�� for ѭ���𲽸ı�Ƕ�
    for (uint8_t angle = current_angle; angle != target_angle; angle += step) {
        // ���������ȣ����Ƕ�ת��Ϊ�����ȣ�ʹ�����Է���
        uint16_t pulse = (angle * 20) + 500;

        // ���� TIM2 ��ʱ��ͨ�� 2 �ıȽ�ֵ
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);

        // ���� TIM2 ��ʱ��ͨ�� 2 �� PWM �ź����������
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

        // ���һ��С�ӳ٣��Ա�۲쵽�Ƕȱ仯
        HAL_Delay(1); // �ӳ� 10 ����
    }

    // // ��������Ŀ��Ƕ�
    // uint16_t pulse = (target_angle * 20) + 500;
    // __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
    // HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    // ���µ�ǰ�Ƕ�
    current_angle = target_angle;
}
