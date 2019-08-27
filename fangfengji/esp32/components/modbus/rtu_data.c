#include "rtu_data.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

rtu_send_t *build_rtu_send(uint8_t bytes)
{
    rtu_send_t *send = STD_CALLOC(1, sizeof(rtu_send_t));
    if(bytes > 0)
    {
        send->write = STD_CALLOC(bytes, 1);
        send->wbytes = bytes;
    }
    return send;
}

void free_rtu_send(rtu_send_t *send)
{
    if(send == NULL)
        return;

    if(send->write == NULL)
    {
        STD_FREE(send);
        return;
    }

    STD_FREE(send->write);
    STD_FREE(send);
}

rtu_recv_t *build_rtu_recv(uint8_t rbytes)
{
    rtu_recv_t *recv = STD_CALLOC(1, sizeof(rtu_recv_t));
    if(rbytes > 0)
    {
        recv->read = STD_MALLOC(rbytes);
        recv->rbytes = rbytes;
    }
    return recv;
}

void free_rtu_recv(rtu_recv_t *recv)
{
    if(recv == NULL)
        return;

    if(recv->read == NULL)
    {
        STD_FREE(recv);
        return;
    }

    STD_FREE(recv->read);
    STD_FREE(recv);
}


void print_rtu_recv(rtu_recv_t *recv)
{
    STD_LOGD("recv<< id[%d] func[%d] address[%x] num[%d] rbytes[%d]", recv->id, recv->func_code, recv->address, recv->num, recv->rbytes);
    if(recv->rbytes > 0)
        PRINT_HEX(recv->read, recv->rbytes);
}

void print_rtu_send(rtu_send_t *send)
{
    STD_LOGD("send>> id[%d] func[%d] address[%x] num[%d] rbytes[%d]", send->id, send->func_code, send->address, send->num, send->wbytes);
    if(send->wbytes > 0)
        PRINT_HEX(send->write, send->wbytes);  
}