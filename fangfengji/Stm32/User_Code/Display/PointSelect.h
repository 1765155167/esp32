#ifndef _POINT_SELECT_H_
#define _POINT_SELECT_H_

#include "global.h"
#include "PointSelect_Struct.h"
#include "Dat.h"

void ScreenConfiguration(void);//����ʾ��ʼ��
void DisplayScanf(void);//����ʾɨ��
void Display12x16(u8 s,u8 page,u8 y,u8* data);//����ʾ12X16�ĺ���

void DisplayColon(u8 s,u8 page,u8 y);//����ʾð��
void Display12x8Char(u8 s,u8 page,u8 y,u8 data);//����ʾ12X8������
void DisplayPoint(u8 set);//����ʾ��
void DisplayLine(u8 s,u8 page,u8 y,u8 len,u8 loc);//����ֱ��


#endif

