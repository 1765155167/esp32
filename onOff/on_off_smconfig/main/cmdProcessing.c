#include "cmdProcessing.h"
#include "mesh_mupgrade.h"
#include "config.h"
#include "cJSON.h"
#include "led.h"

/*引用led.c变量*/
extern struct ledflag ledflag[KEY_MAX];

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
size_t get_flag_info(char * data, int count)
{
	size_t size = 0;
	size = sprintf(data,"%d:{\"Devs\":[{\"ID\":1,\"Flag\": \"%s\"},{\"ID\":2,\"Flag\": \"%s\"},{\"ID\":3,\"Flag\": \"%s\"},{\"ID\":4,\"Flag\": \"%s\"}]}",
						count,ledflag[0].led_flage ? "On":"Off", 
						ledflag[1].led_flage ? "On":"Off", 
						ledflag[2].led_flage ? "On":"Off", 
						ledflag[3].led_flage ? "On":"Off");
	return size;
}

/*上传灯的状态信息*/
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