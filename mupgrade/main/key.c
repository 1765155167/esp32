#include "key.h"
#include "key.h"
#include "led.h"
#include "tcp.h"
#include "mwifi.h"
#include "mdf_common.h"
#include "mupgrade.h"
static const char *TAG = "mesh-key";
static void key_process(void *arg);

mykey_t key[4];/* 按键标志结构体 */
static void test_timer_ej_cb(void* arg);
static void test_timer_el_cb(void* arg);
//定义一个消息队列
static xQueueHandle key_evt_queue = NULL;

//定义2个定时器句柄
esp_timer_handle_t test_ej_handle[4] = {0};/*按下消抖*/
esp_timer_handle_t test_el_handle[4] = {0};/*松开消抖*/
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_ej_arg[4] = {
	{
		.callback = &test_timer_ej_cb, //设置回调函数
		.arg = (void *)0,
		.name = "Key1EliminateJitterTimer1" //定时器名字
	},
	{
		.callback = &test_timer_ej_cb, //设置回调函数
		.arg = (void *)1,
		.name = "Key2EliminateJitterTimer1" //定时器名字
	},
	{
		.callback = &test_timer_ej_cb, //设置回调函数
		.arg = (void *)2,
		.name = "Key3EliminateJitterTimer1" //定时器名字
	},
	{
		.callback = &test_timer_ej_cb, //设置回调函数
		.arg = (void *)3,
		.name = "Key4EliminateJitterTimer1" //定时器名字
	}
};
esp_timer_create_args_t test_el_arg[4] = {
	{
		.callback = &test_timer_el_cb, //设置回调函数
		.arg = (void *)0,
		.name = "Key1EliminateJitterTimer2" //定时器名字
	},
	{
		.callback = &test_timer_el_cb, //设置回调函数
		.arg = (void *)1,
		.name = "Key2EliminateJitterTimer2" //定时器名字
	},
	{
		.callback = &test_timer_el_cb, //设置回调函数
		.arg = (void *)2,
		.name = "Key3EliminateJitterTimer2" //定时器名字
	},
	{
		.callback = &test_timer_el_cb, //设置回调函数
		.arg = (void *)3,
		.name = "Key4EliminateJitterTimer2" //定时器名字
	}
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
	uint32_t num = (uint32_t)arg;
	key[num].time_start = pdFALSE;
	if(gpio_get_level(key[num].io_num) == 0)
	{
		key[num].tick = esp_timer_get_time();
		MDF_LOGD("press %d:tick = %lld", num, key[num].tick);
		key[num].press_key = pdTRUE;
		set_led(key[num].relay_led);
		key[num].relay_led_flag = pdTRUE;
	}
}
//定时器回调函数(按键松开消抖)
static void test_timer_el_cb(void* arg)
{	
	uint32_t num = (uint32_t)arg;
	key[num].time_start = pdFALSE;
	if(gpio_get_level(key[num].io_num) == 1)
	{
		unset_led(key[num].relay_led);
		key[num].relay_led_flag = pdFALSE;

		key[num].press_key = pdFALSE;
		key[num].tick = esp_timer_get_time() - key[num].tick;
		MDF_LOGD("1:lift %d:tick = %lld", num, esp_timer_get_time());
		MDF_LOGD("2:lift %d:tick = %lld", num, key[num].tick);

		if(key[num].tick > longTime * 1000)/* longTime ms */
		{
			ESP_LOGI(TAG,"按键%d长按",num+1);/* 按键长按 */
		}else {
			ESP_LOGI(TAG, "按键%d单击",num+1);
		}
		/*led翻转*/
		if(!key[num].status_led_flag) {
			set_led(key[num].status_led);
			key[num].status_led_flag = pdTRUE;
		}else {
			unset_led(key[num].status_led);
			key[num].status_led_flag = pdFALSE;
		}
	}
}
mdf_err_t key_init(void)
{
	static bool init_flag = pdFALSE;
	if(init_flag) {
		return MDF_OK;
	}
	gpio_config_t io_conf;
	memset(key,0,4*sizeof(mykey_t));

	key[0].io_num = KEY1_GPIO;
	key[0].relay_led = RELAY1;
	key[0].status_led = RELAY1_LED;
	
	key[1].io_num = KEY2_GPIO;
	key[1].relay_led = RELAY2;
	key[1].status_led = RELAY2_LED;

	key[2].io_num = KEY3_GPIO;
	key[2].relay_led = RELAY3;
	key[2].status_led = RELAY3_LED;

	key[3].io_num = KEY4_GPIO;
	key[3].relay_led = RELAY4;
	key[3].status_led = RELAY4_LED;

	for(int i = 0; i < 4; i++) {
		key[i].press_key  = pdFALSE;        /* 按键状态 按下*/
		key[i].lift_key   = pdFALSE;        /* 按键状态 抬起*/
		key[i].time_start = pdFALSE;        /* 消抖动定时器开始标志 */
		key[i].relay_led_flag = pdFALSE;    /*熄灭*/
		key[i].status_led_flag = pdFALSE;
		unset_led(key[i].status_led);
		unset_led(key[i].relay_led);
	}

	io_conf.pin_bit_mask = BIT64(KEY1_GPIO);    /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_conf.mode = GPIO_MODE_INPUT;           /*!< GPIO mode: set input/output mode                     */
    io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 1;
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
	/*创建定时器*/
	for(int i = 0; i < 4; i++) {
		ESP_ERROR_CHECK( esp_timer_create(&test_ej_arg[i], &test_ej_handle[i]) );
		ESP_ERROR_CHECK( esp_timer_create(&test_el_arg[i], &test_el_handle[i]) );
	}
	xTaskCreate(key_process, "key_process", 2048, NULL, 10, NULL);
	init_flag = pdTRUE;
	return MDF_OK;
}

static void key_process(void *arg)
{
	uint32_t io_num;
	uint32_t  num = 0;   /* key 按键号 */
	mdf_err_t err;

	while (pdTRUE)
	{
		/*等待按键IO产生跳沿*/
		if(xQueueReceive(key_evt_queue, &io_num, portMAX_DELAY)) {
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
			if(gpio_get_level(io_num) == 0) {
				/* 开启10ms定时器 消抖处理　*/
				if(key[num].time_start == pdFALSE && !key[num].press_key)
				{
					err = esp_timer_start_once(test_ej_handle[num], DebounceTime * 1000);
					if(err == ESP_OK) {
						key[num].time_start = pdTRUE;
					}else {
						ESP_LOGW(TAG, "esp_timer_start_once err\n");
					}
				}
			}else if (key[num].press_key) {/*　如果已经按下并且现在为高电平　说明松开了　*/
				/* 开启10ms定时器 消抖处理　*/
				if (key[num].time_start == pdFALSE) {
					err = esp_timer_start_once(test_el_handle[num], DebounceTime * 1000);
					if (err == ESP_OK) {
						key[num].time_start = pdTRUE;
					} else {
						ESP_LOGW(TAG, "esp_timer_start_once err\n");
					}
				}
			}
		}
	}
	return;
}