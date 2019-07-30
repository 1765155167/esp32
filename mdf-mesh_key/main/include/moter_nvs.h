#ifndef _NVS_H_
#define _NVS_H_

#include "moter.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

mdf_err_t nvs_init(void);
/**
 *@保存设备对应的风口开度
 **/
mdf_err_t nvs_save_OpenPer(uint8_t drive);
/**
 *@保存设备对应的参数信息
 **/
mdf_err_t nvs_save_arg(uint8_t drive);
/**
 *@加载设备对应的参数信息、风口开度
 **/
mdf_err_t nvs_load(void);
#endif
