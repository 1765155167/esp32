#ifndef _TIME3_H_
#define _TIME3_H_

#include "global.h"
#include "Time3_Struct.h"

extern Time3_Parameter_Struct Time3_Parameter;//Time3控制参数结构体

void Tim3_Interrupt(void);//Timer3中断处理函数	周期100us

#endif

