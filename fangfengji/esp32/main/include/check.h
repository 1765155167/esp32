#ifndef _CHECK_H_
#define _CHECK_H_

#include "mdf-mesh.h"
#include "driver/uart.h"
#include "moter_nvs.h"

//ack_typ
#define DUPLEX_NEED_ACK 2
#define DUPLEX_IS_ACK 3
#define DUPLEX_NO_ACK 1
//typ
#define STR 1
#define BIN 2

unsigned char crc8_com(unsigned char *p, int counter);
mdf_err_t uart_decrypt( unsigned char * data,
						size_t * size,
						uint8_t * ack_typ,
						uint8_t * typ);//解密
void uart_encryption(	unsigned char * data,
						size_t * size,
						uint8_t ack_typ, 
						uint8_t typ);//加密
// mdf_err_t uart_decrypt(unsigned char * data, size_t *len);//解密
// void uart_encryption(unsigned char * data, size_t *len);//加密1
void send_ack(void);/*发送应答信号*/
#endif