#include "moter_nvs.h"

static char * TAG = "moter_nvs";
extern moter_args moter_args1;/*参数信息*/
extern moter_args moter_args2;/*参数信息*/
extern moter_stu moter_flag1;/*实时信息*/
extern moter_stu moter_flag2;/*实时信息*/
extern int32_t TempCalOff[];/*温度校准偏移 真正偏移的10倍*/
extern uint8_t dest_addr[3][MWIFI_ADDR_LEN];

mdf_err_t nvs_init()
{
	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
	return err;
}

mdf_err_t nvs_load()
{
	mdf_err_t err;
	nvs_handle moter_handle;
    printf("\nOpening Non-Volatile moter1 && moter2 (NVS) handle...\n");
    /*load drives 1*/
	err = nvs_open("moter1", NVS_READWRITE, &moter_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
		err = nvs_get_i32(moter_handle, "AlarmTempMax", &moter_args1.AlarmTempMax);
		printf((err != MDF_OK) ? "AlarmTempMax1 Failed!\n" : "AlarmTempMax1 Done\n"); 

		err = nvs_get_i32(moter_handle, "AlarmTempMin", &moter_args1.AlarmTempMin);
		printf((err != MDF_OK) ? "AlarmTempMin1 Failed!\n" : "AlarmTempMin1 Done\n"); 

		err = nvs_get_i32(moter_handle, "SetTempMax", &moter_args1.SetTempMax);
		printf((err != MDF_OK) ? "SetTempMax1 Failed!\n" : "SetTempMax1 Done\n"); 

		err = nvs_get_i32(moter_handle, "SetTempMin", &moter_args1.SetTempMin);
		printf((err != MDF_OK) ? "SetTempMin1 Failed!\n" : "SetTempMin1 Done\n"); 

		err = nvs_get_u32(moter_handle, "TotalTime", &moter_args1.TotalTime);
		printf((err != MDF_OK) ? "TotalTime1 Failed!\n" : "TotalTime1 Done\n"); 

		err = nvs_get_i32(moter_handle, "OpenPer", &moter_flag1.OpenPer);
		printf((err != MDF_OK) ? "OpenPer1 Failed!\n" : "OpenPer1 Done\n"); 

		err = nvs_get_i32(moter_handle, "TempCalOff1", &TempCalOff[0]);
		printf((err != MDF_OK) ? "TempCalOff1 Failed!\n" : "TempCalOff1 Done\n"); 
	}
	nvs_close(moter_handle);// Close
	/*load drives 2*/
	err = nvs_open("moter2", NVS_READWRITE, &moter_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
		err = nvs_get_i32(moter_handle, "AlarmTempMax", &moter_args2.AlarmTempMax);
		printf((err != MDF_OK) ? "AlarmTempMax2 Failed!\n" : "AlarmTempMax2 Done\n"); 

		err = nvs_get_i32(moter_handle, "AlarmTempMin", &moter_args2.AlarmTempMin);
		printf((err != MDF_OK) ? "AlarmTempMin2 Failed!\n" : "AlarmTempMin2 Done\n"); 

		err = nvs_get_i32(moter_handle, "SetTempMax", &moter_args2.SetTempMax);
		printf((err != MDF_OK) ? "SetTempMax2 Failed!\n" : "SetTempMax2 Done\n"); 

		err = nvs_get_i32(moter_handle, "SetTempMin", &moter_args2.SetTempMin);
		printf((err != MDF_OK) ? "SetTempMin2 Failed!\n" : "SetTempMin2 Done\n"); 

		err = nvs_get_u32(moter_handle, "TotalTime", &moter_args2.TotalTime);
		printf((err != MDF_OK) ? "TotalTime2 Failed!\n" : "TotalTime2 Done\n"); 

		err = nvs_get_i32(moter_handle, "OpenPer", &moter_flag2.OpenPer);
		printf((err != MDF_OK) ? "OpenPer2 Failed!\n" : "OpenPer2 Done\n"); 

		err = nvs_get_i32(moter_handle, "TempCalOff2", &TempCalOff[1]);
		printf((err != MDF_OK) ? "TempCalOff2 Failed!\n" : "TempCalOff2 Done\n"); 
	}
	nvs_close(moter_handle);// Close
	return MDF_OK;
}

/**
 *@保存设备对应的参数信息
 **/
mdf_err_t nvs_save_arg(uint8_t drive)
{
	mdf_err_t err;
	nvs_handle moter_handle;
	if(drive == 1) {
		err = nvs_open("moter1", NVS_READWRITE, &moter_handle);
		if (err != MDF_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			err = nvs_set_i32(moter_handle, "AlarmTempMax", moter_args1.AlarmTempMax);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "AlarmTempMax1 Failed!\n" : "AlarmTempMax1 Done\n"); 
			
			err = nvs_set_i32(moter_handle, "AlarmTempMin", moter_args1.AlarmTempMin);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "AlarmTempMin1 Failed!\n" : "AlarmTempMin1 Done\n"); 

			err = nvs_set_i32(moter_handle, "SetTempMax", moter_args1.SetTempMax);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "SetTempMax1 Failed!\n" : "SetTempMax1 Done\n"); 

			err = nvs_set_i32(moter_handle, "SetTempMin", moter_args1.SetTempMin);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "SetTempMin1 Failed!\n" : "SetTempMin1 Done\n"); 

			err = nvs_set_u32(moter_handle, "TotalTime", moter_args1.TotalTime);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "TotalTime1 Failed!\n" : "TotalTime1 Done\n"); 
		}
	}else if(drive == 2) {
		err = nvs_open("moter2", NVS_READWRITE, &moter_handle);
		if (err != MDF_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			err = nvs_set_i32(moter_handle, "AlarmTempMax", moter_args2.AlarmTempMax);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "AlarmTempMax2 Failed!\n" : "AlarmTempMax2 Done\n"); 
			
			err = nvs_set_i32(moter_handle, "AlarmTempMin", moter_args2.AlarmTempMin);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "AlarmTempMin2 Failed!\n" : "AlarmTempMin2 Done\n"); 

			err = nvs_set_i32(moter_handle, "SetTempMax", moter_args2.SetTempMax);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "SetTempMax2 Failed!\n" : "SetTempMax2 Done\n"); 

			err = nvs_set_i32(moter_handle, "SetTempMin", moter_args2.SetTempMin);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "SetTempMin2 Failed!\n" : "SetTempMin2 Done\n"); 

			err = nvs_set_u32(moter_handle, "TotalTime", moter_args2.TotalTime);
			// err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "TotalTime2 Failed!\n" : "TotalTime2 Done\n"); 
		}
	}
	err = nvs_commit(moter_handle);
	printf((err != MDF_OK) ? "保存设备对应的参数信息 Failed!\n" : "保存设备对应的参数信息 OK\n"); 
	nvs_close(moter_handle);// Close
	return MDF_OK;
}
/**
 *@保存设备对应的风口开度
 **/
mdf_err_t nvs_save_OpenPer(uint8_t drive)
{
	mdf_err_t err;
	nvs_handle moter_handle;
	if(drive == 1) {
		err = nvs_open("moter1", NVS_READWRITE, &moter_handle);
		if (err != MDF_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			err = nvs_set_i32(moter_handle, "OpenPer", moter_flag1.OpenPer);
			err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "OpenPer1 Failed!\n" : "OpenPer1 Done\n"); 
		}
	}else if(drive == 2) {
		err = nvs_open("moter2", NVS_READWRITE, &moter_handle);
		if (err != MDF_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			err = nvs_set_i32(moter_handle, "OpenPer", moter_flag2.OpenPer);
			err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "OpenPer2 Failed!\n" : "OpenPer2 Done\n"); 
		}
	}
	nvs_close(moter_handle);// Close
	return MDF_OK;
}
/**
 *@保存温度校准值
 **/
mdf_err_t nvs_save_tempCal(uint8_t drive)
{
	mdf_err_t err;
	nvs_handle moter_handle;
	if(drive == 1) {
		err = nvs_open("moter1", NVS_READWRITE, &moter_handle);
		if (err != MDF_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			err = nvs_set_i32(moter_handle, "TempCalOff1", TempCalOff[0]);
			err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "TempCalOff1 Failed!\n" : "TempCalOff1 Done\n"); 
		}
	}else if(drive == 2) {
		err = nvs_open("moter2", NVS_READWRITE, &moter_handle);
		if (err != MDF_OK) {
			printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		} else {
			err = nvs_set_i32(moter_handle, "TempCalOff2", TempCalOff[1]);
			err = nvs_commit(moter_handle);
			printf((err != MDF_OK) ? "TempCalOff2 Failed!\n" : "TempCalOff2 Done\n"); 
		}
	}
	nvs_close(moter_handle);// Close
	return MDF_OK;	
}

mdf_err_t nvs_load_mac()
{
	mdf_err_t err;
	uint32_t addr[3][MWIFI_ADDR_LEN];
	size_t length = MWIFI_PAYLOAD_LEN;
	nvs_handle moter_handle;
	char * out_value = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	err = nvs_open("moter1", NVS_READWRITE, &moter_handle);
	if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
		// err = nvs_get_str(moter_handle, "mac table", &moter_args1.AlarmTempMax);
		err = nvs_get_str(moter_handle, "mac table", out_value, &length);
		if(err != MDF_OK) {
			MDF_LOGI("Load MAC err.");
		} else {
			MDF_LOGI("mac:%s,size:%d",out_value,length);
		}
		
		if (length == 54) {
			sscanf(out_value,"%x:%x:%x:%x:%x:%x,%x:%x:%x:%x:%x:%x,%x:%x:%x:%x:%x:%x",
					&addr[0][0],&addr[0][1],&addr[0][2],&addr[0][3],&addr[0][4],&addr[0][5],
					&addr[1][0],&addr[1][1],&addr[1][2],&addr[1][3],&addr[1][4],&addr[1][5],
					&addr[2][0],&addr[2][1],&addr[2][2],&addr[2][3],&addr[2][4],&addr[2][5]);
			for (size_t i = 0; i < 3; i++) {
				for (size_t j = 0; j < MWIFI_ADDR_LEN; j++) {
					dest_addr[i][j] = addr[i][j];
					printf("%02x:",dest_addr[i][j]);
				}
				printf("\n");	
			}
		} else if (length == 36) {
			sscanf(out_value,"%x:%x:%x:%x:%x:%x,%x:%x:%x:%x:%x:%x",
					&addr[0][0],&addr[0][1],&addr[0][2],&addr[0][3],&addr[0][4],&addr[0][5],
					&addr[1][0],&addr[1][1],&addr[1][2],&addr[1][3],&addr[1][4],&addr[1][5]);
			for (size_t i = 0; i < 2; i++)
			{
				for (size_t j = 0; j < MWIFI_ADDR_LEN; j++)
				{
					dest_addr[i][j] = addr[i][j];
					printf("%02x:",dest_addr[i][j]);
				}
				printf("\n");	
			}
		} else if (length == 18) {
			sscanf(out_value,"%x:%x:%x:%x:%x:%x",
					&addr[0][0],&addr[0][1],&addr[0][2],&addr[0][3],&addr[0][4],&addr[0][5]);
			for (size_t i = 0; i < 1; i++)
			{
				for (size_t j = 0; j < MWIFI_ADDR_LEN; j++)
				{
					dest_addr[i][j] = addr[i][j];
					printf("%02x:",dest_addr[i][j]);
				}
				printf("\n");	
			}
		} else {
			MDF_LOGW("MAC地址表格信息有误，请重新配置");
		}
	}
	MDF_FREE(out_value);
	return err;
}

mdf_err_t nvs_save_mac(char* value)
{
	mdf_err_t err;
	nvs_handle moter_handle;

	err = nvs_open("moter1", NVS_READWRITE, &moter_handle);
	if (err != MDF_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		err = nvs_set_str(moter_handle, "mac table", value);
		err = nvs_commit(moter_handle);
		printf((err != MDF_OK) ? "mac table Failed!\n" : "mac table Done\n"); 
	}
	nvs_close(moter_handle);// Close
	return MDF_OK;
}