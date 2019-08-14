#ifndef M_UPGRADE_H_
#define M_UPGRADE_H_
#include "mwifi.h"
#include "mupgrade.h"
#include "driver/uart.h"
void mupgrade_ota(char * data);
mdf_err_t set_ota_data(uint8_t * ota_data,size_t size);
mdf_err_t get_version(unsigned char * data);/*获取版本信息*/
// void mugrade_ota_init(void);
#endif