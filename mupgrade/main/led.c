#include "led.h"
#include "key.h"
#include "led.h"
#include "tcp.h"
#include "mwifi.h"
#include "mdf_common.h"
#include "mupgrade.h"

static void flash_led(void *arg);

void set_led(int led)
{
	gpio_set_level(led,0);
}

void unset_led(int led)
{
	gpio_set_level(led,1);
}

void targiet_led(int led)
{

}

mdf_err_t led_init(void)
{
	static bool flag = false;
	if(flag)
	{
		return MDF_OK;
	}
	flag = true;

	gpio_config_t io_config;
	io_config.pin_bit_mask = BIT64(LED0);     /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    io_config.mode = GPIO_MODE_OUTPUT;         	/*!< GPIO mode: set input/output mode*/
    io_config.pull_up_en = 0;       			/*!< GPIO pull-up*/
    io_config.pull_down_en = 0;  				/*!< GPIO pull-down*/
    io_config.intr_type = GPIO_INTR_DISABLE;    /*!< GPIO interrupt type */
	gpio_config(&io_config);
	unset_led(LED0);

	io_config.pin_bit_mask = BIT64(LED1);
	gpio_config(&io_config);
	unset_led(LED1);

	io_config.pin_bit_mask = BIT64(LED2);
	gpio_config(&io_config);
	unset_led(LED2);

	io_config.pin_bit_mask = BIT64(LED3);
	gpio_config(&io_config);
	unset_led(LED3);

	io_config.pin_bit_mask = BIT64(LED4);
	gpio_config(&io_config);
	unset_led(LED4);

	io_config.pin_bit_mask = BIT64(LED5);
	gpio_config(&io_config);
	unset_led(LED5);

	io_config.pin_bit_mask = BIT64(LED6);
	gpio_config(&io_config);
	unset_led(LED6);

	io_config.pin_bit_mask = BIT64(LED7);
	gpio_config(&io_config);
	unset_led(LED7);

	io_config.pin_bit_mask = BIT64(LED8);
	gpio_config(&io_config);
	unset_led(LED8);

	// xTaskCreate(flash_led, "flash_led", 4 * 1024,
	// 				NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
	return MDF_OK;
}

static void flash_led(void *arg)
{
	led_init();
	while (true)
	{
		printf("flash led is running ...\n");
		unset_led(LED0);
		unset_led(LED1);
		unset_led(LED2);
		unset_led(LED3);
		unset_led(LED4);
		unset_led(LED5);
		unset_led(LED6);
		unset_led(LED7);
		unset_led(LED8);
		vTaskDelay(1000 / portTICK_RATE_MS);
		set_led(LED0);
		set_led(LED1);
		set_led(LED2);
		set_led(LED3);
		set_led(LED4);
		set_led(LED5);
		set_led(LED6);
		set_led(LED7);
		set_led(LED8);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}
