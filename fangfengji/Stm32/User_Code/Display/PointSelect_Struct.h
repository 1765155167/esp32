#ifndef _POINTSELECT_Struct_H_
#define _POINTSELECT_Struct_H_


#define SEGMENT_NUM          4      //��ѡIO����
#define BIT_NUM              17     //λѡIO����


//��������ѡλѡ���Ʋ����ṹ��
typedef struct POINTSELECT_PARAMETER
{
  u8 RefreshTime;//��ɨ����ʱ�� ��λ:100ns
  u8 DisplayBit;//��ǰ��ʾλ��

  u8 BitNum;//��IO����
  u8 BitGroup[SEGMENT_NUM];//λ��������
  u8 BitPin[SEGMENT_NUM];//λ����IP��
  
  u8 SegmentNum;//��IO����
  u8 SegmentGroup[SEGMENT_NUM];//����������
  u8 SegmentPin[SEGMENT_NUM];//������IO��
  
  u8 BitLevel[SEGMENT_NUM][BIT_NUM];//����λѡ��ƽ
}PointSelect_Parameter_Struct;

extern PointSelect_Parameter_Struct PointSelect_Parameter;//PointSelect���Ʋ����ṹ��

#endif
