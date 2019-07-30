#include "led.h"
#include "key.h"
#include "mdf-mesh.h"
#include "driver/uart.h"
#include "moter_nvs.h"
static const char *TAG = "mesh-led";

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
	init_flag =pdTRUE;
	return MDF_OK;
}
/*
 *@按键控制
 **/
void key_led_press(int key)
{
	static int led = 0;
	switch (key)
	{
	case KEY1_SHORT_ONCE:
		moter_forward(led+1);
		break;
	case KEY2_SHORT_ONCE:
		moter_reverse(led+1);
		break;
	case KEY3_SHORT_ONCE:
		moter_stop(led+1);
		break;
	case KEY4_SHORT_ONCE:
		MDF_LOGI("切换操作风口");
		led = (led+1)%3;
		switch (led)
		{
		case 0:/*选择第一个放风机*/
			gpio_set_level(LED1_GPIO, 0);
			gpio_set_level(LED2_GPIO, 1);
			break;
		case 1:
			gpio_set_level(LED1_GPIO, 1);
			gpio_set_level(LED2_GPIO, 0);
			break;
		case 2:
			gpio_set_level(LED1_GPIO, 0);
			gpio_set_level(LED2_GPIO, 0);
			break;
		default:
			break;
		}
		break;
	case KEY4_LONG:
		MDF_LOGI("切换风口控制模式");
		moter_change_mode(led+1);
		break;
	default:
		break;
	}
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
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		cJSON_Delete(json_root);
		return;
	}

	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		cJSON_Delete(json_root);
		goto ret;
	}
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
	}else {
		MDF_LOGW("Unknown json cmd");
	}
ret:
	cJSON_Delete(json_root);
	MDF_FREE(json_info);
	return;
}
/*上传信息*/
mdf_err_t information_Upload(char * json_info)
{
	if(esp_mesh_is_root()) {
		size_t size = strlen(json_info);
		send_lock();
		uart_write_bytes(CONFIG_UART_PORT_NUM, json_info, size);
        uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
		send_unlock();
	}else {
		mesh_write(NULL,json_info);
	}
	return MDF_OK;
}
