#ifndef SMART_CONFIG_H_
#define SMART_CONFIG_H_

#include "esp_system.h"

esp_err_t get_wifi_info(mwifi_config_t * config);
esp_err_t erase_wifi_info(void);

#endif