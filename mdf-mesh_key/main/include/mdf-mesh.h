#ifndef MED_MESH_H_
#define MED_MESH_H_

#include "mdf_common.h"
#include "mwifi.h"

// #define MEMORY_DEBUG
#define BUF_SIZE 1024

mdf_err_t mdf_mesh_init();
mdf_err_t mesh_write(uint8_t src_addr[],char *data);
// #define MEMORY_DEBUG
void data_add_root(char * data);/*追加root标志*/
void data_add_mac(char * data,uint8_t src_addr[]);/*追加mac地址*/

#endif

