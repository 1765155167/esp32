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

#include "pwm.h"

#define KEY_GPIO 4 /* key　引脚号 */
#define LED_GPIO 17 /* led 引脚号 */

void led_init(void);/* led 初始化 */
void key_init(uint32_t key_gpio);/* key 初始化 */

BaseType_t press_key = pdFALSE;/* 按键状态 按下*/
BaseType_t lift_key = pdFALSE;/* 按键状态 抬起*/

//静态声明2个定时器的回调函数
void test_timer_once_cb(void *arg);
//定义2个定时器句柄
esp_timer_handle_t test_o_handle = 0;
//定义一个单次运行的定时器结构体
esp_timer_create_args_t test_once_arg = {
	.callback = &test_timer_once_cb, //设置回调函数
	.arg = NULL, //不携带参数
	.name = "TestOnceTimer" //定时器名字
};

extern ledc_timer_config_t ledc_timer;
extern ledc_channel_config_t ledc_channel;

//定义一个消息队列
static 	xQueueHandle key_evt_queue = NULL;

//中断回调函数
void IRAM_ATTR key_handler(void *arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(key_evt_queue, &gpio_num, NULL);
}

//定时器回调函数(按键消抖)
void test_timer_once_cb(void *arg) {
	if(gpio_get_level(KEY_GPIO) == 0)
	{
		press_key = pdTRUE;
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
	ESP_ERROR_CHECK( esp_timer_create(&test_once_arg, &test_o_handle) );
	return;
}

void led_init(void)
{
	gpio_config_t io_config;
	io_config.pin_bit_mask = BIT(LED_GPIO);     /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_config.mode = GPIO_MODE_OUTPUT;         	/*!< GPIO mode: set input/output mode*/
    io_config.pull_up_en = 0;       			/*!< GPIO pull-up*/
    io_config.pull_down_en = 0;  				/*!< GPIO pull-down*/
    io_config.intr_type = GPIO_INTR_DISABLE;    /*!< GPIO interrupt type */    

	gpio_config(&io_config);   
	gpio_set_level(LED_GPIO, 0);
}

void flash_led()
{
	led_init();
	while (pdTRUE)
	{
		printf("led...\n");
		gpio_set_level(LED_GPIO, 1);
		vTaskDelay(1000 / portTICK_RATE_MS);
		gpio_set_level(LED_GPIO, 0);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	return;
}

void key_process()
{
	uint32_t io_num;

	key_init(KEY_GPIO);
	while (pdTRUE)
	{
		if(xQueueReceive(key_evt_queue, &io_num, portMAX_DELAY))
		{
			if(gpio_get_level(io_num) == 0)
			{
				//开启10ms定时器
				ESP_ERROR_CHECK( esp_timer_start_once(test_o_handle, 10 * 1000) );
			}else if(press_key)
			{
				lift_key = pdTRUE;
			}
		}
		//按键处理
		if(press_key && lift_key)
		{
			press_key = pdFALSE;
			lift_key = pdFALSE;
			printf("key press once\n");
		}
	}
	return;
}

void pwm_test()
{
	pwm_init();
	while (1) {
		printf("1. PWM逐渐变大的周期目标 = %d\n", LEDC_TEST_DUTY);
		ESP_ERROR_CHECK( ledc_set_fade_with_time(ledc_channel.speed_mode,
				ledc_channel.channel, LEDC_TEST_DUTY,
				LEDC_TEST_FADE_TIME) );
		ESP_ERROR_CHECK( ledc_fade_start(ledc_channel.speed_mode,
				ledc_channel.channel, LEDC_FADE_NO_WAIT) );
		vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

		printf("2.  PWM逐渐变小的周期目标 = 0\n");
		ESP_ERROR_CHECK( ledc_set_fade_with_time(ledc_channel.speed_mode,
				ledc_channel.channel, 0, LEDC_TEST_FADE_TIME) );
		ESP_ERROR_CHECK( ledc_fade_start(ledc_channel.speed_mode,
				ledc_channel.channel, LEDC_FADE_NO_WAIT) );
		vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);
	}
}
void app_main()
{
	printf("hello Mr.Hu\n");
	ESP_ERROR_CHECK( nvs_flash_init() );
	//设置GPIO输出
	xTaskCreate(flash_led, "flash_led", 2048, NULL, 10, NULL);
	xTaskCreate(key_process, "key_process", 2048, NULL, 10, NULL);
	xTaskCreate(pwm_test, "pwm_test", 2048, NULL, 10, NULL);
}

