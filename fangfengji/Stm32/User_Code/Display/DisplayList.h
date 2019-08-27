#ifndef _DISPLAY_LIST_H_
#define _DISPLAY_LIST_H_

#include "global.h"
#include "DisplayList_Struct.h"

extern DisplayList_Parameter_Struct DisplayList_Parameter;//DisplayList控制参数结构体

void DisplayInit(void);//显示初始化
void ScanfDisplayF0(u8 Area);//段码表扫描显示页数1
#endif

