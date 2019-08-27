#ifndef __GLOBAL_H
#define __GLOBAL_H 	

#include "main.h"
#include "stm32f0xx_hal.h"
#include "dma.h"
#include "tim.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"

#include "User_Deal_Code.h"//用户代码
#include "Usart1.h"//串口1
#include "Usart2.h"//串口2
#include "Usart_Magagement.h"//串口回调函数统一管理
#include "Time3.h"//定时器3
#include "Time6.h"
#include "Time_Magagement.h"//定时器回调函数统一管理
#include "TypeDef.h"//数据类型定义
#include "gpio_c.h"
#include "PointSelect.h"
#include "DisplayList.h"
#include "Modbus.h"


extern unsigned char version_number[];
void Stm32_Reboot_system(void);//系统重启
			
#endif

/****************************私有宏定义*******************************/

/****************************私有函数声明*****************************/

/****************************私有全局变量定义*************************/

/****************************公有全局变量定义*************************/

/****************************私有函数*********************************/

/****************************公有函数*********************************/

