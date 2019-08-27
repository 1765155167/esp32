#ifndef _POINTSELECT_Struct_H_
#define _POINTSELECT_Struct_H_


#define SEGMENT_NUM          4      //段选IO数量
#define BIT_NUM              17     //位选IO数量


//段码屏段选位选控制参数结构体
typedef struct POINTSELECT_PARAMETER
{
  u8 RefreshTime;//段扫描间隔时间 单位:100ns
  u8 DisplayBit;//当前显示位号

  u8 BitNum;//段IO数量
  u8 BitGroup[SEGMENT_NUM];//位引脚组名
  u8 BitPin[SEGMENT_NUM];//位引脚IP号
  
  u8 SegmentNum;//段IO数量
  u8 SegmentGroup[SEGMENT_NUM];//段引脚组名
  u8 SegmentPin[SEGMENT_NUM];//段引脚IO号
  
  u8 BitLevel[SEGMENT_NUM][BIT_NUM];//各段位选电平
}PointSelect_Parameter_Struct;

extern PointSelect_Parameter_Struct PointSelect_Parameter;//PointSelect控制参数结构体

#endif
