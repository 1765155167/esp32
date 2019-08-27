#ifndef STD_TYPE_H
#define STD_TYPE_H

#include "std_common.h"

typedef void (*data_handle_cb_t)(uint8_t *, int);

uint16_t swap_byte_order_16bit(uint16_t value);
uint32_t swap_byte_order_32bit(uint32_t value);
uint64_t swap_byte_order_64bit(uint64_t value);

void flush_big_uint16(void *buf, uint16_t num);
void flush_big_uint32(void *buf, uint32_t num);
void flush_big_uint64(void *buf, uint64_t num);

void flush_little_uint16(void *buf, uint16_t num);
void flush_little_uint32(void *buf, uint32_t num);
void flush_little_uint64(void *buf, uint64_t num);

#endif
