#include "DisplayList.h"

/****************************私有宏定义*******************************/

/****************************私有函数声明*****************************/

/****************************私有全局变量定义*************************/

/****************************公有全局变量定义*************************/
DisplayList_Parameter_Struct DisplayList_Parameter;//DisplayList控制参数结构体

/****************************私有函数*********************************/


/****************************公有函数*********************************/

/****************************************************************************
* 名    称：DisplayInit
* 功    能：显示初始化
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void DisplayInit(void)
{
  ScreenConfiguration();//屏显示初始化
  ScanfDisplayF0(1);
}
/****************************************************************************
* 名    称：vScanfDisplayF0
* 功    能：段码表扫描显示页数1
* 入口参数：u8 Area:1全部刷新 0，刷新数据
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void ScanfDisplayF0(u8 Area)
{
    u8 temp = 0;
    DisplayPoint(2);
    
    if(Area == 1)
    {
      DisplayLine(0,1,3,61,2);
      DisplayLine(1,1,0,61,2);
      Display12x16(0,0,4,"风");
      Display12x16(0,0,16,"口");
      DisplayColon(0,0,30);
    }
    Display12x8Char(0,0,34,DisplayF0NumGet(D_RopeNum_H));
    Display12x8Char(0,0,41,DisplayF0NumGet(D_RopeNum_L));
    if(Area == 1)
    {
      Display12x16(0,0,51,"开");
      Display12x16(1,0,0,"度");
      DisplayColon(1,0,15);
    }
    Display12x8Char(1,0,19,DisplayF0NumGet(D_RopeLen_H));
    Display12x8Char(1,0,26,DisplayF0NumGet(D_RopeLen_M));
    Display12x8Char(1,0,33,DisplayF0NumGet(D_RopeLen_L));
    if(Area == 1)
    {
      Display12x8Char(1,0,44,'C');
      Display12x8Char(1,0,52,'M');
    }  
    
    DisplayPoint(0);
    if(Area == 1)
    {
      Display12x16(0,2,4,"大");
      Display12x16(0,2,16,"棚");
    }
    DisplayPoint(2);
    if(Area == 1)
    {
      DisplayLine(0,5,3,61,2);
      DisplayLine(1,5,0,61,2);
      Display12x16(0,4,4,"温");
      Display12x16(0,4,16,"度");
    }
    DisplayPoint(0);
    Display12x8Char(0,3,34,DisplayF0NumGet(D_CurTemp_H));
    Display12x8Char(0,3,41,DisplayF0NumGet(D_CurTemp_L));
    if(Area == 1)
    {
      Display12x16(0,3,49,"℃");
      Display12x16(1,2,0,"上");
      Display12x16(1,2,12,"限");
      DisplayColon(1,2,26);
      Display12x16(1,2,45,"℃");
    }
    Display12x8Char(1,2,29,DisplayF0NumGet(D_TemH_H));
    Display12x8Char(1,2,36,DisplayF0NumGet(D_TemH_L));
      
    DisplayPoint(2);
    if(Area == 1)
    {
      Display12x16(1,4,0,"下");
      Display12x16(1,4,12,"限");
      DisplayColon(1,4,26);
      Display12x16(1,4,45,"℃");
    }
      Display12x8Char(1,4,29,DisplayF0NumGet(D_TemL_H));
      Display12x8Char(1,4,36,DisplayF0NumGet(D_TemL_L));
      
    
    DisplayPoint(0);
    if(Area == 1)
    {
      Display12x16(0,6,4,"电");
      Display12x16(0,6,16,"机");
      DisplayColon(0,6,30);
    }
    temp = DisplayF0NumGet(D_MotoSta);
    if(temp == 0x01)
    {
      Display12x16(0,6,34,"正");
      Display12x16(0,6,46,"转");
    }
    else if(temp == 0x02)
    {
      Display12x16(0,6,34,"反");
      Display12x16(0,6,46,"转");
    }
    else 
    {
      Display12x16(0,6,34,"停");
      Display12x16(0,6,46,"止");
    }  
    if(Area == 1)
    {
      Display12x16(1,6,0,"模");
      Display12x16(1,6,12,"式");
      DisplayColon(1,6,26);
    }
    temp = DisplayF0NumGet(D_CtrlMode);
    if(temp == 0x01)
    {
      Display12x16(1,6,29,"自");
      Display12x16(1,6,41,"动");
    }
    else 
    {
      Display12x16(1,6,29,"手");
      Display12x16(1,6,41,"动");
    }  
}


