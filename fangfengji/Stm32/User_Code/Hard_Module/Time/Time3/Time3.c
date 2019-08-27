#include "Time3.h"


Time3_Parameter_Struct Time3_Parameter;//Time3控制参数结构体


/****************************************************************************
* 名    称：Tim3_Interrupt
* 功    能：Timer3中断处理函数	周期100ns
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Tim3_Interrupt(void)
{
  Time3_Parameter.flag_dog++;//看门狗喂狗标记
  Time3_Parameter.flag_display++;//段码屏扫描标记
}

