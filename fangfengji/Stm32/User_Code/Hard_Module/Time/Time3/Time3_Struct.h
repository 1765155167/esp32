#ifndef _TIME3_Struct_H_
#define _TIME2_Struct_H_

//Time3控制参数结构体
typedef struct TIME3_PARAMETER
{
  volatile unsigned int flag_dog;//看门狗喂狗标记
  volatile unsigned int flag_display;//段码屏扫描标记
}Time3_Parameter_Struct;

#endif
