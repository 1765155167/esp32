#include "Time_Magagement.h"

/****************************************************************************
* 名    称：HAL_TIM_PeriodElapsedCallback
* 功    能：CUBE生成的初始化程序的定时器回调函数
* 入口参数：unsigned char num:灯的编号,unsigned char flag：0代表开，1代表关
* 出口参数：TIM_HandleTypeDef *htim：当前调用该回调函数的定时器的结构体
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   /**************************定时器3调用了该回调函数************************************/
   if( htim->Instance == TIM3)
   {
     Tim3_Interrupt();//Timer3中断处理函数周期100ns
   }
   /**************************定时器6调用了该回调函数************************************/
   if( htim->Instance == TIM6)
   {
     Tim6_Interrupt();//Timer3中断处理函数周期1ms
   }
}

