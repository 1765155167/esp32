#include "PointSelect.h"
//#include "intrins.h"
/****************************私有宏定义*******************************/

/****************************私有函数声明*****************************/
static void GpioSet(u8 data);//12864数据输出IO
static void WriteCmd(u8 cmd);//12864指令写入
static void WriteData(u8 data);//12864数据写入
static void SelectScreen(u8 Screen);//12864左右屏幕选择
static void SetOnOff(u8 onoff);//12864显示开关  
static void Setpage(u8 page);//12864页设置   
static void Setx(u8 x);//12864起始行设置 



/****************************私有全局变量定义*************************/

const unsigned  char GB_12[] =        
{
    0x80,0x88,0x88,0x88,0xF8,0x88,0x88,0x88,
    0xF8,0x88,0x88,0x80,0x00,0x00,0x20,0x10,
    0x0F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,//开0

    0x00,0x00,0xF8,0x28,0x48,0x88,0x48,0x28,
    0x08,0xF8,0x00,0x00,0x20,0x18,0x07,0x08,
    0x06,0x01,0x06,0x08,0x00,0x1F,0x20,0x38,//风1
};

static u8 SetPoint = 0;
/****************************公有全局变量定义*************************/

PointSelect_Parameter_Struct PointSelect_Parameter;//PointSelect控制参数结构体

/****************************私有函数*********************************/

/****************************************************************************
* 名    称：GpioSet
* 功    能：12864数据输出IO
* 入口参数：u8 data：IO口输出的数据
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void GpioSet(u8 data)
{
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('C',15,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('C',15,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',15,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',15,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',4,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',4,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',8,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',8,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',5,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',5,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',12,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',12,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',6,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',6,1);
  }
  
  data = data >> 1;
  
  if( (data & 1) == 0)
  {
    Gpio_Pin_Reset_Set('A',11,0);
  }
  else
  {
    Gpio_Pin_Reset_Set('A',11,1);
  }
}

/****************************************************************************
* 名    称：WriteCmd
* 功    能：12864指令写入
* 入口参数：u8 cmd：写入的指令
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void WriteCmd(u8 cmd)
{
  Gpio_Pin_Reset_Set('B',4,0);
  Gpio_Pin_Reset_Set('C',14,0);
  
  GpioSet(cmd);
  
  Gpio_Pin_Reset_Set('B',3,1);
	__nop();__nop();__nop();
  //__nop();__nop();__nop();
  Gpio_Pin_Reset_Set('B',3,0); 
  //__nop();__nop();__nop();
	__nop();__nop();__nop();
}

/****************************************************************************
* 名    称：WriteData
* 功    能：12864数据写入
* 入口参数：u8 data：写入的数据
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void WriteData(u8 data)
{
  Gpio_Pin_Reset_Set('B',4,1);
  Gpio_Pin_Reset_Set('C',14,0);
  GpioSet(data);
  Gpio_Pin_Reset_Set('B',3,1);
//  __nop();__nop();__nop();
	__nop();__nop();__nop();
  Gpio_Pin_Reset_Set('B',3,0);
//  __nop();__nop();__nop();
	__nop();__nop();__nop();
  
}

/****************************************************************************
* 名    称：SelectScreen
* 功    能：12864左右屏幕选择
* 入口参数：u8 Screen： 0左半屏幕 1右半屏幕
* 出口参数：无
* 说    明：
* 调用方法：无 asm("nop");
****************************************************************************/
static void SelectScreen(u8 Screen)
{
  switch(Screen)
  {  
    case 0: 
            Gpio_Pin_Reset_Set('A',7,1);
            __nop();__nop();__nop();
            Gpio_Pin_Reset_Set('B',11,0);
            __nop();__nop();__nop();
            break; 
    case 1:
            Gpio_Pin_Reset_Set('A',7,0);
            __nop();__nop();__nop();
            Gpio_Pin_Reset_Set('B',11,1);
            __nop();__nop();__nop();
            break;
		default:
            break;
  }
}

/****************************************************************************
* 名    称：SetOnOff
* 功    能：12864显示开关
* 入口参数：u8 onoff： 0关显示 1开显示
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void SetOnOff(u8 onoff)           
{
   onoff=0x3e|onoff; 
   WriteCmd(onoff);
}

/****************************************************************************
* 名    称：Setpage
* 功    能：12864页设置
* 入口参数：u8 page： 页数
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void Setpage(u8 page)        
{
  page=0xb8|page; 
  WriteCmd(page);
}

/****************************************************************************
* 名    称：Setx
* 功    能：12864起始行设置
* 入口参数：u8 x： 起始行数
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void Setx(u8 x)            
{
  x=0xc0|x; 
  WriteCmd(x); 
}

/****************************************************************************
* 名    称：Setx
* 功    能：12864起始行设置
* 入口参数：u8 x： 起始行数
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Sety(u8 y)        
{
  y=y&0x3f; 
  y= 0x40|y; 
  WriteCmd(y);
}

/****************************************************************************
* 名    称：ClearScreen
* 功    能：12864清屏
* 入口参数：u8 screen:清屏选择
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
static void ClearScreen(u8 screen)          
{         
  u8 i,j;
  SelectScreen(screen);
  for(i=0;i<8;i++)         //控制页数0-7，共8页
  {
    Setpage(i);
    Sety(0);
    for(j=0;j<64;j++)     //控制列数0-63，共64列
    {
      WriteData(0x00); //写点内容，列地址自动加1
    }
  }              
}

/****************************公有函数*********************************/

/****************************************************************************
* 名    称：ScreenConfiguration
* 功    能：屏显示初始化
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void ScreenConfiguration(void)
{
  //重开显示
  SelectScreen(0);
  SetOnOff(0);
  SetOnOff(1); 
  SelectScreen(1);
  SetOnOff(0);
  SetOnOff(1);
  
  SelectScreen(0);
  Setx(0);
  ClearScreen(0); 
  
  SelectScreen(1);
  Setx(0);
  ClearScreen(1);
}

/****************************************************************************
* 名    称：Display12x16
* 功    能：屏显示12X16的汉字
* 入口参数：u8 s,u8 page,u8 y,u8 number
*           选屏参数，pagr选页参数，column选列参数，number选第几汉字输出
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Display12x16(u8 s,u8 page,u8 y,u8* data)
{
  int i = 0;
  u8* pData = 0;
  u8 temp = 0;
  
  for( i = 0;i< GBLen_16X12;i++)
  {
    if( (data[0] == GBList_16X12[i*2]) && (data[1] == GBList_16X12[i*2 + 1]) )
    {
        break;
    }
  }
  
  if(i< GBLen_16X12)
  {
    pData = &GB_16X12[(i+1)*24];
  }
  else
  {
    pData = GB_16X12;
  }
          
  SelectScreen(s);
  y=y&0x3f;
  
  if(SetPoint == 1)
  {
    temp =  0x01;
  }
  else 
  {
    temp = 0;
  }
  Setpage(page);        
  Sety(y); 
  for(i=0;i<12;i++)  
  {
    WriteData(*(pData+i) | temp); 
  }

  if(SetPoint == 2)
  {
    temp = 0x80;
  }
  else
  {
    temp = 0;
  }
  Setpage(page+1);                 
  Sety(y);          
  for(i=0;i<12;i++)          
  {
    WriteData(*(pData+12+i) | temp);        
  }
} 


/****************************************************************************
* 名    称：DisplayColon
* 功    能：屏显示冒号
* 入口参数：u8 s,u8 page,u8 y,u8 number
*           选屏参数，pagr选页参数，column选列参数
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void DisplayColon(u8 s,u8 page,u8 y)
{
  u8 temp = 0;
  
  SelectScreen(s);
  y=y&0x3f;

  if(SetPoint == 1)
  {
    temp =  0x01;
  }
  else 
  {
    temp = 0;
  }
  Setpage(page);        
  Sety(y); 
  WriteData(0x60|temp); 
  
  if(SetPoint == 2)
  {
    temp = 0x80;
  }
  else
  {
    temp = 0;
  }
  Setpage(page + 1);        
  Sety(y); 
  WriteData(0x06|temp); 
}

/****************************************************************************
* 名    称：Display12x8Char
* 功    能：屏显示12X8的字符
* 入口参数：u8 s,u8 page,u8 y,u8 data
*           选屏参数，pagr选页参数，column选列参数，data 数据
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Display12x8Char(u8 s,u8 page,u8 y,u8 data)
{
  int i = 0;
  u8 j; 
  u8 temp = 0;
  
  if((data >= 0x20)&&(data <= 0x7e)) 
  { 
    j=data-0x20;    
  } 
  else
  {
    return;
  }
  
  SelectScreen(s);
  y=y&0x3f;

  if(SetPoint == 1)
  {
    temp =  0x01;
  }
  else 
  {
    temp = 0;
  }
  Setpage(page);        
  Sety(y); 
  for(i=0;i<8;i++)  
  {
    WriteData(ascii_table_8x16[j][i]|temp); 
  }

  if(SetPoint == 2)
  {
    temp = 0x80;
  }
  else
  {
    temp = 0;
  }
  Setpage(page+1);                 
  Sety(y);          
  for(i=0;i<8;i++)          
  {
    WriteData(ascii_table_8x16[j][i+8]|temp);        
  }
} 

/****************************************************************************
* 名    称：DisplayPoint
* 功    能：屏显示点
* 入口参数：u8 set： 1：最上边显示点 2：最下边显示点
* 出口参数：无 
* 说    明：
* 调用方法：无 
****************************************************************************/
void DisplayPoint(u8 set)
{
  SetPoint = set;
}

/****************************************************************************
* 名    称：DisplayLine
* 功    能：绘制直线
* 入口参数：u8 s,u8 page,u8 y,u8 len,u8 loc
*           选屏参数，pagr选页参数，column选列参数，长度，位置
* 出口参数：无 
* 说    明：
* 调用方法：无 
****************************************************************************/
void DisplayLine(u8 s,u8 page,u8 y,u8 len,u8 loc)
{
  u8 i = 0;
  u8 data = 0;
  SelectScreen(s);
  Setpage(page);
  Sety(y); 
  
  if(loc == 1)
  {
    data = 0x01;
  }
  else if(loc == 2)
  {
    data = 0x80;
  }
  for(i=0;i<len;i++)          
  {
    WriteData(data);        
  }
}

