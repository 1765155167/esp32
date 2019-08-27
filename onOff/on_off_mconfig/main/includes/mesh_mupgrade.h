#ifndef _MUPGRADE_H_
#define _MUPGRADE_H_

#include "config.h"
#include "key.h"
#include "led.h"
#include "mwifi.h"
#include "mdf_common.h"

mdf_err_t mupgrade_init(void);
mdf_err_t mesh_write(uint8_t src_addr[],char *data);
void ota_task(void * arg);/*OTA升级[任务]函数*/
#endif