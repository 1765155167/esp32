#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "key.h"

BaseType_t press_key  = pdFALSE;/* 按键状态 按下*/
BaseType_t lift_key   = pdFALSE;/* 按键状态 抬起*/
BaseType_t time_start = pdFALSE;/* 消抖动定时器开始标志 */
int64_t    tick;                /* 按键按下的时间 */
int8_t     keyclick   = 1;      /* 按键单击次数 */

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
	.arg = NULL, //不携带参数
	.name = "KeyEliminateJitterTimer" //定时器名字
};
esp_timer_create_args_t test_ct_arg = {
	.callback = &test_timer_ct_cb, //设置回调函数
	.arg = NULL, //不携带参数
	.name = "KeyClickTimer" //定时器名字
};

//中断回调函数
void IRAM_ATTR key_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(key_evt_queue, &gpio_num, NULL);
}

//定时器回调函数(按键消抖)
void test_timer_ej_cb(void *arg) {
	if(gpio_get_level(KEY_GPIO) == 0)
	{
		tick = esp_timer_get_time();
		press_key = pdTRUE;
	}
	time_start = pdFALSE;
}
//定时器回调函数(单击延迟)
void test_timer_ct_cb(void *arg) {
	if(press_key)//如果上一次按键按下短时间内又又按键按下
	{
		keyclick ++;
	}else
	{
		printf("按键单击了%d次\n",keyclick);
		keyclick = 1;
	}
}

void key_init(uint32_t key_gpio)
{
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = BIT(key_gpio);     /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_conf.mode = GPIO_MODE_INPUT;           /*!< GPIO mode: set input/output mode                     */
    io_conf.pull_up_en = 1;       			  /*!< GPIO pull-up                                         */
    io_conf.pull_down_en = 0;                 /*!< GPIO pull-down                                       */
    io_conf.intr_type = GPIO_INTR_ANYEDGE;    /*!< GPIO interrupt type   								*/

	gpio_config(&io_conf);

	//注册中断服务
	gpio_install_isr_service(0);
	//注册中断回调函数
	gpio_isr_handler_add(key_gpio, key_handler, (void *) key_gpio);
	//创建一个消息队列，
	key_evt_queue = xQueueCreate(2, sizeof(uint32_t));
	//创建单次运行的定时器 
	ESP_ERROR_CHECK( esp_timer_create(&test_ej_arg, &test_ej_handle) );
	ESP_ERROR_CHECK( esp_timer_create(&test_ct_arg, &test_ct_handle) );
	return;
}

void key_process(void *arg)
{
	uint32_t io_num;
	esp_err_t err;
	key_init(KEY_GPIO);
	while (pdTRUE)
	{
		if(xQueueReceive(key_evt_queue, &io_num, portMAX_DELAY))
		{
			if(gpio_get_level(io_num) == 0)
			{
				//开启10ms定时器
				if(time_start == pdFALSE)
				{
					err = esp_timer_start_once(test_ej_handle, 10 * 1000);
					if(err == ESP_OK)
					{
						time_start = pdTRUE;
					}else
					{
						ESP_LOGI(TAG, "esp_timer_start_once err\n");
					}
				}
			}else if(press_key)
			{
				tick = esp_timer_get_time() - tick;
				lift_key = pdTRUE;
			}
		}
		if(press_key && lift_key)
		{
			press_key = pdFALSE;
			lift_key = pdFALSE;
			if(tick > longTime * 1000)/* longTime ms */
			{
				printf("key long press once\n");
			}else
			{
				err = esp_timer_start_once(test_ct_handle, clickTime * 1000);/* 100ms */
				if(err == ESP_FAIL)
				{
					ESP_LOGI(TAG, "esp_timer_start_once err\n");
				}
			}
		}
	}
	return;
}