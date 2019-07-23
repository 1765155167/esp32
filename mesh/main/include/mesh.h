#ifndef _MESH_H_
#define _MESH_H_

#include "esp_wifi.h"
/*******************************************************
 *                选择根节点
 *******************************************************/
#define MESH_SET_ROOT

#ifdef MESH_SET_ROOT
#define IPADDRESS "192.168.1.101"
#endif

#ifndef MESH_SET_ROOT
#define MESH_SET_NODE
#define IPADDRESS "192.168.1.102"
#endif

#define SEND_LEN 32/* 数据最大长度 */
static const uint8_t MESH_ID[6]   = {0x75, 0x77, 0x77, 0x77, 0x77, 0x77};
esp_err_t mymesh_init(void);/* mymesh　初始化函数 */
void mesh_read_err(esp_err_t err);
void mesh_write_err(esp_err_t err);
void router_err(esp_err_t err);

#endif