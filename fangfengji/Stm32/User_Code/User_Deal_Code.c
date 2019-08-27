#include "User_Deal_Code.h"

/****************************************************************************
* 名    称：System_Init(void)
* 功    能：外部设备的初始化和启用
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void System_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim3);//启动定时器3
  HAL_TIM_Base_Start_IT(&htim6);//启动定时器6
  USART1_Configuration();
  USART2_Configuration();
}

/****************************************************************************
* 名    称：System_Ready(void)
* 功    能：在进入大循环前，参数初始化配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void System_Ready(void)
{
  //printf("system start\r\n");
  DisplayInit();
  ScanfDisplayF0(1);
}

/****************************************************************************
* 名    称：void Main_Loop(void)
* 功    能：主函数大循环
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Main_Loop(void)
{	
  System_Init();//外部设备的初始化和启用
  System_Ready();//在进入大循环前，参数初始化配置

  while(1)
  {
    /********************01.5s喂一次狗********************/
    if(Time3_Parameter.flag_dog >= 10000)    
    {
      Time3_Parameter.flag_dog = Time3_Parameter.flag_dog - 10000;
      HAL_IWDG_Refresh(&hiwdg);//喂狗
    }
		if(0)
		{
			
			printf("Hello 。。。。。。。\n");
			
		}
    /********************02.段码屏扫描********************/
    if(Time3_Parameter.flag_display >= 1000)    
    {
      Time3_Parameter.flag_display = Time3_Parameter.flag_display - 1000;
      if(ChangeData == 1)
      {
        ChangeData = 0;
        ScanfDisplayF0(0);
      }
    }
    
  }
}

