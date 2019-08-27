#ifndef SERIAL_H
#define SERIAL_H

#include "std_port_common.h"


typedef struct rtu_packet_t{
    uint8_t *raw_data;
    int len;
} rtu_packet_t;

int serial_init(uint8_t id);
rtu_packet_t *build_rtu(uint8_t *data, int len);
void free_rtu(rtu_packet_t *packet);
rtu_packet_t *wait_rtu(int time_ms);
rtu_packet_t *wait_self_rtu(int time_ms);
int send_rtu(rtu_packet_t *packet);

#endif