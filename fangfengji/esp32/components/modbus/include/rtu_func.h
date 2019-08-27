#ifndef RTU_FUNC_H
#define RTU_FUNC_H

#include "rtu_data.h"
#include "rtu_handle.h"
#include "rom/crc.h"
#include "my_type.h"
#include "modbus_crc.h"

void rtu_func_init();

typedef enum rut_func_code_t {
    READ_COIN_FUNC = 0x01,
    READ_REG_FUNC = 0x03,

    WRITE_COIN_FUNC = 0x05,
    WRITE_COINS_FUNC = 0x0F,

    READ_BIT_FUNC = 0x02,

    READ_INPUT_REG_FUNC = 0x04,
    READ_HOLD_REG_FUNC = 0x03,

    WRITE_REG_FUNC = 0x06,
    WRITE_REGS_FUNC = 0x10,
}rut_func_code_t;


#endif