#include "moter.h"

static const char *TAG = "mesh-moter";

moter_stu moter_flag = {/*实时信息*/
	.Typ = "fan",
	.NTemp = 15,
	.OpenPer = 10,
	.ConSta = "auto",
	.MoSta = "forward"
};

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
	return MDF_OK;
}

mdf_err_t moter_forward(int io)
{
	switch (io)
	{
	case 1:/* 电机１正转 */
		MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 0) );
		MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
		break;
	case 2:/* 电机２正转 */
		MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 0) );
		MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
		break;
	default:
		MDF_LOGI("风口IO选择错误");
		break;
	}
	
	return MDF_OK;
}

mdf_err_t moter_reverse(int io)
{
	switch (io)
	{
	case 1:/* 电机１反转 */
		MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
		MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 0) );
		break;
	case 2:/* 电机２反转 */
		MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
		MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 0) );
		break;
	default:
		MDF_LOGI("风口IO选择错误");
		break;
	}
	return MDF_OK;
}

mdf_err_t moter_stop(int io)
{
	switch (io)
	{
	case 1:/* 电机１*/
		MDF_ERROR_ASSERT( gpio_set_level(MOTER1_FORWARD_IO, 1) );
		MDF_ERROR_ASSERT( gpio_set_level(MOTER1_REVERSE_IO, 1) );
		break;
	case 2:/* 电机２*/
		MDF_ERROR_ASSERT( gpio_set_level(MOTER2_FORWARD_IO, 1) );
		MDF_ERROR_ASSERT( gpio_set_level(MOTER2_REVERSE_IO, 1) );
		break;
	default:
		MDF_LOGI("风口IO选择错误");
		break;
	}
	return MDF_OK;
}

mdf_err_t get_json_info(char * json_info,int id)
{
	sprintf(json_info,"{\"Typ\":\"%s\",\"ID\": %d,\"Cmd\": \"Info\",\"Params\": {\"NTemp\": %d,\"OpenPer\": %d,\"ConSta\": \"%s\",\"MoSta\": \"%s\"}}","fan",
					   id,moter_flag.NTemp,moter_flag.OpenPer,
					   moter_flag.ConSta,
					   moter_flag.MoSta);//
	return MDF_OK;
}