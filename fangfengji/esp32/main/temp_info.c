#include "temp_info.h"
#include <stdlib.h>
#include <math.h>
static const char *TAG = "temp_info";

#define TEMP_TASK_SIZE 2048
#define TEMP_TASK_PRI 10

#define ADC_UNIT 1
#define DEFAULT_VREF 1100 
#define REF_MV 3300 //3300mv
#define INI_VAL 50 /*默认为25度*/
#define MAX_TEMP 2 /*ntc个数*/

int32_t TempCalOff[MAX_TEMP] = {0};/*温度校准偏移 真正偏移的10倍*/
static temp_info_t *g_info[MAX_TEMP] = {0};

static esp_adc_cal_characteristics_t *adc_chars;

/*电压值转成kom*/
static float mv2kom(uint16_t raw,int dev)
{ 
	float i, kom;
	if(dev == 2)  
	{
		i = (float)(REF_MV - raw) / RNF2;
	} else if(dev == 1)
	{
		i = (float)(REF_MV - raw) / RNF1;
	} else {
		i = 1;
		MDF_LOGI("dev err.........");
	}

    kom = raw / i;//raw 2 kom transfrom

    return kom;
}

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
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
            // MDF_LOGD("adc[%d] 电压:[%d]mV", g_info[n]->channel, val);
            float kom = mv2kom(val,n+1);
            // MDF_LOGD("adc[%d] kom[%f]",  g_info[n]->channel, kom);
            g_info[n]->kom = mdf_calcu_kalman(g_info[n]->kalman, kom);
            // MDF_LOGD("adc[%d] after kalman kom[%f]", g_info[n]->channel, g_info[n]->kom);
        }
		vTaskDelay(TP_MEASURE_PEROID_MS / portTICK_PERIOD_MS);
    }
}
/*温度校准*/
void tempCal(temp_info_t *info,float temp)
{
	info->TempCalOff = temp - convert2temp(info->kom + info->kom_offset);
	MDF_LOGI("TempCalOff:%f",info->TempCalOff);
	TempCalOff[0] = (int32_t)(g_info[0]->TempCalOff * 10);
	TempCalOff[1] = (int32_t)(g_info[1]->TempCalOff * 10);
}
/*读取温度*/
float get_temp(temp_info_t *info)
{
    return convert2temp(info->kom + info->kom_offset) + info->TempCalOff;
}

float calcu_kom_offset(temp_info_t *info, int true_temp)
{
    float kom = convert2kom(true_temp);
    // MDF_LOGD("temp info kom[%f]", info->kom);
    return kom - info->kom;
}

int _temp_init()
{
	esp_adc_cal_value_t val_type;
    // MDF_ASSERT(ADC_UNIT == 1);
	check_efuse();
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	val_type = esp_adc_cal_characterize(ADC_UNIT_1 , ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
	print_char_val_type(val_type);
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
			info->TempCalOff = TempCalOff[n] / 10.0;
            g_info[n] = info;
    		// MDF_LOGI("register temp info[%d],TempCalOff:%d", channel, TempCalOff[n]);
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