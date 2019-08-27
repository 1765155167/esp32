#include "DisplayList.h"

/****************************˽�к궨��*******************************/

/****************************˽�к�������*****************************/

/****************************˽��ȫ�ֱ�������*************************/

/****************************����ȫ�ֱ�������*************************/
DisplayList_Parameter_Struct DisplayList_Parameter;//DisplayList���Ʋ����ṹ��

/****************************˽�к���*********************************/


/****************************���к���*********************************/

/****************************************************************************
* ��    �ƣ�DisplayInit
* ��    �ܣ���ʾ��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void DisplayInit(void)
{
  ScreenConfiguration();//����ʾ��ʼ��
  ScanfDisplayF0(1);
}
/****************************************************************************
* ��    �ƣ�vScanfDisplayF0
* ��    �ܣ������ɨ����ʾҳ��1
* ��ڲ�����u8 Area:1ȫ��ˢ�� 0��ˢ������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void ScanfDisplayF0(u8 Area)
{
    u8 temp = 0;
    DisplayPoint(2);
    
    if(Area == 1)
    {
      DisplayLine(0,1,3,61,2);
      DisplayLine(1,1,0,61,2);
      Display12x16(0,0,4,"��");
      Display12x16(0,0,16,"��");
      DisplayColon(0,0,30);
    }
    Display12x8Char(0,0,34,DisplayF0NumGet(D_RopeNum_H));
    Display12x8Char(0,0,41,DisplayF0NumGet(D_RopeNum_L));
    if(Area == 1)
    {
      Display12x16(0,0,51,"��");
      Display12x16(1,0,0,"��");
      DisplayColon(1,0,15);
    }
    Display12x8Char(1,0,19,DisplayF0NumGet(D_RopeLen_H));
    Display12x8Char(1,0,26,DisplayF0NumGet(D_RopeLen_M));
    Display12x8Char(1,0,33,DisplayF0NumGet(D_RopeLen_L));
    if(Area == 1)
    {
      Display12x8Char(1,0,44,'C');
      Display12x8Char(1,0,52,'M');
    }  
    
    DisplayPoint(0);
    if(Area == 1)
    {
      Display12x16(0,2,4,"��");
      Display12x16(0,2,16,"��");
    }
    DisplayPoint(2);
    if(Area == 1)
    {
      DisplayLine(0,5,3,61,2);
      DisplayLine(1,5,0,61,2);
      Display12x16(0,4,4,"��");
      Display12x16(0,4,16,"��");
    }
    DisplayPoint(0);
    Display12x8Char(0,3,34,DisplayF0NumGet(D_CurTemp_H));
    Display12x8Char(0,3,41,DisplayF0NumGet(D_CurTemp_L));
    if(Area == 1)
    {
      Display12x16(0,3,49,"��");
      Display12x16(1,2,0,"��");
      Display12x16(1,2,12,"��");
      DisplayColon(1,2,26);
      Display12x16(1,2,45,"��");
    }
    Display12x8Char(1,2,29,DisplayF0NumGet(D_TemH_H));
    Display12x8Char(1,2,36,DisplayF0NumGet(D_TemH_L));
      
    DisplayPoint(2);
    if(Area == 1)
    {
      Display12x16(1,4,0,"��");
      Display12x16(1,4,12,"��");
      DisplayColon(1,4,26);
      Display12x16(1,4,45,"��");
    }
      Display12x8Char(1,4,29,DisplayF0NumGet(D_TemL_H));
      Display12x8Char(1,4,36,DisplayF0NumGet(D_TemL_L));
      
    
    DisplayPoint(0);
    if(Area == 1)
    {
      Display12x16(0,6,4,"��");
      Display12x16(0,6,16,"��");
      DisplayColon(0,6,30);
    }
    temp = DisplayF0NumGet(D_MotoSta);
    if(temp == 0x01)
    {
      Display12x16(0,6,34,"��");
      Display12x16(0,6,46,"ת");
    }
    else if(temp == 0x02)
    {
      Display12x16(0,6,34,"��");
      Display12x16(0,6,46,"ת");
    }
    else 
    {
      Display12x16(0,6,34,"ͣ");
      Display12x16(0,6,46,"ֹ");
    }  
    if(Area == 1)
    {
      Display12x16(1,6,0,"ģ");
      Display12x16(1,6,12,"ʽ");
      DisplayColon(1,6,26);
    }
    temp = DisplayF0NumGet(D_CtrlMode);
    if(temp == 0x01)
    {
      Display12x16(1,6,29,"��");
      Display12x16(1,6,41,"��");
    }
    else 
    {
      Display12x16(1,6,29,"��");
      Display12x16(1,6,41,"��");
    }  
}


