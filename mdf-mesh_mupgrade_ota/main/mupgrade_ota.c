#include "mupgrade_ota.h"
#include "mdf-mesh.h"
#include "check.h"

static const char *TAG = "mupgrade_ota";


mdf_err_t get_version()/*获取版本信息*/
{
	size_t size;
	char * data = (char *) MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	MDF_LOGI("start 准备ota升级......");
	sprintf((char *)data,"{\"Typ\":\"cut_ota\",\"Seq\":0}");
	
	size = strlen((char *)data);
	uart_encryption((uint8_t *)data,&size,DUPLEX_NO_ACK,STR);/*加密　crc检验位*/
	send_lock();
	uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)data, size);
	send_unlock();
	
	MDF_FREE(data);
	return MDF_OK;
}
