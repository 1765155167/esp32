#include "rtu.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

#define MAX_FUNC 10

rtu_func_handle_t *func_list[MAX_FUNC] = {NULL};

void rtu_add_func(rtu_func_handle_t *func_encode)
{
    for(int n = 0; n < MAX_FUNC; n++)
    {
        if(func_list[n] == NULL)
        {   
            func_list[n] = func_encode;
            STD_LOGD("add rtu func[%d]", func_encode->func_code);
            return ;
        }
    }
    STD_ASSERT(-1);
}

static rtu_func_handle_t *get_func_handle(uint8_t func_code)
{
    for(int n = 0; n < MAX_FUNC; n++)
    {
        if(func_list[n]->func_code == func_code)
        {
            return func_list[n];
        }
    }
    STD_LOGE("undefined func[%d]", func_code);
    STD_ASSERT(-1);
    return NULL;
}

rtu_recv_t *send_func(rtu_send_t *send, int time_ms)
{
    print_rtu_send(send);
    rtu_func_handle_t *handle = get_func_handle(send->func_code);
    rtu_packet_t *packet;
    packet = handle->encode(send);
    if(send_rtu(packet) == -1)
    {
        STD_LOGE("send rtu fail");
        return NULL;
    }
    free_rtu(packet);
    packet = wait_rtu(time_ms);
    if(packet == NULL)
    {
        STD_LOGE("recv rtu time out");
        return NULL;
    }
    rtu_recv_t *recv = handle->decode(packet);
    free_rtu(packet);
    if(recv != NULL)
        print_rtu_recv(recv);
    return recv;
}
