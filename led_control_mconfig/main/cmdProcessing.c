#include "cmdProcessing.h"
#include "mesh_mupgrade.h"
#include "config.h"
#include "cJSON.h"
#include "led.h"

static const char *TAG = "cmd";

/*处理MQTT下发的指令*/
void cmdProcessing(char * data)
{
	// mdf_err_t err = MDF_OK;
	cJSON *json_root   = NULL;
    cJSON *json_id     = NULL;
	cJSON *json_cmd    = NULL;
	
	MDF_LOGI("start cmdProcessing");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, EXIT, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	MDF_ERROR_GOTO(!json_id, EXIT, "ID is not found");

	json_cmd = cJSON_GetObjectItem(json_root,"Cmd");
	MDF_ERROR_GOTO(!json_cmd, EXIT, "Cmd is not found");

	if (strcmp(json_cmd->valuestring, "OTA") == 0) {
		if (esp_mesh_get_layer() == MESH_ROOT) {
			MDF_LOGI("START MUPGRADE TASK");
			xTaskCreate(ota_task,"ota_task", 4*1024, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY + 1, NULL);
		} else {
			MDF_LOGI("不是根节点，不能进行升级。请发给根节点");
		}
	} else if (strcmp(json_cmd->valuestring, "ONLED") == 0) {//开灯
		set_led_flag(json_id->valueint - 1,ON_LED);
	} else if (strcmp(json_cmd->valuestring, "OFFLED") == 0) {//关灯
		set_led_flag(json_id->valueint - 1,OFF_LED);
	} else if (strcmp(json_cmd->valuestring, "ONLEDT") == 0) {//定时开
		cJSON *json_time    = NULL;
		json_time = cJSON_GetObjectItem(json_root,"Time");
		MDF_ERROR_GOTO(!json_time,EXIT,"Time is not found");

		MDF_LOGI("%d s后开灯...", json_time->valueint);

	} else if (strcmp(json_cmd->valuestring, "OFFLEDT") == 0) {//定时关
		cJSON *json_time    = NULL;
		json_time = cJSON_GetObjectItem(json_root,"Time");
		MDF_ERROR_GOTO(!json_time,EXIT,"Time is not found");

		MDF_LOGI("%d s后关灯...", json_time->valueint);
	}
	uploadInformation();
EXIT:
	cJSON_Delete(json_root);
}

/*上传灯的状态信息 手动上传*/
mdf_err_t uploadInformation(void)
{
	mdf_err_t err;
	char * data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	get_flag_info(data,-1);
	err = mesh_write(NULL, data);
	MDF_ERROR_GOTO(err!=MDF_OK, EXIT, "uploadInformation : %s",data);

EXIT:
	MDF_FREE(data);
	return err;
}