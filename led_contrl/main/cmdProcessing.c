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
	
	MDF_LOGI("start mupgrade_ota");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, EXIT, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	MDF_ERROR_GOTO(!json_id, EXIT, "ID is not found");

	json_cmd = cJSON_GetObjectItem(json_root,"Cmd");
	MDF_ERROR_GOTO(!json_cmd, EXIT, "Cmd is not found");

	if(strcmp(json_cmd->valuestring, "ota") == 0)
	{
		xTaskCreate(ota_task,"ota_task", 4*1024, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY + 1, NULL);
	}
		
EXIT:
	cJSON_Delete(json_root);
}