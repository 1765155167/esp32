#include "PointSelect.h"
//#include "intrins.h"
/****************************˽�к궨��*******************************/

/****************************˽�к�������*****************************/
static void GpioSet(u8 data);//12864�������IO
static void WriteCmd(u8 cmd);//12864ָ��д��
static void WriteData(u8 data);//12864����д��
static void SelectScreen(u8 Screen);//12864������Ļѡ��
static void SetOnOff(u8 onoff);//12864��ʾ����  
static void Setpage(u8 page);//12864ҳ����   
static void Setx(u8 x);//12864��ʼ������ 



/****************************˽��ȫ�ֱ�������*************************/

const unsigned  char GB_12[] =        
{
    0x80,0x88,0x88,0x88,0xF8,0x88,0x88,0x88,
    0xF8,0x88,0x88,0x80,0x00,0x00,0x20,0x10,
    0x0F,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,//��0

    0x00,0x00,0xF8,0x28,0x48,0x88,0x48,0x28,
    0x08,0xF8,0x00,0x00,0x20,0x18,0x07,0x08,
    0x06,0x01,0x06,0x08,0x00,0x1F,0x20,0x38,//��1
};

static u8 SetPoint = 0;
/****************************����ȫ�ֱ�������*************************/

PointSelect_Parameter_Struct PointSelect_Parameter;//PointSelect���Ʋ����ṹ��

/****************************˽�к���*********************************/

/****************************************************************************
* ��    �ƣ�GpioSet
* ��    �ܣ�12864�������IO
* ��ڲ�����u8 data��IO�����������
* ���ڲ�������
* ˵    ����
* ���÷������� 
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
* ��    �ƣ�WriteCmd
* ��    �ܣ�12864ָ��д��
* ��ڲ�����u8 cmd��д���ָ��
* ���ڲ�������
* ˵    ����
* ���÷������� 
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
* ��    �ƣ�WriteData
* ��    �ܣ�12864����д��
* ��ڲ�����u8 data��д�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
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
* ��    �ƣ�SelectScreen
* ��    �ܣ�12864������Ļѡ��
* ��ڲ�����u8 Screen�� 0�����Ļ 1�Ұ���Ļ
* ���ڲ�������
* ˵    ����
* ���÷������� asm("nop");
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
* ��    �ƣ�SetOnOff
* ��    �ܣ�12864��ʾ����
* ��ڲ�����u8 onoff�� 0����ʾ 1����ʾ
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static void SetOnOff(u8 onoff)           
{
   onoff=0x3e|onoff; 
   WriteCmd(onoff);
}

/****************************************************************************
* ��    �ƣ�Setpage
* ��    �ܣ�12864ҳ����
* ��ڲ�����u8 page�� ҳ��
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static void Setpage(u8 page)        
{
  page=0xb8|page; 
  WriteCmd(page);
}

/****************************************************************************
* ��    �ƣ�Setx
* ��    �ܣ�12864��ʼ������
* ��ڲ�����u8 x�� ��ʼ����
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static void Setx(u8 x)            
{
  x=0xc0|x; 
  WriteCmd(x); 
}

/****************************************************************************
* ��    �ƣ�Setx
* ��    �ܣ�12864��ʼ������
* ��ڲ�����u8 x�� ��ʼ����
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void Sety(u8 y)        
{
  y=y&0x3f; 
  y= 0x40|y; 
  WriteCmd(y);
}

/****************************************************************************
* ��    �ƣ�ClearScreen
* ��    �ܣ�12864����
* ��ڲ�����u8 screen:����ѡ��
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static void ClearScreen(u8 screen)          
{         
  u8 i,j;
  SelectScreen(screen);
  for(i=0;i<8;i++)         //����ҳ��0-7����8ҳ
  {
    Setpage(i);
    Sety(0);
    for(j=0;j<64;j++)     //��������0-63����64��
    {
      WriteData(0x00); //д�����ݣ��е�ַ�Զ���1
    }
  }              
}

/****************************���к���*********************************/

/****************************************************************************
* ��    �ƣ�ScreenConfiguration
* ��    �ܣ�����ʾ��ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void ScreenConfiguration(void)
{
  //�ؿ���ʾ
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
* ��    �ƣ�Display12x16
* ��    �ܣ�����ʾ12X16�ĺ���
* ��ڲ�����u8 s,u8 page,u8 y,u8 number
*           ѡ��������pagrѡҳ������columnѡ�в�����numberѡ�ڼ��������
* ���ڲ�������
* ˵    ����
* ���÷������� 
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
* ��    �ƣ�DisplayColon
* ��    �ܣ�����ʾð��
* ��ڲ�����u8 s,u8 page,u8 y,u8 number
*           ѡ��������pagrѡҳ������columnѡ�в���
* ���ڲ�������
* ˵    ����
* ���÷������� 
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
* ��    �ƣ�Display12x8Char
* ��    �ܣ�����ʾ12X8���ַ�
* ��ڲ�����u8 s,u8 page,u8 y,u8 data
*           ѡ��������pagrѡҳ������columnѡ�в�����data ����
* ���ڲ�������
* ˵    ����
* ���÷������� 
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
* ��    �ƣ�DisplayPoint
* ��    �ܣ�����ʾ��
* ��ڲ�����u8 set�� 1�����ϱ���ʾ�� 2�����±���ʾ��
* ���ڲ������� 
* ˵    ����
* ���÷������� 
****************************************************************************/
void DisplayPoint(u8 set)
{
  SetPoint = set;
}

/****************************************************************************
* ��    �ƣ�DisplayLine
* ��    �ܣ�����ֱ��
* ��ڲ�����u8 s,u8 page,u8 y,u8 len,u8 loc
*           ѡ��������pagrѡҳ������columnѡ�в��������ȣ�λ��
* ���ڲ������� 
* ˵    ����
* ���÷������� 
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

