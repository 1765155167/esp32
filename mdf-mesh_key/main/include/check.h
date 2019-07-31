#ifndef _CHECK_H_
#define _CHECK_H_

#include "mdf-mesh.h"
#include "driver/uart.h"
#include "moter_nvs.h"

unsigned char crc8_com(unsigned char *p, int counter);
mdf_err_t uart_decrypt(unsigned char * data, size_t *len);//解密
void uart_encryption(unsigned char * data, size_t *len);//加密

#endif