#include "key.h"
#include "led.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"

static const char *TAG = "mesh-key";
void key_process(void *arg);/* 按键扫描任务函数 */
void key_scan(void *arg);
mykey_t key[4];/* 按键标志结构体 */
static void test_timer_ej_cb(void* arg);
static void test_timer_el_cb(void* arg);
//定义一个消息队列
static xQueueHandle key_evt_queue = NULL;

//定义2个定时器句柄
esp_timer_handle_t test_ej_handle = 0;
esp_timer_handle_t test_el_handle = 0;
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_ej_arg = {
	.callback = &test_timer_ej_cb, //设置回调函数
	.name = "KeyEliminateJitterTimer1" //定时器名字
};
esp_timer_create_args_t test_el_arg = {
	.callback = &test_timer_el_cb, //设置回调函数
	.name = "KeyEliminateJitterTimer2" //定时器名字
};
//中断回调函数
void IRAM_ATTR key_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(key_evt_queue, &gpio_num, NULL);
}

//定时器回调函数(按键按下消抖)
static void test_timer_ej_cb(void* arg)
{
	esp_timer_delete(test_ej_handle);
	uint32_t num = (uint32_t)arg;
	if(gpio_get_level(key[num].io_num) == 0)
	{
		key[num].tick = esp_timer_get_time();
		// MDF_LOGI("press %d:tick = %lld", num, key[num].tick);
		key[num].press_key = pdTRUE;
	}
	key[num].time_start = pdFALSE;
}
//定时器回调函数(按键松开消抖)
static void test_timer_el_cb(void* arg)
{
	esp_timer_delete(test_el_handle);
	uint32_t num = (uint32_t)arg;
	if(gpio_get_level(key[num].io_num) == 1)
	{
		key[num].lift_key = pdTRUE;
		key[num].tick = esp_timer_get_time() - key[num].tick;
		MDF_LOGI("lift %d:tick = %lld", num, key[num].tick);

		key[num].press_key = pdFALSE;
		key[num].lift_key = pdFALSE;
		if(key[num].tick > longTime * 1000)/* longTime ms */
		{
			ESP_LOGI(TAG,"按键%d长按",num+1);/* 按键长按 */
			if(num == 3)
				key_led_press(KEY4_LONG);
		}else
		{
			ESP_LOGI(TAG, "按键%d单击",num+1);
			switch (num)
			{
			case 0:
				key_led_press(KEY1_SHORT_ONCE);
				break;
			case 1:
				key_led_press(KEY2_SHORT_ONCE);
				break;
			case 2:
				key_led_press(KEY3_SHORT_ONCE);
				break;
			case 3:
				key_led_press(KEY4_SHORT_ONCE);
				break;
			default:
				break;
			}
		}

	}
	key[num].time_start = pdFALSE;
}
mdf_err_t key_init(void)
{
	gpio_config_t io_conf;
	memset(key,0,4*sizeof(mykey_t));
	key[0].io_num = KEY1_GPIO;
	key[1].io_num = KEY2_GPIO;
	key[2].io_num = KEY3_GPIO;
	key[3].io_num = KEY4_GPIO;
	for(int i = 0; i < 4; i++)
	{
		key[i].press_key  = pdFALSE;        /* 按键状态 按下*/
		key[i].lift_key   = pdFALSE;        /* 按键状态 抬起*/
		key[i].time_start = pdFALSE;        /* 消抖动定时器开始标志 */
		key[i].keyclick   = 1;              /* 按键单击次数 */
	}
	io_conf.pin_bit_mask = BIT64(KEY1_GPIO);    /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_conf.mode = GPIO_MODE_INPUT;           /*!< GPIO mode: set input/output mode                     */
    io_conf.intr_type = GPIO_INTR_ANYEDGE;    /*!< GPIO interrupt type   								*/
	gpio_config(&io_conf);

	io_conf.pin_bit_mask = BIT64(KEY2_GPIO);
	gpio_config(&io_conf);

	io_conf.pin_bit_mask = BIT64(KEY3_GPIO);
	gpio_config(&io_conf);

	io_conf.pin_bit_mask = BIT64(KEY4_GPIO);
	gpio_config(&io_conf);

	//注册中断服务
	gpio_install_isr_service(0);
	//注册中断回调函数
	gpio_isr_handler_add(KEY1_GPIO, key_handler, (void *) KEY1_GPIO);
	gpio_isr_handler_add(KEY2_GPIO, key_handler, (void *) KEY2_GPIO);
	gpio_isr_handler_add(KEY3_GPIO, key_handler, (void *) KEY3_GPIO);
	gpio_isr_handler_add(KEY4_GPIO, key_handler, (void *) KEY4_GPIO);
	//创建一个消息队列，
	key_evt_queue = xQueueCreate(2, sizeof(uint32_t));

	xTaskCreate(key_process, "key_process", 2048, NULL, 10, NULL);
	// xTaskCreate(key_scan, "key_scan", 2048, NULL, 10, NULL);
	return MDF_OK;
}

void key_process(void *arg)
{
	uint32_t io_num;
	uint32_t  num = 0;   /* key 按键号 */
	mdf_err_t err;

	while (pdTRUE)
	{
		if(xQueueReceive(key_evt_queue, &io_num, portMAX_DELAY))
		{
			switch (io_num)
			{
				case KEY1_GPIO:num = 0;break;
				case KEY2_GPIO:num = 1;break;
				case KEY3_GPIO:num = 2;break;
				case KEY4_GPIO:num = 3;break;
				default:
					ESP_LOGI(TAG, "%d is not key\n",io_num);
				break;
			}
			if(gpio_get_level(io_num) == 0)
			{
				/* 开启10ms定时器 消抖处理　*/
				if(key[num].time_start == pdFALSE)
				{
					test_ej_arg.arg = (void *)num;
					ESP_ERROR_CHECK( esp_timer_create(&test_ej_arg, &test_ej_handle) );
					err = esp_timer_start_once(test_ej_handle, keyxd * 1000);
					
					if(err == ESP_OK)
					{
						key[num].time_start = pdTRUE;
					}else
					{
						ESP_LOGW(TAG, "esp_timer_start_once err\n");
					}
				}
			}else if(key[num].press_key)/*　如果已经按下并且现在为高电平　说明松开了　*/
			{
				/* 开启10ms定时器 消抖处理　*/
				if(key[num].time_start == pdFALSE)
				{
					test_el_arg.arg = (void *)num;
					ESP_ERROR_CHECK( esp_timer_create(&test_el_arg, &test_el_handle) );
					err = esp_timer_start_once(test_el_handle, keyxd * 1000);
					if(err == ESP_OK)
					{
						key[num].time_start = pdTRUE;
					}else
					{
						ESP_LOGW(TAG, "esp_timer_start_once err\n");
					}
				}
			}
		}
	}
	return;
}