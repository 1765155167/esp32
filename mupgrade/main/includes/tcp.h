#ifndef TCP_H_
#define TCP_H_

#include "mdf_common.h"
#include "mupgrade.h"
#include "mwifi.h"
#include "driver/uart.h"

mdf_err_t tcp_client_read(char *data, size_t * size);
mdf_err_t tcp_client_write(char *data);

#endif