#include "global.h"

unsigned char version_number[28] = "booting from V1.00\r";

/****************************************************************************
* 名    称：Stm32_Reboot_system
* 功    能：系统重启
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Stm32_Reboot_system(void)
{
  HAL_Delay(1000);
  NVIC_SystemReset();//软件复位
}


