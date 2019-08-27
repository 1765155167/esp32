#ifndef _POINT_SELECT_H_
#define _POINT_SELECT_H_

#include "global.h"
#include "PointSelect_Struct.h"
#include "Dat.h"

void ScreenConfiguration(void);//屏显示初始化
void DisplayScanf(void);//屏显示扫描
void Display12x16(u8 s,u8 page,u8 y,u8* data);//屏显示12X16的汉字

void DisplayColon(u8 s,u8 page,u8 y);//屏显示冒号
void Display12x8Char(u8 s,u8 page,u8 y,u8 data);//屏显示12X8的数字
void DisplayPoint(u8 set);//屏显示点
void DisplayLine(u8 s,u8 page,u8 y,u8 len,u8 loc);//绘制直线


#endif

