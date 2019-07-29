#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "ntc.h"

static const char *TAG = "mesh-ntc";
//  NTC1 32 //ADC1_CH4
//  NTC2 33 //ADC1_CH5
static void ntc_task(void *arg)
{
	for(;;)
	{
		int val1 = adc1_get_raw(NTC1_CHANNEL);
		int val2 = adc1_get_raw(NTC2_CHANNEL);
		printf("val1 = %d\n", val1);
		printf("val2 = %d\n", val2);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
	vTaskDelete(NULL);
}
mdf_err_t ntc_init(void)
{
	adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(NTC1_CHANNEL,ADC_ATTEN_DB_0);
	adc1_config_channel_atten(NTC2_CHANNEL,ADC_ATTEN_DB_0);
	xTaskCreate(ntc_task, "ntc_task", 2 * 1024, NULL, 10, NULL);
	return MDF_OK;
}


uint32_t get_temp(int ntc)
{
	if(ntc == 1)
		return adc1_get_raw(NTC1_CHANNEL);
	if(ntc == 2)
		return adc1_get_raw(NTC1_CHANNEL);
	MDF_LOGI("ntc io error");
	return 0;
}