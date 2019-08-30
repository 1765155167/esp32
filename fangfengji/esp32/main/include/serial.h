#ifndef SERIAL_H_
#define SERIAL_H_

#include "moter.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "check.h"

#define BUF_SIZE MWIFI_PAYLOAD_LEN
void send_unlock(void);
void send_lock(void);
bool get_air202();
mdf_err_t uart_initialize(void);
#endif