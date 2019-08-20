#include "led.h"
#include "key.h"
#include "config.h"
#include "driver/gpio.h"

void led_init(void)
{
	gpio_config_t io_config;
	io_config.pin_bit_mask = BIT64(LED0);     /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_config.mode = GPIO_MODE_OUTPUT;         	/*!< GPIO mode: set input/output mode*/
    io_config.pull_up_en = 0;       			/*!< GPIO pull-up*/
    io_config.pull_down_en = 0;  				/*!< GPIO pull-down*/
    io_config.intr_type = GPIO_INTR_DISABLE;    /*!< GPIO interrupt type */
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);

	io_config.pin_bit_mask = BIT64(LED1);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);

	io_config.pin_bit_mask = BIT64(LED2);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);

	io_config.pin_bit_mask = BIT64(LED3);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);

	io_config.pin_bit_mask = BIT64(LED4);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);

	io_config.pin_bit_mask = BIT64(LED5);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);
	io_config.pin_bit_mask = BIT64(LED6);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);
	io_config.pin_bit_mask = BIT64(LED7);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);
	io_config.pin_bit_mask = BIT64(LED8);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);
}

void flash_led(void *arg)
{
	led_init();
	while (true)
	{
		printf("flash led is running ...\n");
		gpio_set_level(LED0, 1);
		gpio_set_level(LED1, 0);
		gpio_set_level(LED2, 0);
		gpio_set_level(LED3, 1);
		gpio_set_level(LED4, 0);
		gpio_set_level(LED5, 0);
		gpio_set_level(LED6, 0);
		gpio_set_level(LED7, 0);
		gpio_set_level(LED8, 0);
		vTaskDelay(1000 / portTICK_RATE_MS);
		gpio_set_level(LED0, 0);
		gpio_set_level(LED1, 1);
		gpio_set_level(LED2, 1);
		gpio_set_level(LED3, 0);
		gpio_set_level(LED4, 1);
		gpio_set_level(LED5, 1);
		gpio_set_level(LED6, 1);
		gpio_set_level(LED7, 1);
		gpio_set_level(LED8, 1);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}