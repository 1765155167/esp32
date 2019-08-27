#ifndef _CRC16_H
#define _CRC16_H

#include "global.h"


//虚拟寄存器
extern u8 Register[255];
extern u8 ModbusTXBuf[255];
extern u8 ChangeData;//数据改变标记

u8 DisplayF0NumGet(u8 DisplayType);//获取主页面显示的数据
u8* FrameDeal(u8* FrameData);//数据帧处理
u8 GetFrameLen(u8* FrameData);//获取回复数据帧的长度
  


//寄存器地址
#define	DeviceID        0x00        //设备ID
#define	DisplayE        0x02        //显示使能
#define RopeNum_H       0x04        //风口号H
#define RopeNum_L       0x05        //风口号L
#define RopeLen_H       0x06        //风口长度H
#define RopeLen_L       0x07        //风口长度L
#define CurTemp_H       0x08        //当前温度H
#define CurTemp_L       0x09        //当前温度L
#define TemTrend        0x0a        //温度趋势
#define TemH_H          0x0c        //温度上限H
#define TemH_L          0x0d        //温度上限L
#define TemL_H          0x0e        //温度下限H
#define TemL_L          0x0f        //温度下限L
#define MotoSta         0x10        //电机状态
#define CtrlMode        0x12        //控制模式

//显示编号
#define D_RopeNum_H       0x00        //风口号H
#define D_RopeNum_L       0x01        //风口号L
#define D_RopeLen_H       0x02        //风口长度H
#define D_RopeLen_M       0x03        //风口长度M
#define D_RopeLen_L       0x04        //风口长度L
#define D_CurTemp_H       0x05        //当前温度H
#define D_CurTemp_L       0x06        //当前温度L
#define D_TemTrend        0x07        //温度趋势
#define D_TemH_H          0x08        //温度上限H
#define D_TemH_L          0x09        //温度上限L
#define D_TemL_H          0x0A        //温度下限H
#define D_TemL_L          0x0B        //温度下限L
#define D_MotoSta         0x0C        //电机状态
#define D_CtrlMode        0x0D        //控制模式

#endif

