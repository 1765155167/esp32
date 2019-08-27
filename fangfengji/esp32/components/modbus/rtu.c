#include "rtu.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

void rtu_init()
{   
    serial_init(0x77);
    rtu_func_init();
}

int rtu_read_coin(uint8_t id, uint16_t address, int time_ms, uint8_t *data, uint8_t len)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(0);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->address = address;
    send->func_code = READ_COIN_FUNC;
    send->num = len;
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == READ_COIN_FUNC);
    for(int n = 0; n < len; n++)
    {
        int byte = n / 8;
        int bit = n % 8;
        if(recv->read[byte] & (0x01 << bit))
            data[n] = 1;
        else
            data[n] = 0;
    }
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("read coin success, id[%d] address[%d] len[%d]", id, address, len);
    else
        STD_LOGE("read coin fail, id[%d] address[%d] len[%d]", id, address, len);
    return res;
}

int rtu_write_coin(uint8_t id, uint16_t address, bool value, int time_ms)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(0);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->address = address;
    send->func_code = WRITE_COIN_FUNC;
    if(value)
        send->num = 0xFF00;
    else
        send->num = 0x0000;
    
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == WRITE_COIN_FUNC);
    STD_CHECK_GOTO_FAIL(recv->num == send->num);
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("write singal coin success, id[%d] address[%d]", id, address);
    else
        STD_LOGE("write singal coin fail, id[%d] address[%d]", id, address);
    return res;
}

int rtu_write_coins(uint8_t id, uint16_t address, int time_ms, uint8_t *data, int len)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send((len - 1) / 8 + 1);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->func_code = WRITE_COINS_FUNC;
    send->address = address;
    send->num = len;

    for(int n = 0; n < len; n++)
    {
        int byte = n / 8;
        int bit = n % 8;
        if(data[n])
            send->write[byte] = send->write[byte] | (0x01 << bit);
    }
    
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == WRITE_COINS_FUNC);
    STD_CHECK_GOTO_FAIL(recv->address == send->address);
    STD_CHECK_GOTO_FAIL(recv->num == send->num);
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("write multi coin success, id[%d] address[%d]", id, address);
    else
        STD_LOGE("write multi coin fail, id[%d] address[%d]", id, address);
    return res;
}

int rtu_read_bit(uint8_t id, uint16_t address, int time_ms, uint8_t *data, uint8_t len)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(0);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->address = address;
    send->func_code = READ_BIT_FUNC;
    send->num = len;
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == READ_BIT_FUNC);
    for(int n = 0; n < len; n++)
    {
        int byte = n / 8;
        int bit = n % 8;
        if(recv->read[byte] & (0x01 << bit))
            data[n] = 1;
        else
            data[n] = 0;
    }
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("read bit success, id[%d] address[%d] len[%d]", id, address, len);
    else
        STD_LOGE("read bit fail, id[%d] address[%d] len[%d]", id, address, len);
    return res;
}

int rtu_read_hold_reg(uint8_t id, uint16_t address, int time_ms, uint16_t *data, uint8_t len)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(0);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->address = address;
    send->func_code = READ_HOLD_REG_FUNC;
    send->num = len;
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == READ_HOLD_REG_FUNC);
    for(int n = 0; n < len; n++)
    {
        uint16_t *p = (uint16_t *)recv->read;
        data[n] = p[n];
    }
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("read hold reg success, id[%d] address[%d] len[%d]", id, address, len);
    else
        STD_LOGE("read hold reg fail, id[%d] address[%d] len[%d]", id, address, len);
    return res;
}

int rtu_read_input_reg(uint8_t id, uint16_t address, int time_ms, uint16_t *data, uint8_t len)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(0);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->address = address;
    send->func_code = READ_INPUT_REG_FUNC;
    send->num = len;
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == READ_INPUT_REG_FUNC);
    for(int n = 0; n < len; n++)
    {
        uint16_t *p = (uint16_t *)recv->read;
        data[n] = p[n];
    }
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("read input reg success, id[%d] address[%d] len[%d]", id, address, len);
    else
        STD_LOGE("read input reg fail, id[%d] address[%d] len[%d]", id, address, len);
    return res;
}

int rtu_write_reg(uint8_t id, uint16_t address, uint16_t value, int time_ms)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(0);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->address = address;
    send->func_code = WRITE_REG_FUNC;
    send->num = value;

    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == WRITE_REG_FUNC);
    STD_CHECK_GOTO_FAIL(recv->num == send->num);
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("write reg success, id[%d] address[%d]", id, address);
    else
        STD_LOGE("write reg fail, id[%d] address[%d]", id, address);
    return res;
}

int rtu_write_regs(uint8_t id, uint16_t address, int time_ms, uint16_t *data, int len)
{
    int res = -1;
    rtu_send_t *send = build_rtu_send(len * 2);
    rtu_recv_t *recv = NULL;
    send->id = id;
    send->func_code = WRITE_REGS_FUNC;
    send->address = address;
    send->num = len;

    for(int n = 0; n < len; n++)
    {
        uint16_t *p = (uint16_t *)send->write;
        flush_big_uint16((uint8_t *)&p[n], data[n]);
    }
    
    recv = send_func(send, time_ms);
    STD_CHECK_GOTO_FAIL(recv != NULL);
    STD_CHECK_GOTO_FAIL(recv->id == id);
    STD_CHECK_GOTO_FAIL(recv->func_code == WRITE_REGS_FUNC);
    STD_CHECK_GOTO_FAIL(recv->address == send->address);
    STD_CHECK_GOTO_FAIL(recv->num == send->num);
    res = 0;
FAIL:
    free_rtu_send(send);
    free_rtu_recv(recv);
    if(res == 0)
        STD_LOGD("write regs success, id[%d] address[%d]", id, address);
    else
        STD_LOGE("write reg fail, id[%d] address[%d]", id, address);
    return res;
}

#if PRODUCT_TEST

#endif