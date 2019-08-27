#ifndef TEMP_TABLE_H
#define TEMP_TABLE_H

#include "mdf_common.h"
extern const float g_temp_table[72][3];

float convert2temp(float kom);
float convert2kom(float temp);

#endif