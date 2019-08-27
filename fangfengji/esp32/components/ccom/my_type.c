#include "my_type.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_DEBUG
#define LOCAL_LITTLE_ENDIAN 1

uint16_t swap_byte_order_16bit(uint16_t value) {
  uint16_t result = 0;
  result |= (value & 0x00FF) << 8;
  result |= (value & 0xFF00) >> 8;
  return result;
}

uint32_t swap_byte_order_32bit(uint32_t value) {
  uint32_t result = 0;
  result |= (value & 0x000000FF) << 24;
  result |= (value & 0x0000FF00) << 8;
  result |= (value & 0x00FF0000) >> 8;
  result |= (value & 0xFF000000) >> 24;
  return result;
}

uint64_t swap_byte_order_64bit(uint64_t value) {
  uint64_t result = 0;
  result |= (value & 0x00000000000000FF) << 56;
  result |= (value & 0x000000000000FF00) << 40;
  result |= (value & 0x0000000000FF0000) << 24;
  result |= (value & 0x00000000FF000000) << 8;
  result |= (value & 0x000000FF00000000) >> 8;
  result |= (value & 0x0000FF0000000000) >> 24;
  result |= (value & 0x00FF000000000000) >> 40;
  result |= (value & 0xFF00000000000000) >> 56;
  return result;
}

void flush_big_uint16(void *buf, uint16_t num)
{
#if LOCAL_LITTLE_ENDIAN
    num = swap_byte_order_16bit(num);
#endif
    memcpy(buf, &num, sizeof(uint16_t));
}

void flush_big_uint32(void *buf, uint32_t num)
{
#if LOCAL_LITTLE_ENDIAN
    num = swap_byte_order_32bit(num);
#endif
    memcpy(buf, &num, sizeof(uint32_t));
}

void flush_big_uint64(void *buf, uint64_t num)
{
#if LOCAL_LITTLE_ENDIAN
    num = swap_byte_order_64bit(num);
#endif
    memcpy(buf, &num, sizeof(uint64_t));
}

void flush_little_uint16(void *buf, uint16_t num)
{
#if LOCAL_LITTLE_ENDIAN
    
#endif
    memcpy(buf, &num, sizeof(uint16_t));
}

void flush_little_uint32(void *buf, uint32_t num)
{
#if LOCAL_LITTLE_ENDIAN
    
#endif
    memcpy(buf, &num, sizeof(uint32_t));
}

void flush_little_uint64(void *buf, uint64_t num)
{
#if LOCAL_LITTLE_ENDIAN
    
#endif
    memcpy(buf, &num, sizeof(uint64_t));
}

