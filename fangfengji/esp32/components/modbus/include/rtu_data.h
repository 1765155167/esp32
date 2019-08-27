#ifndef RTU_DATA_H
#define RTU_DATA_H

#include "std_port_common.h"
#include "serial.h"

typedef struct rtu_send_t{
    uint8_t id;
    uint8_t func_code;
    uint16_t address;
    uint16_t num;
    uint8_t wbytes;
    uint8_t *write;
}__attribute__((__packed__)) rtu_send_t;

typedef struct rtu_recv_t{
    uint8_t id;
    uint8_t func_code;
    uint16_t address;
    uint16_t num;
    uint8_t rbytes;
    uint8_t *read;
}__attribute__((__packed__)) rtu_recv_t;

rtu_send_t *build_rtu_send(uint8_t bytes);
void free_rtu_send(rtu_send_t *send);

rtu_recv_t *build_rtu_recv(uint8_t rbytes);
void free_rtu_recv(rtu_recv_t *recv);

void print_rtu_recv(rtu_recv_t *recv);
void print_rtu_send(rtu_send_t *send);

#endif