#ifndef M_UPGRADE_H_
#define M_UPGRADE_H_
#include "mwifi.h"
#include "mupgrade.h"
#include "driver/uart.h"
#include "serial.h"

typedef struct {/*OAT升级固件信息*/
    size_t size;     /**< Received size */
    uint8_t *data; /**< Received data */
} queue_data_t;

mdf_err_t get_version();/*获取版本信息*/
void esp_start_ota(char * data);
// void mugrade_ota_init(void);
#endif