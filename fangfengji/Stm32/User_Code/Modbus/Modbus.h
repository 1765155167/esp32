#ifndef _CRC16_H
#define _CRC16_H

#include "global.h"


//����Ĵ���
extern u8 Register[255];
extern u8 ModbusTXBuf[255];
extern u8 ChangeData;//���ݸı���

u8 DisplayF0NumGet(u8 DisplayType);//��ȡ��ҳ����ʾ������
u8* FrameDeal(u8* FrameData);//����֡����
u8 GetFrameLen(u8* FrameData);//��ȡ�ظ�����֡�ĳ���
  


//�Ĵ�����ַ
#define	DeviceID        0x00        //�豸ID
#define	DisplayE        0x02        //��ʾʹ��
#define RopeNum_H       0x04        //��ں�H
#define RopeNum_L       0x05        //��ں�L
#define RopeLen_H       0x06        //��ڳ���H
#define RopeLen_L       0x07        //��ڳ���L
#define CurTemp_H       0x08        //��ǰ�¶�H
#define CurTemp_L       0x09        //��ǰ�¶�L
#define TemTrend        0x0a        //�¶�����
#define TemH_H          0x0c        //�¶�����H
#define TemH_L          0x0d        //�¶�����L
#define TemL_H          0x0e        //�¶�����H
#define TemL_L          0x0f        //�¶�����L
#define MotoSta         0x10        //���״̬
#define CtrlMode        0x12        //����ģʽ

//��ʾ���
#define D_RopeNum_H       0x00        //��ں�H
#define D_RopeNum_L       0x01        //��ں�L
#define D_RopeLen_H       0x02        //��ڳ���H
#define D_RopeLen_M       0x03        //��ڳ���M
#define D_RopeLen_L       0x04        //��ڳ���L
#define D_CurTemp_H       0x05        //��ǰ�¶�H
#define D_CurTemp_L       0x06        //��ǰ�¶�L
#define D_TemTrend        0x07        //�¶�����
#define D_TemH_H          0x08        //�¶�����H
#define D_TemH_L          0x09        //�¶�����L
#define D_TemL_H          0x0A        //�¶�����H
#define D_TemL_L          0x0B        //�¶�����L
#define D_MotoSta         0x0C        //���״̬
#define D_CtrlMode        0x0D        //����ģʽ

#endif

