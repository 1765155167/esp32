#include "key.h"

mykey_t key[4];/* 按键标志结构体 */
static void test_timer_ej_cb(void* arg);
static void test_timer_ct_cb(void* arg);
//定义一个消息队列
static xQueueHandle key_evt_queue = NULL;
static const char *TAG = "key";
//静态声明2个定时器的回调函数
void test_timer_ej_cb(void *arg);/* 按键消抖 */
void test_timer_ct_cb(void *arg);/* 单击延迟 */
//定义2个定时器句柄
esp_timer_handle_t test_ej_handle = 0;
esp_timer_handle_t test_ct_handle = 0;
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_ej_arg = {
	.callback = &test_timer_ej_cb, //设置回调函数
	.name = "KeyEliminateJitterTimer" //定时器名字
};
esp_timer_create_args_t test_ct_arg = {
	.callback = &test_timer_ct_cb, //设置回调函数
	.name = "KeyClickTimer" //定时器名字
};

//中断回调函数
void IRAM_ATTR key_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(key_evt_queue, &gpio_num, NULL);
}

//定时器回调函数(按键消抖)
static void test_timer_ej_cb(void* arg)
{
	esp_timer_delete(test_ej_handle);
	uint32_t num = (uint32_t)arg;
	if(gpio_get_level(key[num].io_num) == 0)
	{
		key[num].tick = esp_timer_get_time();
		key[num].press_key = pdTRUE;
	}
	key[num].time_start = pdFALSE;
}
//定时器回调函数(单击延迟)
static void test_timer_ct_cb(void *arg) 
{
	esp_timer_delete(test_ct_handle);
	uint32_t num = (uint32_t)arg;
	if(key[num].press_key)//如果上一次按键按下短时间内又又按键按下
	{
		key[num].keyclick ++;
	}else
	{
		ESP_LOGI(TAG, "按键%d单击了%d次",key[num].io_num,key[num].keyclick);
		key[num].keyclick = 1;
	}
}

void key_init()
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
    // io_conf.pull_up_en = 1;       			  /*!< GPIO pull-up                                         */
    // io_conf.pull_down_en = 0;                 /*!< GPIO pull-down                                       */
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
	return;
}

void key_process(void *arg)
{
	uint32_t io_num;
	uint32_t  num = 0;   /* key 按键号 */
	esp_err_t err;
	key_init();

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
					err = esp_timer_start_once(test_ej_handle, 10 * 1000);
					
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
				key[num].press_key = pdFALSE;
				key[num].tick = esp_timer_get_time() - key[num].tick;
				if(key[num].tick > longTime * 1000)/* longTime ms */
				{
					ESP_LOGI(TAG,"%d key long press once",key[num].io_num);/* 按键长按 */
				}else
				{
					/*　判断按键是否为多击 */
					test_ct_arg.arg = (void *)num;
					ESP_ERROR_CHECK( esp_timer_create(&test_ct_arg, &test_ct_handle) );
					err = esp_timer_start_once(test_ct_handle, clickTime * 1000);/* clickTime ms */
					if(err == ESP_FAIL)
					{
						ESP_LOGW(TAG, "esp_timer_start_once err\n");
					}
				}
			}
		}
	}
	return;
}