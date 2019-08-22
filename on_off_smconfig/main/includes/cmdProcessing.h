#ifndef CMDPROCESS_H_
#define CMDPROCESS_H_

#include "mwifi.h"
#include "mdf_common.h"

void cmdProcessing(char *data);
size_t get_flag_info(char * data, int count);
mdf_err_t uploadInformation(void);

#endif