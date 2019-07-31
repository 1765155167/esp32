#include "moter.h"

static const char *TAG = "mdf-moter";

moter_args moter_args1 = {
	.AlarmTempMax = 30,	//报警高温
	.AlarmTempMin = 10,	//报警低温
	.SetTempMax = 20,	//设定控制温度上限
	.SetTempMin = 15,	//设定控制温度下限
	.TotalTime = 600,   //风口完整开启或关闭一次所需时间，单位s
};/*参数信息*/

moter_args moter_args2= {
	.AlarmTempMax = 30,	//报警高温
	.AlarmTempMin = 10,	//报警低温
	.SetTempMax = 20,	//设定控制温度上限
	.SetTempMin = 15,	//设定控制温度下限
	.TotalTime = 600,   //风口完整开启或关闭一次所需时间，单位s
};/*参数信息*/

moter_stu moter_flag1 = {/*实时信息*/
	.Typ = "fan",
	.NTemp = 15,
	.OpenPer = 0,
	.ConSta = "manual",
	.MoSta = "stop"
};

moter_stu moter_flag2 = {/*实时信息*/
	.Typ = "fan",
	.NTemp = 15,
	.OpenPer = 0,
	.ConSta = "manual",
	.MoSta = "stop"
};
/*手动*/
static void test_timer_once(void* arg);
//定义定时器句柄
esp_timer_handle_t test_once1_handle = 0;
esp_timer_handle_t test_once2_handle = 0;
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_once1_arg = {
	.callback = &test_timer_once, //设置回调函数
	.arg = (void *)1,
	.name = "moter time 1" //定时器名字
};
esp_timer_create_args_t test_once2_arg = {
	.callback = &test_timer_once, //设置回调函数
	.arg = (void *)2,
	.name = "moter time 2" //定时器名字
};
/*自动*/
static void test_timer_once_auto(void* arg);
//定义定时器句柄
esp_timer_handle_t test_once1_auto_handle = 0;
esp_timer_handle_t test_once2_auto_handle = 0;
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_once1_auto_arg = {
	.callback = &test_timer_once_auto, //设置回调函数
	.arg = (void *)1,
	.name = "moter time 1" //定时器名字
};
esp_timer_create_args_t test_once2_auto_arg = {
	.callback = &test_timer_once_auto, //设置回调函数
	.arg = (void *)2,
	.name = "moter time 2" //定时器名字
};

//定时器回调函数
static void test_timer_once(void* arg)
{
	uint32_t io = (uint32_t)arg;
	if(io == 1) {
		moter_stop(io);
	}else if(io == 2) {
		moter_stop(io);
	}
}

//定时器回调函数
static void test_timer_once_auto(void* arg)
{
	uint32_t io = (uint32_t)arg;
	if(io == 1) {
		moter_flag1.MoSta = "stop";
		nvs_save_OpenPer(1);
	}else if(io == 2) {
		moter_flag2.MoSta = "stop";
		nvs_save_OpenPer(2);
	}
}
/*自动控制*/
static void moter_auto_ctrl(void *arg)
{
	for(;;)
	{
		if(strcmp(moter_flag1.ConSta,"auto") == 0) {
			if(moter_flag1.NTemp >= moter_args1.SetTempMin && moter_flag1.NTemp <= moter_args1.SetTempMax)
			{
				moter_flag1.MoSta = "stop";
			}else if(moter_flag1.NTemp < moter_args1.SetTempMin)
			{
				moter_flag1.MoSta = "forward";
				esp_timer_stop(test_once1_auto_handle);
				esp_timer_start_once(test_once1_auto_handle, 
					moter_args1.SetTempMin - moter_flag1.NTemp * 100 * moter_args1.TotalTime);
			}else {
				moter_flag1.MoSta = "reverse";
				esp_timer_stop(test_once1_auto_handle);
				esp_timer_start_once(test_once1_auto_handle, 
					moter_flag1.NTemp - moter_args1.SetTempMax * 100 * moter_args1.TotalTime);
			}
		}
		if(strcmp(moter_flag2.ConSta,"auto") == 0) {
			if(moter_flag2.NTemp >= moter_args2.SetTempMin && moter_flag2.NTemp <= moter_args2.SetTempMax)
			{
				moter_flag2.MoSta = "stop";
			}else if(moter_flag2.NTemp < moter_args2.SetTempMin)
			{
				moter_flag2.MoSta = "forward";
				esp_timer_stop(test_once2_handle);
				esp_timer_start_once(test_once2_handle, 
					moter_args2.SetTempMin - moter_flag2.NTemp * 100 * moter_args2.TotalTime);
			}else {
				moter_flag2.MoSta = "reverse";
				esp_timer_stop(test_once2_handle);
				esp_timer_start_once(test_once2_handle,
					moter_flag2.NTemp - moter_args2.SetTempMax * 100 * moter_args2.TotalTime);
			}
		}
		vTaskDelay(60 * 1000 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

static void moter_ctrl(void *arg)
{
	for(;;)
	{
		if(strcmp(moter_flag1.MoSta, "forward") == 0) {
			if(moter_flag1.OpenPer < 1000) {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 0) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
			}else {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
				moter_flag1.MoSta = "stop";
				MDF_LOGW("风口1开度已达到100");
			}
		}else if (strcmp(moter_flag1.MoSta, "reverse") == 0) {
			if(moter_flag1.OpenPer > 0) {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 0) );
			}else {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
				moter_flag1.MoSta = "stop";
				MDF_LOGW("风口1开度已达到0");
			}
		}else if (strcmp(moter_flag1.MoSta,"stop") == 0) {
			MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
			MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
		}

		if(strcmp (moter_flag2.MoSta, "forward") == 0) {
			if(moter_flag2.OpenPer < 1000) {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 0) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
			}else {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
				moter_flag2.MoSta = "stop";
				MDF_LOGW("风口2开度已达到100");
			}
		}else if (strcmp(moter_flag2.MoSta, "reverse")==0) {
			if(moter_flag2.OpenPer > 0) {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 0) );
			}else {
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
				moter_flag2.MoSta = "stop";
				MDF_LOGW("风口2开度已达到0");
			}
		}else if (strcmp (moter_flag2.MoSta, "stop")==0) {
			MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
			MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

/* 定时上传信息 UP_INFO_TIMER s*/
static void uploadinfor(void *timer)
{
	char *json_info   = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	get_json_info(json_info, CONFIG_DEVICE_NUM * 2 - 1);
	information_Upload(json_info);
	get_json_info(json_info, CONFIG_DEVICE_NUM * 2);
	information_Upload(json_info);
	MDF_FREE(json_info);
}

/*计算风口开度并保存 0.6s 计算一次 */
static void Calculation(void *timer)
{
	static uint8_t caltimers1 = 0;
	static uint8_t caltimers2 = 0;
	if(strcmp(moter_flag1.MoSta,"forward")==0) {
		moter_flag1.OpenPer += 600 / moter_args1.TotalTime;
		caltimers1 ++;
		if(moter_flag1.OpenPer >= 1000) {
			moter_flag1.OpenPer = 1000;
		}
	}else if(strcmp(moter_flag1.MoSta,"reverse")==0) {
		moter_flag1.OpenPer -= 600 / moter_args1.TotalTime;
		caltimers1 ++;
		if(moter_flag1.OpenPer <= 0){
			moter_flag1.OpenPer = 0;
		}
	}
	if(caltimers1 > 20) {
		nvs_save_OpenPer(1);
		caltimers1  = 0;
	}
	if(strcmp(moter_flag2.MoSta,"forward")==0) {
		moter_flag2.OpenPer += 600 / moter_args2.TotalTime;
		caltimers2 ++;
		if(moter_flag2.OpenPer >= 1000) {
			moter_flag2.OpenPer = 1000;
		}
	}else if(strcmp(moter_flag2.MoSta,"reverse")==0) {
		moter_flag2.OpenPer -= 600 / moter_args2.TotalTime;
		caltimers2 ++;
		if(moter_flag2.OpenPer <= 0){
			moter_flag2.OpenPer = 0;
		}
	}
	if(caltimers2 > 20) {
		nvs_save_OpenPer(2);
		caltimers2  = 0;
	}
}
/*放风机IO初始化*/
mdf_err_t moter_init(void)
{
	static bool init_flag = pdFALSE;
	if(init_flag) {
		return MDF_OK;
	}
	gpio_config_t io_config;
	io_config.pin_bit_mask = BIT64(MOTER1_FORWARD_IO);  /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_config.mode = GPIO_MODE_OUTPUT;         	/*!< GPIO mode: set input/output mode*/
    io_config.intr_type = GPIO_INTR_DISABLE;    /*!< GPIO interrupt type */    
	gpio_config(&io_config);

	io_config.pin_bit_mask = BIT64(MOTER1_REVERSE_IO);
	gpio_config(&io_config);

	io_config.pin_bit_mask = BIT64(MOTER2_FORWARD_IO);
	gpio_config(&io_config);

	io_config.pin_bit_mask = BIT64(MOTER2_REVERSE_IO);
	gpio_config(&io_config);
	
	MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
	MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
	MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
	MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );

	ESP_ERROR_CHECK( esp_timer_create(&test_once1_arg, &test_once1_handle) );
	ESP_ERROR_CHECK( esp_timer_create(&test_once2_arg, &test_once2_handle) );

	ESP_ERROR_CHECK( esp_timer_create(&test_once1_auto_arg, &test_once1_auto_handle) );
	ESP_ERROR_CHECK( esp_timer_create(&test_once2_auto_arg, &test_once2_auto_handle) );

	xTaskCreate(moter_ctrl, "moter_ctrl", 2 * 1024, NULL, 10, NULL);
	xTaskCreate(moter_auto_ctrl, "moter_auto_ctrl", 2 * 1024, NULL, 10, NULL);
	/* 定时计算风口开度 */
    TimerHandle_t timer1 = xTimerCreate("Calculation", 600 / portTICK_RATE_MS,
                                       true, NULL, Calculation);
    xTimerStart(timer1, 0);

	/* 定时上传信息 */
    TimerHandle_t timer2 = xTimerCreate("Upload information", UP_INFO_TIMER*1000 / portTICK_RATE_MS,
                                       true, NULL, uploadinfor);
    xTimerStart(timer2, 0);

	init_flag = pdTRUE;
	return MDF_OK;
}
/*手动模式下放风机正转*/
mdf_err_t moter_forward(int io)
{
	switch (io)
	{
	case 1:/* 电机１正转 */
		if(moter_flag1.OpenPer < 1000 && strcmp(moter_flag1.ConSta,"manual") == 0) {
			moter_flag1.MoSta = "forward";
		}else {
			MDF_LOGW("风口1开度已达到100 or 自动模式");		
		}
		break;
	case 2:/* 电机２正转 */
		if(moter_flag2.OpenPer < 1000 && strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag2.MoSta = "forward";
		}else {
			MDF_LOGW("风口2开度已达到100 or 自动模式");
		}
		break;
	case 3:/* 电机１&2正转 */
		if(moter_flag1.OpenPer < 1000 && strcmp(moter_flag1.ConSta,"manual") == 0) {
			moter_flag1.MoSta = "forward";
		}else {
			MDF_LOGW("风口1开度已达到100 or 自动模式");		
		}
		if(moter_flag2.OpenPer < 1000 && strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag2.MoSta = "forward";
		}else {
			MDF_LOGW("风口2开度已达到100 or 自动模式");
		}
		break;
	default:
		MDF_LOGW("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}
/*手动模式下放风机反转*/
mdf_err_t moter_reverse(int io)
{
	switch (io)
	{
	case 1:/* 电机１反转 */
		if(moter_flag1.OpenPer > 0 && strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag1.MoSta = "reverse";
		}else {
			MDF_LOGW("风口1开度为0 or 自动模式");
		}
		break;
	case 2:/* 电机２反转 */
		if(moter_flag2.OpenPer > 0 && strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag2.MoSta = "reverse";
		}else {
			MDF_LOGW("风口2开度为0 or 自动模式");
		}
		break;
	case 3:/* 电机１&2反转 */
		if(moter_flag1.OpenPer > 0 && strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag1.MoSta = "reverse";
		}else {
			MDF_LOGW("风口1开度为0 or 自动模式");
		}
		if(moter_flag2.OpenPer > 0 && strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag2.MoSta = "reverse";
		}else {
			MDF_LOGW("风口2开度为0 or 自动模式");
		}
		break;
	default:
		MDF_LOGW("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}
/*手动模式下放风机停止*/
mdf_err_t moter_stop(int io)
{
	switch (io)
	{
	
	case 1:/* 电机１*/
		if(strcmp(moter_flag1.ConSta,"manual") == 0) {
			moter_flag1.MoSta = "stop";
			nvs_save_OpenPer(1);
		}
		else
			MDF_LOGW("电机1自动模式");
		break;
	case 2:/* 电机２*/
		if(strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag2.MoSta = "stop";
			nvs_save_OpenPer(2);
		}
		else
			MDF_LOGW("电机2自动模式");
		break;
	case 3:/* 电机１&2*/
		if(strcmp(moter_flag1.ConSta,"manual") == 0) {
			moter_flag1.MoSta = "stop";
			nvs_save_OpenPer(1);
		}
		else
			MDF_LOGW("电机1自动模式");
		if(strcmp(moter_flag2.ConSta,"manual") == 0) {
			moter_flag2.MoSta = "stop";
			nvs_save_OpenPer(2);
		}
		else
			MDF_LOGW("电机2自动模式");
		break;
	default:
		MDF_LOGI("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}
/*改变放风机模式*/
mdf_err_t moter_change_mode(int io)
{
	switch (io)
	{
	case 1:/* 电机１*/
		if(strcmp(moter_flag1.ConSta,"auto") == 0) {
			moter_flag1.ConSta = "manual";
		}else {
			moter_flag1.ConSta = "auto";
		}
		break;
	case 2:/* 电机２*/
		if(strcmp(moter_flag2.ConSta,"auto") == 0) {
			moter_flag2.ConSta = "manual";
		}else {
			moter_flag2.ConSta = "auto";
		}
		break;
	case 3:/* 电机１&2*/
		if(strcmp(moter_flag1.ConSta,"auto") == 0) {
			moter_flag1.ConSta = "manual";
		}else {
			moter_flag1.ConSta = "auto";
		}
		if(strcmp(moter_flag2.ConSta,"auto") == 0) {
			moter_flag2.ConSta = "manual";
		}else {
			moter_flag2.ConSta = "auto";
		}
		break;
	default:
		MDF_LOGW("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}

/*获取放风机信息*/
mdf_err_t get_json_info(char * json_info, int id)
{
	if(id%2+1 == 1)
	{
		sprintf(json_info,"{\"Typ\":\"%s\",\"ID\": %d,\"Cmd\": \"Info\",\"Params\": {\"NTemp\": %d,\"OpenPer\": %d,\"ConSta\": \"%s\",\"MoSta\": \"%s\"}}","fan",
					   id,moter_flag1.NTemp,moter_flag1.OpenPer/10,
					   moter_flag1.ConSta,
					   moter_flag1.MoSta);//
	}else if(id%2+1 == 2)
	{
		sprintf(json_info,"{\"Typ\":\"%s\",\"ID\": %d,\"Cmd\": \"Info\",\"Params\": {\"NTemp\": %d,\"OpenPer\": %d,\"ConSta\": \"%s\",\"MoSta\": \"%s\"}}","fan",
					   id,moter_flag2.NTemp,moter_flag2.OpenPer/10,
					   moter_flag2.ConSta,
					   moter_flag2.MoSta);//
	}
	return MDF_OK;
}

/*设置参数信息 参数配置*/
mdf_err_t set_args_info(char * data, uint8_t id)
{
	cJSON *json_root  = NULL;
    cJSON *json_id    = NULL;
	cJSON *json_cmd   = NULL;
	cJSON *json_params= NULL;
	cJSON *json_atmax = NULL;//报警高温
	cJSON *json_atmin = NULL;//报警低温
	cJSON *json_tmax  = NULL;//设定控制温度上限
	cJSON *json_tmin  = NULL;//设定控制温度下限
	cJSON *json_ttime = NULL;//风口完整开启或关闭一次所需时间，单位s

	MDF_LOGI("start set_args_info");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		goto ret;
	}
	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		goto ret;
	}
	if(strcmp(json_cmd->valuestring,"cfg") != 0) {
		MDF_LOGW("Cmd not is cfg");
		goto ret;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	if (!json_params) {/* json cmd 不存在 */
		MDF_LOGW("Params not found");
		goto ret;
	}
	json_atmax = cJSON_GetObjectItem(json_params, "AlarmTempMax");
	if (!json_atmax) {
		MDF_LOGW("AlarmTempMax not found");
		goto ret;
	}
	json_atmin = cJSON_GetObjectItem(json_params, "AlarmTempMin");
	if (!json_atmin) {
		MDF_LOGW("AlarmTempMin not found");
		goto ret;
	}
	json_tmax = cJSON_GetObjectItem(json_params, "SetTempMax");
	if (!json_tmax) {
		MDF_LOGW("SetTempMax not found");
		goto ret;
	}
	json_tmin = cJSON_GetObjectItem(json_params, "SetTempMin");
	if (!json_tmin) {
		MDF_LOGW("SetTempMin not found");
		goto ret;
	}
	json_ttime = cJSON_GetObjectItem(json_params, "TotalTime");
	if (!json_ttime) {
		MDF_LOGW("TotalTime not found");
		goto ret;
	}

	if(id%2+1 == 1)
	{
		moter_args1.AlarmTempMax = json_atmax->valueint;
		moter_args1.AlarmTempMin = json_atmin->valueint;
		moter_args1.SetTempMax = json_tmax->valueint;
		moter_args1.SetTempMin = json_tmin->valueint;
		moter_args1.TotalTime = json_ttime->valueint;
		MDF_LOGI("1:AlarmTempMax:%d\r\nAlarmTempMin:%d\r\nSetTempMax:%d\r\nSetTempMin:%d\r\nTotalTime:%d\r\n"
				,moter_args1.AlarmTempMax
				,moter_args1.AlarmTempMin
				,moter_args1.SetTempMax
				,moter_args1.SetTempMin
				,moter_args1.TotalTime);
		nvs_save_arg(1);//保存设置的信息
	}else if(id%2+1 == 2)
	{
		moter_args2.AlarmTempMax = json_atmax->valueint;
		moter_args2.AlarmTempMin = json_atmin->valueint;
		moter_args2.SetTempMax = json_tmax->valueint;
		moter_args2.SetTempMin = json_tmin->valueint;
		moter_args2.TotalTime = json_ttime->valueint;
		MDF_LOGI("2:AlarmTempMax:%d\r\nAlarmTempMin:%d\r\nSetTempMax:%d\r\nSetTempMin:%d\r\nTotalTime:%d\r\n"
				,moter_args2.AlarmTempMax
				,moter_args2.AlarmTempMin
				,moter_args2.SetTempMax
				,moter_args2.SetTempMin
				,moter_args2.TotalTime);
		nvs_save_arg(2);//保存设置的信息
	}

ret:	
	cJSON_Delete(json_root);	
	return MDF_OK;
}

/*手动控制*/
mdf_err_t manual_moter(char * data, uint8_t id)
{
	cJSON *json_root  = NULL;
    cJSON *json_id    = NULL;
	cJSON *json_cmd   = NULL;
	cJSON *json_params= NULL;
	cJSON *json_sta   = NULL;
	cJSON *json_times = NULL;
	MDF_LOGI("start manual_moter");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		goto ret;
	}
	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		goto ret;
	}
	if(strcmp(json_cmd->valuestring,"conMan") != 0) {
		MDF_LOGW("Cmd not is conMan");
		goto ret;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	if (!json_params) {
		MDF_LOGW("Params not found");
		goto ret;
	}
	json_sta = cJSON_GetObjectItem(json_params, "Sta");
	if (!json_sta) {
		MDF_LOGW("Sta not found");
		goto ret;
	}
	json_times = cJSON_GetObjectItem(json_params, "TimeS");
	if (!json_times) {
		MDF_LOGW("TimeS not found");
		goto ret;
	}
	if(id%2+1 == 1) {
		if(strcmp(json_sta->valuestring,"forward") == 0) {
			moter_forward(1);
			esp_timer_stop(test_once1_handle);
			esp_timer_start_once(test_once1_handle,  json_times->valueint  * 1000 * 1000);
		}else if(strcmp(json_sta->valuestring,"reverse") == 0) {
			moter_reverse(1);
			esp_timer_stop(test_once1_handle);
			esp_timer_start_once(test_once1_handle,  json_times->valueint  * 1000 * 1000);
		}else if(strcmp(json_sta->valuestring,"stop") == 0) {
			moter_stop(1);
		}else
		{
			MDF_LOGW("Sta error");
		}
	}else if(id%2+1 == 2) {
		if(strcmp(json_sta->valuestring,"forward") == 0) {
			moter_forward(2);
			esp_timer_stop(test_once2_handle);
			esp_timer_start_once(test_once2_handle,  json_times->valueint  * 1000 * 1000);
		}else if(strcmp(json_sta->valuestring,"reverse") == 0) {
			moter_reverse(2);
			esp_timer_stop(test_once2_handle);
			esp_timer_start_once(test_once2_handle,  json_times->valueint  * 1000 * 1000);
		}else if(strcmp(json_sta->valuestring,"stop") == 0) {
			moter_stop(2);
		}else
		{
			MDF_LOGW("Sta error");
		}
	}
	
ret:	
	cJSON_Delete(json_root);	
	return MDF_OK;
}
/*设置放风机模式*/
//{"Devs":[{"ID":1,"Cmd":"conMode","Params":{"Sta":"auto"}}]}
mdf_err_t moter_set_mode(char * data, uint8_t id)
{
	cJSON *json_root  = NULL;
    cJSON *json_id    = NULL;
	cJSON *json_cmd   = NULL;
	cJSON *json_params= NULL;
	cJSON *json_sta   = NULL;
	MDF_LOGI("start moter_set_mode");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		goto ret;
	}
	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		goto ret;
	}
	if(strcmp(json_cmd->valuestring,"conMode") != 0) {
		MDF_LOGW("Cmd not is conMode");
		goto ret;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	if (!json_params) {
		MDF_LOGW("Params not found");
		goto ret;
	}
	json_sta = cJSON_GetObjectItem(json_params, "Sta");
	if (!json_sta) {
		MDF_LOGW("Sta not found");
		goto ret;
	}
	if (id%2+1 == 1) {
		if(strcmp(json_sta->valuestring,"manual") == 0) {
			moter_flag1.ConSta = "manual";
		}else if (strcmp(json_sta->valuestring,"auto") == 0) {
			moter_flag1.ConSta = "auto";
		} else {
			MDF_LOGW("set conMode error");
		}
	} else if(id%2+1 == 2) {
		if(strcmp(json_sta->valuestring,"manual") == 0) {
			moter_flag2.ConSta = "manual";
		}else if (strcmp(json_sta->valuestring,"auto") == 0) {
			moter_flag2.ConSta = "auto";
		} else {
			MDF_LOGW("set conMode error");
		}
	}
ret:	
	cJSON_Delete(json_root);	
	return MDF_OK;
}
/**
 * 风口校准
 **/
mdf_err_t moter_openAdjust(char * data, uint8_t id)
{
	cJSON *json_root  = NULL;
    cJSON *json_id    = NULL;
	cJSON *json_cmd   = NULL;
	cJSON *json_params= NULL;
	cJSON *json_openper   = NULL;
	MDF_LOGI("start moter_openAdjust");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		goto ret;
	}
	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		goto ret;
	}
	if(strcmp(json_cmd->valuestring,"openAdjust") != 0) {
		MDF_LOGW("Cmd not is openAdjust");
		goto ret;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	if (!json_params) {
		MDF_LOGW("Params not found");
		goto ret;
	}
	json_openper = cJSON_GetObjectItem(json_params, "Openper");
	if (!json_openper) {
		MDF_LOGW("Openper not found");
		goto ret;
	}
	if(id%2+1 == 1) {
		if(json_openper->valueint>=0&&json_openper->valueint<=100) {
			moter_flag1.OpenPer = json_openper->valueint * 10;
		} else {
			MDF_LOGW("Openper value >100 || < 0");	
		} 
	} else if(id%2+1 == 2) {
		if(json_openper->valueint>=0&&json_openper->valueint<=100) {
			moter_flag2.OpenPer = json_openper->valueint * 10;
		} else {
			MDF_LOGW("Openper value >100 || < 0");	
		} 
	}
ret:
	cJSON_Delete(json_root);	
	return MDF_OK;
}
/*温度校准*/
mdf_err_t moter_tempAdjust(char * data, uint8_t id)
{
	cJSON *json_root  = NULL;
    cJSON *json_id    = NULL;
	cJSON *json_cmd   = NULL;
	cJSON *json_params= NULL;
	cJSON *json_temp   = NULL;
	MDF_LOGI("start moter_tempAdjust");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		goto ret;
	}
	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		goto ret;
	}
	if(strcmp(json_cmd->valuestring,"tempAdjust") != 0) {
		MDF_LOGW("Cmd not is tempAdjust");
		goto ret;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	if (!json_params) {
		MDF_LOGW("Params not found");
		goto ret;
	}
	json_temp = cJSON_GetObjectItem(json_params, "Temp");
	if (!json_temp) {
		MDF_LOGW("Temp not found");
		goto ret;
	}
	if(id%2+1 == 1) {
		// if(json_temp->valueint<=moter_args1.AlarmTempMax&&json_temp->valueint>=moter_args1.AlarmTempMin) {
		// 	moter_flag1.NTemp = json_temp->valueint;
		// } else {
		// 	MDF_LOGW("１Temp value 超出温度上下限");	
		// } 
		if(json_temp->valueint<=100&&json_temp->valueint>=-100) {
			moter_flag1.NTemp = json_temp->valueint;
		} else {
			MDF_LOGW("１Temp value 超出温度上下限");	
		} 
	} else if(id%2+1 == 2) {
		// if(json_temp->valueint<=moter_args2.AlarmTempMax&&json_temp->valueint>=moter_args2.AlarmTempMin) {
		// 	moter_flag2.NTemp = json_temp->valueint;
		// } else {
		// 	MDF_LOGW("２Temp value 超出温度上下限");
		// } 
		if(json_temp->valueint<=100&&json_temp->valueint>=-100) {
			moter_flag2.NTemp = json_temp->valueint;
		} else {
			MDF_LOGW("２Temp value 超出温度上下限");
		} 
	}
ret:
	cJSON_Delete(json_root);	
	return MDF_OK;	
}