#include "gpio_c.h"


/****************************************************************************
* ��    �ƣ�void Gpio_Pin_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
* ��    �ܣ�STM32F0ϵ�ж˿���λ����
* ��ڲ�����unsigned char GPIO_X:��Ҫ������GPIO��,unsigned char GPIO_PIN_X����Ҫ������GPIO��
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void Gpio_Pin_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
{	
  switch(GPIO_X)
  {
    case 'A':
                HAL_GPIO_WritePin(GPIOA,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;
    case 'B':
                HAL_GPIO_WritePin(GPIOB,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;
    case 'C':
                HAL_GPIO_WritePin(GPIOC,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;	
    case 'D':
                HAL_GPIO_WritePin(GPIOD,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;	
    case 'F':
                HAL_GPIO_WritePin(GPIOF,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;	
    default:   break;
  }	
}

/****************************************************************************
* ��    �ƣ�void Gpio_Pin_Reset(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
* ��    �ܣ�STM32F0ϵ�ж˿ڸ�λ����
* ��ڲ�����unsigned char GPIO_X:��Ҫ������GPIO��,unsigned char GPIO_PIN_X����Ҫ������GPIO��
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void Gpio_Pin_Reset(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
{	
  switch(GPIO_X)
  {
    case 'A':
                HAL_GPIO_WritePin(GPIOA,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;
    case 'B':
                HAL_GPIO_WritePin(GPIOB,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;
    case 'C':
                HAL_GPIO_WritePin(GPIOC,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;	
    case 'D':
                HAL_GPIO_WritePin(GPIOD,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;	
    case 'F':
                HAL_GPIO_WritePin(GPIOF,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;	
    default:   break;
  }	
}

/****************************************************************************
* ��    �ƣ�void Gpio_Pin_Reset_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X,unsigned char bit)
* ��    �ܣ�STM32F0ϵ�ж˿ڸ�λ,��λ����
* ��ڲ�����unsigned char GPIO_X:��Ҫ������GPIO��,unsigned char GPIO_PIN_X����Ҫ������GPIO�ڣ�unsigned char bit��1��λ��0��λ
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void Gpio_Pin_Reset_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X,unsigned char bit)
{	
  if(bit == 0)
  {
    Gpio_Pin_Reset(GPIO_X,GPIO_PIN_X);
  }
  else  
  {
    Gpio_Pin_Set(GPIO_X,GPIO_PIN_X);
  }
}


