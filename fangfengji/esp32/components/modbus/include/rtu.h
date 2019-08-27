#ifndef RTU_H
#define RTU_H

#include "rtu_func.h"
#include "rtu_data.h"

void rtu_init();
int rtu_read_coin(uint8_t id, uint16_t address, int time_ms, uint8_t *data, uint8_t len);
int rtu_write_coin(uint8_t id, uint16_t address, bool value, int time_ms);
int rtu_write_coins(uint8_t id, uint16_t address, int time_ms, uint8_t *data, int len);
int rtu_read_bit(uint8_t id, uint16_t address, int time_ms, uint8_t *data, uint8_t len);
int rtu_read_hold_reg(uint8_t id, uint16_t address, int time_ms, uint16_t *data, uint8_t len);
int rtu_read_input_reg(uint8_t id, uint16_t address, int time_ms, uint16_t *data, uint8_t len);
int rtu_write_reg(uint8_t id, uint16_t address, uint16_t value, int time_ms);
int rtu_write_regs(uint8_t id, uint16_t address, int time_ms, uint16_t *data, int len);

#endif