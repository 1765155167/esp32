#ifndef _DISPLAYLIST_Struct_H_
#define _DISPLAYLIST_Struct_H_

//段码屏显示参数结构体
typedef struct DISPLAYLIST_PARAMETER
{
  u8 Pag;//显示页号
  u8 FlagChange;//数据发送变化
  u8 FlagPagChange;//页号发生变化
  
  //页面0显示的数据
  u8 Pag0RopeNum;//风口号
  u8 Pag0RopeLength;//风口长度
  u8 CurTemp;//当前温度
  u8 TempTrend;//温度趋势
  u8 MAXTemp;//温度上限
  u8 MINTemp;//温度下限
}DisplayList_Parameter_Struct;

#endif



