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
#include "key.h"

#define LED_GPIO 17 /* led 引脚号 */
void led_init(void);/* led 初始化 */

extern ledc_timer_config_t ledc_timer;
extern ledc_channel_config_t ledc_channel;

void led_init(void)
{
	gpio_config_t io_config;
	io_config.pin_bit_mask = BIT(LED_GPIO);     /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_config.mode = GPIO_MODE_INPUT;         	/*!< GPIO mode: set input/output mode*/
    io_config.pull_up_en = 0;       			/*!< GPIO pull-up*/
    io_config.pull_down_en = 0;  				/*!< GPIO pull-down*/
    io_config.intr_type = GPIO_INTR_DISABLE;    /*!< GPIO interrupt type */    

	gpio_config(&io_config);   
	//gpio_set_level(LED_GPIO, 0);
}

// void flash_led(void *arg)
// {
// 	led_init();
// 	while (pdTRUE)
// 	{
// 		printf("led...\n");
// 		gpio_set_level(LED_GPIO, 1);
// 		vTaskDelay(1000 / portTICK_RATE_MS);
// 		gpio_set_level(LED_GPIO, 0);
// 		vTaskDelay(1000 / portTICK_RATE_MS);
// 	}
// 	return;
// }
static void read_gpio(void *arg)
{
	led_init();
	while (pdTRUE)
	{
		printf("gpio:%d\n",gpio_get_level(LED_GPIO));
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
void app_main()
{
	printf("hello Mr.Hu\n");
	ESP_ERROR_CHECK( nvs_flash_init() );
	//设置GPIO输出
	xTaskCreate(read_gpio, "flash_led", 2048, NULL, 10, NULL);
	// xTaskCreate(flash_led, "flash_led", 2048, NULL, 10, NULL);
	// xTaskCreate(key_process, "key_process", 2048, NULL, 10, NULL);
	// xTaskCreate(pwm_test, "pwm_test", 2048, NULL, 10, NULL);
}

