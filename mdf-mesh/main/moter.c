#include "moter.h"

static const char *TAG = "mdf-moter";
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

void moter_ctrl(void *arg)
{
	for(;;)
	{
		if(strcmp(moter_flag1.MoSta,"forward")==0)
		{
			if(moter_flag1.OpenPer < 100)
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 0) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
			}else
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
				moter_flag1.MoSta = "stop";
				MDF_LOGI("风口1开度已达到100");
			}
		}else if(strcmp(moter_flag1.MoSta,"reverse")==0)
		{
			if(moter_flag1.OpenPer > 0)
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 0) );
			}else
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
				moter_flag1.MoSta = "stop";
				MDF_LOGI("风口1开度已达到0");
			}
		}else if(strcmp(moter_flag1.MoSta,"stop")==0)
		{
			MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
			MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
			
		}

		if(strcmp(moter_flag2.MoSta,"forward")==0)
		{
			if(moter_flag2.OpenPer < 100)
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 0) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
			}else
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
				moter_flag2.MoSta = "stop";
				MDF_LOGI("风口2开度已达到100");
			}
		}else if(strcmp(moter_flag2.MoSta,"reverse")==0)
		{
			if(moter_flag2.OpenPer > 0)
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 0) );
			}else
			{
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
				MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
				moter_flag2.MoSta = "stop";
				MDF_LOGI("风口2开度已达到0");
			}
		}else if(strcmp(moter_flag2.MoSta,"stop")==0)
		{
			MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
			MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}
/*计算风口开度*/
static void Calculation(void *timer)
{
	if(strcmp(moter_flag1.MoSta,"forward")==0)
	{
		moter_flag1.OpenPer += ONEMIN_OPENPER/10;
	}else if(strcmp(moter_flag1.MoSta,"reverse")==0)
	{
		moter_flag1.OpenPer -= ONEMIN_OPENPER/10;
	}

	if(strcmp(moter_flag2.MoSta,"forward")==0)
	{
		moter_flag2.OpenPer += ONEMIN_OPENPER/10;
	}else if(strcmp(moter_flag2.MoSta,"reverse")==0)
	{
		moter_flag2.OpenPer -= ONEMIN_OPENPER/10;
	}
}

mdf_err_t moter_init(void)
{
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

	xTaskCreate(moter_ctrl, "moter_ctrl", 2 * 1024,
                    NULL, 10, NULL);
	/* 定时计算风口开度 */
    TimerHandle_t timer = xTimerCreate("Calculation", 6000 / portTICK_RATE_MS,
                                       true, NULL, Calculation);
    xTimerStart(timer, 0);
	return MDF_OK;
}

mdf_err_t moter_forward(int io)
{
	switch (io)
	{
	case 1:/* 电机１正转 */
		if(moter_flag1.OpenPer < 100)
		{
			moter_flag1.MoSta = "forward";
		}else
		{
			MDF_LOGI("风口1开度已达到100");		}
		break;
	case 2:/* 电机２正转 */
		if(moter_flag2.OpenPer < 100)
		{
			moter_flag2.MoSta = "forward";
		}else
		{
			MDF_LOGI("风口2开度已达到100");
		}
		break;
	case 3:/* 电机１&2正转 */
		if(moter_flag1.OpenPer < 100)
		{
			moter_flag1.MoSta = "forward";
		}else
		{
			MDF_LOGI("风口1开度已达到100");
		}
		if(moter_flag2.OpenPer < 100)
		{
			moter_flag2.MoSta = "forward";
		}else
		{
			MDF_LOGI("风口2开度已达到100");
		}
		break;
	default:
		MDF_LOGI("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}

mdf_err_t moter_reverse(int io)
{
	switch (io)
	{
	case 1:/* 电机１反转 */
		if(moter_flag1.OpenPer > 0)
		{
			moter_flag1.MoSta = "reverse";
		}else
		{
			MDF_LOGI("风口1开度为0");
		}
		break;
	case 2:/* 电机２反转 */
		if(moter_flag2.OpenPer > 0)
		{
			moter_flag2.MoSta = "reverse";
		}else
		{
			MDF_LOGI("风口2开度为0");
		}
		break;
	case 3:/* 电机１&2反转 */
		if(moter_flag1.OpenPer > 0)
		{
			moter_flag1.MoSta = "reverse";
		}else
		{
			MDF_LOGI("风口1开度为0");
		}
		
		if(moter_flag2.OpenPer > 0)
		{
			moter_flag2.MoSta = "reverse";
		}else
		{
			MDF_LOGI("风口2开度为0");
		}
		break;
	default:
		MDF_LOGI("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}

mdf_err_t moter_stop(int io)
{
	switch (io)
	{
	case 1:/* 电机１*/
		moter_flag1.MoSta = "stop";
		break;
	case 2:/* 电机２*/
		moter_flag2.MoSta = "stop";
		break;
	case 3:/* 电机１&2*/
		moter_flag1.MoSta = "stop";
		moter_flag2.MoSta = "stop";
		break;
	default:
		MDF_LOGI("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}

mdf_err_t moter_set_mode(int io)
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
	default:
		MDF_LOGI("风口IO选择错误:%d",io);
		break;
	}
	return MDF_OK;
}

mdf_err_t get_json_info(char * json_info, int id)
{
	if(id%2+1 == 1)
	{
		sprintf(json_info,"{\"Typ\":\"%s\",\"ID\": %d,\"Cmd\": \"Info\",\"Params\": {\"NTemp\": %d,\"OpenPer\": %d,\"ConSta\": \"%s\",\"MoSta\": \"%s\"}}","fan",
					   id,moter_flag1.NTemp,moter_flag1.OpenPer,
					   moter_flag1.ConSta,
					   moter_flag1.MoSta);//
	}else if(id%2+1 == 2)
	{
		sprintf(json_info,"{\"Typ\":\"%s\",\"ID\": %d,\"Cmd\": \"Info\",\"Params\": {\"NTemp\": %d,\"OpenPer\": %d,\"ConSta\": \"%s\",\"MoSta\": \"%s\"}}","fan",
					   id,moter_flag2.NTemp,moter_flag2.OpenPer,
					   moter_flag2.ConSta,
					   moter_flag2.MoSta);//
	}
	return MDF_OK;
}