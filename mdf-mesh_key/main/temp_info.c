#include "temp_info.h"
#include <stdlib.h>
#include <math.h>
static const char *TAG = "temp_info";

#define TEMP_TASK_SIZE 2048
#define TEMP_TASK_PRI 10

#define ADC_UNIT 1
#define DEFAULT_VREF 1100 
#define RNF 10
#define REF_MV 3300
#define INI_VAL 50/*默认为25度*/
#define MAX_TEMP 2
static int32_t TempCalOff = 0;/*温度校准偏移*/
static temp_info_t *g_info[MAX_TEMP] = {0};

static esp_adc_cal_characteristics_t *adc_chars;
/*电压值转成kom*/
static float mv2kom(uint16_t raw)
{   
    float i = (REF_MV - raw) / RNF;
    float kom = raw / i;//raw 2 kom transfrom
    return kom;
}

static void temp_task(void *argc)
{
	int val;
	MDF_LOGI("temp task is start");
   
    for(;;)
    {
        for(int n = 0; n < MAX_TEMP; n++)
        {
            if(g_info[n] == NULL)
                continue;
            adc1_config_channel_atten(g_info[n]->channel, ADC_ATTEN_DB_11);
            val = adc1_get_raw(g_info[n]->channel);
            val = esp_adc_cal_raw_to_voltage(val, adc_chars);/*将ADC读数转换为以mV为单位的电压*/
            MDF_LOGI("adc[%d] 电压:[%d]mV", g_info[n]->channel, val);
            float kom = mv2kom(val);
            MDF_LOGI("adc[%d] kom[%f]",  g_info[n]->channel, kom);
            g_info[n]->kom = mdf_calcu_kalman(g_info[n]->kalman, kom);
            MDF_LOGI("adc[%d] after kalman kom[%f]", g_info[n]->channel, g_info[n]->kom);
        }
		vTaskDelay(TP_MEASURE_PEROID_MS / portTICK_PERIOD_MS);
    }
}

float get_temp(temp_info_t *info)
{
    return convert2temp(info->kom + info->kom_offset) + TempCalOff;
}

float _get_temp(int moter)
{
	if(moter == 1) {
		return get_temp(g_info[0]);
	}else if(moter == 2) {
		return get_temp(g_info[1]);
	}
	MDF_LOGW("moter num error");
	return 0;
}

float calcu_kom_offset(temp_info_t *info, int true_temp)
{
    float kom = convert2kom(true_temp);
    MDF_LOGD("temp info kom[%f]", info->kom);
    return kom - info->kom;
}


int _temp_init()
{
	esp_adc_cal_value_t val_type;
    // MDF_ASSERT(ADC_UNIT == 1);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	val_type = esp_adc_cal_characterize(ADC_UNIT_1 , ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
	MDF_LOGD("esp_adc_cal_characterize :%d",val_type);
    xTaskCreate(temp_task, "temp task", TEMP_TASK_SIZE, NULL, TEMP_TASK_PRI, NULL);
    return 0;
}

temp_info_t *build_temp_info(int channel)
{
    static bool init = false;
    if(init == false)
    {
        _temp_init();
        init = true;
    }
    
    temp_info_t *info = MDF_CALLOC(1, sizeof(temp_info_t));
    info->kom_offset = 0;
    info->kalman = mdf_build_kalman(TP_Q, TP_R, INI_VAL);
    info->channel = channel;
    for(int n = 0; n < MAX_TEMP; n++)
    {
        if(g_info[n] == NULL)
        {
            g_info[n] = info;
            MDF_LOGD("register temp info[%d]", channel);
            return info;
        } 
    }
    MDF_LOGE("temp info overflow");
    // MDF_ASSERT(-1);
    return NULL;
}

void print_temp_info(temp_info_t *temp_info)
{
    printf("*************temp_info**************\r\n");
    printf("kom[%f] offset[%f]\r\n", temp_info->kom, temp_info->kom_offset);
    printf("temp[%f]\r\n", get_temp(temp_info));
    printf("-------------------------------------\r\n");
}