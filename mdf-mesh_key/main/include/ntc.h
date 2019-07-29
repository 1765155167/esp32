#ifndef _NTC_H__
#define _NTC_H__

#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "driver/adc.h"

#define NTC1 32 //ADC1_CH4
#define NTC2 33 //ADC1_CH5
#define NTC1_CHANNEL ADC1_CHANNEL_4
#define NTC2_CHANNEL ADC1_CHANNEL_5
mdf_err_t ntc_init(void);
uint32_t get_temp(int ntc);

#endif
