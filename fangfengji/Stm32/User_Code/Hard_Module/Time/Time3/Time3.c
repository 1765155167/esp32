#include "Time3.h"


Time3_Parameter_Struct Time3_Parameter;//Time3���Ʋ����ṹ��


/****************************************************************************
* ��    �ƣ�Tim3_Interrupt
* ��    �ܣ�Timer3�жϴ�����	����100ns
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void Tim3_Interrupt(void)
{
  Time3_Parameter.flag_dog++;//���Ź�ι�����
  Time3_Parameter.flag_display++;//������ɨ����
}

