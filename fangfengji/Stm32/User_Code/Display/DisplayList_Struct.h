#ifndef _DISPLAYLIST_Struct_H_
#define _DISPLAYLIST_Struct_H_

//��������ʾ�����ṹ��
typedef struct DISPLAYLIST_PARAMETER
{
  u8 Pag;//��ʾҳ��
  u8 FlagChange;//���ݷ��ͱ仯
  u8 FlagPagChange;//ҳ�ŷ����仯
  
  //ҳ��0��ʾ������
  u8 Pag0RopeNum;//��ں�
  u8 Pag0RopeLength;//��ڳ���
  u8 CurTemp;//��ǰ�¶�
  u8 TempTrend;//�¶�����
  u8 MAXTemp;//�¶�����
  u8 MINTemp;//�¶�����
}DisplayList_Parameter_Struct;

#endif



