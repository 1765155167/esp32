#include "User_Deal_Code.h"

/****************************************************************************
* ��    �ƣ�System_Init(void)
* ��    �ܣ��ⲿ�豸�ĳ�ʼ��������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void System_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim3);//������ʱ��3
  HAL_TIM_Base_Start_IT(&htim6);//������ʱ��6
  USART1_Configuration();
  USART2_Configuration();
}

/****************************************************************************
* ��    �ƣ�System_Ready(void)
* ��    �ܣ��ڽ����ѭ��ǰ��������ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void System_Ready(void)
{
  //printf("system start\r\n");
  DisplayInit();
  ScanfDisplayF0(1);
}

/****************************************************************************
* ��    �ƣ�void Main_Loop(void)
* ��    �ܣ���������ѭ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void Main_Loop(void)
{	
  System_Init();//�ⲿ�豸�ĳ�ʼ��������
  System_Ready();//�ڽ����ѭ��ǰ��������ʼ������

  while(1)
  {
    /********************01.5sιһ�ι�********************/
    if(Time3_Parameter.flag_dog >= 10000)    
    {
      Time3_Parameter.flag_dog = Time3_Parameter.flag_dog - 10000;
      HAL_IWDG_Refresh(&hiwdg);//ι��
    }
		if(0)
		{
			
			printf("Hello ��������������\n");
			
		}
    /********************02.������ɨ��********************/
    if(Time3_Parameter.flag_display >= 1000)    
    {
      Time3_Parameter.flag_display = Time3_Parameter.flag_display - 1000;
      if(ChangeData == 1)
      {
        ChangeData = 0;
        ScanfDisplayF0(0);
      }
    }
    
  }
}

