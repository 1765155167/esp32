#include "Time_Magagement.h"

/****************************************************************************
* ��    �ƣ�HAL_TIM_PeriodElapsedCallback
* ��    �ܣ�CUBE���ɵĳ�ʼ������Ķ�ʱ���ص�����
* ��ڲ�����unsigned char num:�Ƶı��,unsigned char flag��0������1�����
* ���ڲ�����TIM_HandleTypeDef *htim����ǰ���øûص������Ķ�ʱ���Ľṹ��
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   /**************************��ʱ��3�����˸ûص�����************************************/
   if( htim->Instance == TIM3)
   {
     Tim3_Interrupt();//Timer3�жϴ���������100ns
   }
   /**************************��ʱ��6�����˸ûص�����************************************/
   if( htim->Instance == TIM6)
   {
     Tim6_Interrupt();//Timer3�жϴ���������1ms
   }
}

