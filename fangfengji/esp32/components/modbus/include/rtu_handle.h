#ifndef RTU_HANDLE_H
#define RTU_HANDLE_H

#include "std_port_common.h"
#include "serial.h"
#include "rtu_func.h"

typedef struct rtu_func_handle_t{
    uint8_t func_code;
    rtu_packet_t* (*encode)(rtu_send_t *send);
    rtu_recv_t* (*decode)(rtu_packet_t *packet);
}__attribute__((__packed__)) rtu_func_handle_t;

#define ADD_FUNC_HANDLE(__func_code, __encode, __decode) do{\
    static rtu_func_handle_t __handle = {\
        .func_code = __func_code,\
        .encode = __encode,\
        .decode = __decode,\
    };\
    rtu_add_func(&__handle);\
}while(0)

void rtu_add_func(rtu_func_handle_t *func_encode);
rtu_recv_t *send_func(rtu_send_t *send, int time_ms);

#endif