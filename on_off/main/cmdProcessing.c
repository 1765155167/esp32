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

	if(strcmp(json_cmd->valuestring, "ota") == 0)
	{
		if(esp_mesh_get_layer() == MESH_ROOT)
		{
			MDF_LOGI("START MUPGRADE TASK");
			xTaskCreate(ota_task,"ota_task", 4*1024, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY + 1, NULL);
		}else{
			MDF_LOGI("不是根节点，不能进行升级。请发给根节点");
		}
	}
		
EXIT:
	cJSON_Delete(json_root);
}

/*获取开关状态信息 json字符串*/
void get_flag_info(char * data)
{

}