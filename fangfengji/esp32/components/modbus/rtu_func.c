#include "rtu_func.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO
#define CONST_LEN 8

static uint16_t crcbe_modbus(uint8_t *buf, int len)
{
    uint16_t big;
    flush_big_uint16((uint8_t *)&big,modbus_crc(buf, len)); 
    return big;
}

static rtu_packet_t *encode_send(rtu_send_t *send)
{
    rtu_packet_t *packet;
    if(send->wbytes > 0)
        packet = build_rtu(NULL, 8 + send->wbytes + 1);
    else
        packet = build_rtu(NULL, 8);

    rtu_send_t *temp = (rtu_send_t *)packet->raw_data;
    temp->id = send->id;
    temp->func_code = send->func_code;
    flush_big_uint16((uint8_t *)&temp->address, send->address);
    flush_big_uint16((uint8_t *)&temp->num, send->num);
    if(send->wbytes > 0)
    {
        temp->wbytes = send->wbytes;
        memcpy(&temp->wbytes + 1, send->write, send->wbytes);
    }

    uint16_t crc_big =  crcbe_modbus(packet->raw_data, packet->len - 2);
    memcpy(&packet->raw_data[packet->len - 2], &crc_big, 2);
    return packet;
}

bool check_crc(rtu_packet_t *packet)
{
    uint16_t crc_big = crcbe_modbus(packet->raw_data, packet->len - 2);

    if(memcmp(&crc_big, &packet->raw_data[packet->len - 2], 2) == 0)
        return true;
    else
    {
        STD_LOGE("crc check fail");
        return false;
    }
        
}

bool check_err(uint8_t *buf)
{
    uint8_t func_code = buf[0];
    if(func_code & 0x80)
    {
        switch(buf[1])
        {
            case 1:
                STD_LOGE("illegal func");
            break;

            case 2:
                STD_LOGE("illegal address");
            break;

            case 3:
                STD_LOGE("illegal data");
            break;        
        }
        return false;
    }
    return true;
}

rtu_packet_t *read_coin_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *read_coin_decoode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint8_t rbytes;
        uint8_t data;
    }__attribute__((__packed__)) specfic_recv_t;
    
    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;

    if(!check_err(&cspecfic_recv->func_code))
        return NULL;

    rtu_recv_t *recv = build_rtu_recv(cspecfic_recv->rbytes);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    memcpy(recv->read, &cspecfic_recv->data, cspecfic_recv->rbytes);
    return recv;
}

rtu_packet_t *write_coin_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *write_coin_decode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint16_t address;
        uint16_t value;
    }__attribute__((__packed__)) specfic_recv_t;

    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;
    if(!check_err(&cspecfic_recv->func_code))
        return NULL;
    
    rtu_recv_t *recv = build_rtu_recv(0);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    recv->address = swap_byte_order_16bit(cspecfic_recv->address);
    recv->num = swap_byte_order_16bit(cspecfic_recv->value);
    return recv;
}

rtu_packet_t *write_coins_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *write_coins_decode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint16_t address;
        uint16_t count;
    }__attribute__((__packed__)) specfic_recv_t;

    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;
    if(!check_err(&cspecfic_recv->func_code))
        return NULL;
    
    rtu_recv_t *recv = build_rtu_recv(0);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    recv->address = swap_byte_order_16bit(cspecfic_recv->address);
    recv->num = swap_byte_order_16bit(cspecfic_recv->count);
    return recv;
}

rtu_packet_t *read_bit_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *read_bit_decoode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint8_t rbytes;
        uint8_t data;
    }__attribute__((__packed__)) specfic_recv_t;
    
    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;

    if(!check_err(&cspecfic_recv->func_code))
        return NULL;

    rtu_recv_t *recv = build_rtu_recv(cspecfic_recv->rbytes);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    memcpy(recv->read, &cspecfic_recv->data, cspecfic_recv->rbytes);
    return recv;
}

rtu_packet_t *read_hold_reg_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *read_hold_reg_decoode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint8_t rbytes;
        uint8_t data;
    }__attribute__((__packed__)) specfic_recv_t;
    
    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;

    if(!check_err(&cspecfic_recv->func_code))
        return NULL;

    rtu_recv_t *recv = build_rtu_recv(cspecfic_recv->rbytes);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    memcpy(recv->read, &cspecfic_recv->data, cspecfic_recv->rbytes);
    return recv;
}

rtu_packet_t *read_input_reg_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *read_input_reg_decoode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint8_t rbytes;
        uint8_t data;
    }__attribute__((__packed__)) specfic_recv_t;
    
    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;

    if(!check_err(&cspecfic_recv->func_code))
        return NULL;

    rtu_recv_t *recv = build_rtu_recv(cspecfic_recv->rbytes);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    memcpy(recv->read, &cspecfic_recv->data, cspecfic_recv->rbytes);
    return recv;
}

rtu_packet_t *write_reg_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *write_reg_decode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint16_t address;
        uint16_t value;
    }__attribute__((__packed__)) specfic_recv_t;

    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;
    if(!check_err(&cspecfic_recv->func_code))
        return NULL;
    
    rtu_recv_t *recv = build_rtu_recv(0);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    recv->address = swap_byte_order_16bit(cspecfic_recv->address);
    recv->num = swap_byte_order_16bit(cspecfic_recv->value);
    return recv;
}

rtu_packet_t *write_regs_encode(rtu_send_t *send)
{
    return encode_send(send);
}

rtu_recv_t *write_regs_decode(rtu_packet_t *packet)
{
    if(!check_crc(packet))
        return NULL;

    typedef struct specfic_recv_t{
        uint8_t id;
        uint8_t func_code;
        uint16_t address;
        uint16_t count;
    }__attribute__((__packed__)) specfic_recv_t;

    specfic_recv_t *cspecfic_recv = (specfic_recv_t *)packet->raw_data;
    if(!check_err(&cspecfic_recv->func_code))
        return NULL;
    
    rtu_recv_t *recv = build_rtu_recv(0);
    recv->id = cspecfic_recv->id;
    recv->func_code = cspecfic_recv->func_code;
    recv->address = swap_byte_order_16bit(cspecfic_recv->address);
    recv->num = swap_byte_order_16bit(cspecfic_recv->count);
    return recv;
}

void rtu_func_init()
{
    ADD_FUNC_HANDLE(READ_COIN_FUNC, read_coin_encode, read_coin_decoode);
    ADD_FUNC_HANDLE(WRITE_COIN_FUNC, write_coin_encode, write_coin_decode);
    ADD_FUNC_HANDLE(WRITE_COINS_FUNC, write_coins_encode, write_coins_decode);
    ADD_FUNC_HANDLE(READ_BIT_FUNC, read_bit_encode, read_bit_decoode);
    ADD_FUNC_HANDLE(READ_HOLD_REG_FUNC, read_hold_reg_encode, read_hold_reg_decoode);
    ADD_FUNC_HANDLE(READ_INPUT_REG_FUNC, read_input_reg_encode, read_input_reg_decoode);
    ADD_FUNC_HANDLE(WRITE_REG_FUNC, write_reg_encode, write_reg_decode);
    ADD_FUNC_HANDLE(WRITE_REGS_FUNC, write_regs_encode, write_regs_decode);
}