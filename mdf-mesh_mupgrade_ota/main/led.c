#include "led.h"
#include "key.h"
#include "mdf-mesh.h"
#include "driver/uart.h"
#include "moter_nvs.h"
#include "check.h"
#include "mupgrade_ota.h"
#include "screen_info.h"
static const char *TAG = "mesh-led";
extern int CONFIG_DEVICE_NUM;/*设备号*/
extern int DEVICE_TYPE;/*设备类型*/
/*主设备等待从设备发送信息*/
static void test_root_once(void* arg);
//定义定时器句柄
esp_timer_handle_t test_root_handle = 0;
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_root_arg = {
	.callback = &test_root_once, //设置回调函数
	.arg = (void *)1,
	.name = "moter time 1" //定时器名字
};
static void test_root_once(void* arg)
{
	information_Upload(NULL);
}
mdf_err_t led_init(void)
{
	static bool init_flag = pdFALSE;
	if(init_flag) {
		return MDF_OK;
	}
	gpio_config_t io_config;
	io_config.pin_bit_mask = BIT64(LED1_GPIO);  /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_config.mode = GPIO_MODE_OUTPUT;         	/*!< GPIO mode: set input/output mode*/
    io_config.intr_type = GPIO_INTR_DISABLE;    /*!< GPIO interrupt type */    
	gpio_config(&io_config);
	MDF_ERROR_ASSERT( gpio_set_level(LED1_GPIO, 1) );

	io_config.pin_bit_mask = BIT64(LED2_GPIO);
	gpio_config(&io_config);
	MDF_ERROR_ASSERT( gpio_set_level(LED2_GPIO, 1) );

	io_config.pin_bit_mask = BIT64(LED3_GPIO);
	gpio_config(&io_config);
	MDF_ERROR_ASSERT( gpio_set_level(LED3_GPIO, 1) );

	MDF_ERROR_ASSERT( gpio_set_level(LED1_GPIO, 0) );
	ESP_ERROR_CHECK( esp_timer_create(&test_root_arg, &test_root_handle) );
	init_flag =pdTRUE;
	return MDF_OK;
}
/*设置等的状态*/
void led_status_set(int status)
{
    switch(status)
    {
        case MESH_CONNECTION_ERROR:
            gpio_set_level(LED1_GPIO, 1);
            break;

        case HIGH_TEMP:
            gpio_set_level(LED2_GPIO, 1);
            break;

        case LOW_TEMP:
            gpio_set_level(LED3_GPIO, 1);
            break;
    }
}

void led_status_unset(int status)
{
    switch(status)
    {
        case MESH_CONNECTION_ERROR:
            gpio_set_level(LED1_GPIO, 0);
            break;

        case HIGH_TEMP:
            gpio_set_level(LED2_GPIO, 0);
            break;

        case LOW_TEMP:
            gpio_set_level(LED3_GPIO, 0);
            break;
    }
}
/*
 *@按键控制
 **/
void key_led_press(int key)
{
	static int id = 0;
	if(id == 0) {
		id =  get_drive_id(); //获取id
	}
	switch (key)
	{
	case KEY1_SHORT_ONCE:
		moter_forward(id);
		break;
	case KEY2_SHORT_ONCE:
		moter_reverse(id);
		break;
	case KEY3_SHORT_ONCE:
		moter_stop(id);
		break;
	case KEY4_SHORT_ONCE:
		MDF_LOGI("切换操作风口");
		id =  change_screen_info();
		break;
	case KEY4_LONG:
		MDF_LOGI("切换风口控制模式");
		moter_change_mode(id);
		break;
	default:
		break;
	}
	trig_screen_info_refresh();//屏幕刷新
}
/*
 *@json 信息控制
 **/
void json_led_press(char * data)
{
	cJSON *json_root  = NULL;
    cJSON *json_id    = NULL;
	cJSON *json_cmd   = NULL;
	char *json_info   = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	MDF_LOGI("start json press");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	MDF_ERROR_GOTO(!json_id, ret, "json id 不存在 , data: %s", data);

	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	MDF_ERROR_GOTO(!json_cmd, ret, "json_cmd 不存在 , data: %s", data);

	if (strcmp(json_cmd->valuestring,"cfg") == 0) {
		MDF_LOGI("参数配置");
		if(json_id->valueint == -1) {
			set_args_info(data,CONFIG_DEVICE_NUM * 2 - 1);
			set_args_info(data,CONFIG_DEVICE_NUM * 2);
		}else {
			set_args_info(data,json_id->valueint);
		}
	}else if (strcmp(json_cmd->valuestring,"conMan") == 0) {
		MDF_LOGI("手动控制");
		if(json_id->valueint == -1) {
			manual_moter(data,CONFIG_DEVICE_NUM * 2 - 1);
			manual_moter(data,CONFIG_DEVICE_NUM * 2);
		}else {
			manual_moter(data,json_id->valueint);
		}
	}else if (strcmp(json_cmd->valuestring,"conMode") == 0) {
		MDF_LOGI("控制模式");
		if(json_id->valueint == -1) {
			moter_set_mode(data,CONFIG_DEVICE_NUM * 2 - 1);
			moter_set_mode(data,CONFIG_DEVICE_NUM * 2);
		}else {
			moter_set_mode(data,json_id->valueint);
		}
	}else if (strcmp(json_cmd->valuestring,"openAdjust") == 0) {
		MDF_LOGI("风口校准");
		if(json_id->valueint == -1) {
			moter_openAdjust(data,CONFIG_DEVICE_NUM * 2 - 1);
			moter_openAdjust(data,CONFIG_DEVICE_NUM * 2);
		}else {
			moter_openAdjust(data,json_id->valueint);
		}
	}else if (strcmp(json_cmd->valuestring,"tempAdjust") == 0) {
		MDF_LOGI("温度校准");
		if(json_id->valueint == -1) {
			moter_tempAdjust(data,CONFIG_DEVICE_NUM * 2 - 1);
			moter_tempAdjust(data,CONFIG_DEVICE_NUM * 2);
		}else {
			moter_tempAdjust(data,json_id->valueint);
		}
	}else if (strcmp(json_cmd->valuestring,"getInfo") == 0) {
		MDF_LOGI("触发实时数据上传");
		if (json_id->valueint == -1) {
			get_json_info(json_info, CONFIG_DEVICE_NUM * 2 - 1);
			information_Upload(json_info);
			get_json_info(json_info, CONFIG_DEVICE_NUM * 2);
			information_Upload(json_info);
		}else {
			get_json_info(json_info, json_id->valueint);
			information_Upload(json_info);
		}
	}else if (strcmp(json_cmd->valuestring,"ota") == 0) {
		MDF_LOGI("OTA升级");
		if(json_id->valueint == 1)
		{
			mupgrade_ota(data);//id=1
		}
	} else {
		MDF_LOGW("Unknown json cmd");
	}
	/*
	 *@每次收到一条指令就上传一次状态信息
	 * */

	MDF_LOGI("触发实时数据上传");
	if(esp_mesh_is_root()) {//主设备
		esp_timer_stop(test_root_handle);
		esp_timer_start_once(test_root_handle, 25 * 1000);
	}else{
		get_json_info(json_info, CONFIG_DEVICE_NUM * 2 - 1);
		information_Upload(json_info);
		get_json_info(json_info, CONFIG_DEVICE_NUM * 2);
		information_Upload(json_info);
	}
ret:
	cJSON_Delete(json_root);
	MDF_FREE(json_info);
	return;
}

/*上传温度报警信息*/
mdf_err_t up_alarm_temp_info(int id)
{
	char * json_info = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	if(esp_mesh_is_root()) {//主设备
		get_alarm_temp_info(json_info,id);
		add_dev_info(json_info);/*追加信息*/
		MDF_LOGI("up_alarm_temp_info:%s",json_info);
		size_t size = strlen(json_info);
		send_lock();
		uart_encryption((uint8_t *)json_info,&size,DUPLEX_NO_ACK,STR);/*加密　crc检验位*/
		uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)json_info, size);
        uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
		send_unlock();
	}else
	{
		get_alarm_temp_info(json_info,id);
		mesh_write(NULL,json_info);
		MDF_LOGI("up_alarm_temp_info:%s",json_info);
	}
	MDF_FREE(json_info);
	return MDF_OK;
}

/*上传信息*/
mdf_err_t information_Upload(char * json_info)
{
	if(esp_mesh_is_root()) {//主设备
		char * json_info_all = MDF_MALLOC(6 * MWIFI_PAYLOAD_LEN);
		get_json_info_all(json_info_all);
		MDF_LOGI("information_Upload:%s",json_info_all);
		size_t size = strlen(json_info_all);
		send_lock();
		uart_encryption((uint8_t *)json_info_all,&size,DUPLEX_NO_ACK,STR);/*加密　crc检验位*/
		uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)json_info_all, size);
        uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
		send_unlock();
		moter_3 = pdFALSE;
		moter_4 = pdFALSE;
		moter_5 = pdFALSE;
		moter_6 = pdFALSE;
		MDF_FREE(json_info_all);
	}else {//从设备通过mesh网络发送到主设备
		mesh_write(NULL,json_info);
		MDF_LOGI("information_Upload to ROOT ok data:%s",json_info);
	}
	return MDF_OK;
}
