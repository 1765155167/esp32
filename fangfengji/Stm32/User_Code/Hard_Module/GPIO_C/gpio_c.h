#ifndef _GPIO_C_H_
#define _GPIO_C_H_

#include "global.h"


void Gpio_Pin_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X);//STM32F0ϵ�ж˿���λ����
void Gpio_Pin_Reset(unsigned char GPIO_X,unsigned char GPIO_PIN_X);//STM32F0ϵ�ж˿ڸ�λ����
void Gpio_Pin_Reset_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X,unsigned char bit);//STM32F0ϵ�ж˿ڸ�λ,��λ����

#endif

