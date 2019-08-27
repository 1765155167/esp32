#include "led.h"
#include "key.h"
#include "led.h"
#include "mwifi.h"
#include "mdf_common.h"
#include "mupgrade.h"

struct ledflag ledflag[KEY_MAX];
EventGroupHandle_t led_event_group;

static void led_control_task(void * arg);

void set_led(struct led led)
{
	if(led.flag) {
		gpio_set_level(led.io_num,0);
	}else {
		gpio_set_level(led.io_num,1);
	}
}

void unset_led(struct led led)
{
	if(led.flag) {
		gpio_set_level(led.io_num,1);
	}else {
		gpio_set_level(led.io_num,0);
	}
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
	gpio_set_level(LED0, 0);

	io_config.pin_bit_mask = BIT64(LED1);
	gpio_config(&io_config);
	gpio_set_level(LED1, 0);

	io_config.pin_bit_mask = BIT64(LED2);
	gpio_config(&io_config);
	gpio_set_level(LED2, 0);

	io_config.pin_bit_mask = BIT64(LED3);
	gpio_config(&io_config);
	gpio_set_level(LED3, 0);

	io_config.pin_bit_mask = BIT64(LED4);
	gpio_config(&io_config);
	gpio_set_level(LED4, 0);

	io_config.pin_bit_mask = BIT64(LED5);
	gpio_config(&io_config);
	gpio_set_level(LED5, 0);

	io_config.pin_bit_mask = BIT64(LED6);
	gpio_config(&io_config);
	gpio_set_level(LED6, 0);

	io_config.pin_bit_mask = BIT64(LED7);
	gpio_config(&io_config);
	gpio_set_level(LED7, 0);

	io_config.pin_bit_mask = BIT64(LED8);
	gpio_config(&io_config);
	gpio_set_level(LED8, 0);

	/*灯开始状态为全灭*/
	for(int i = 0; i < KEY_MAX; i++) {
		ledflag[i].led_flage = pdFALSE;
	}
	ledflag[0].light.io_num = RELAY1;
	ledflag[0].light.flag = RELAY1_FLAG;/*低电平点亮*/
	ledflag[0].backlight.io_num = RELAY1_LED;
	ledflag[0].backlight.flag = RELAY1_LED_FLAG;/*高电平点亮*/

	ledflag[1].light.io_num = RELAY2;
	ledflag[1].light.flag = RELAY2_FLAG;/*低电平点亮*/
	ledflag[1].backlight.io_num = RELAY2_LED;
	ledflag[1].backlight.flag = RELAY2_LED_FLAG;/*高电平点亮*/

	ledflag[2].light.io_num = RELAY3;
	ledflag[2].light.flag = RELAY3_FLAG;/*低电平点亮*/
	ledflag[2].backlight.io_num = RELAY3_LED;
	ledflag[2].backlight.flag = RELAY3_LED_FLAG;/*高电平点亮*/

	ledflag[3].light.io_num = RELAY4;
	ledflag[3].light.flag = RELAY4_FLAG;/*低电平点亮*/
	ledflag[3].backlight.io_num = RELAY4_LED;
	ledflag[3].backlight.flag = RELAY4_LED_FLAG;/*高电平点亮*/

	for(int i = 0; i < KEY_MAX; i++) {
		if(ledflag[i].led_flage)
		{
			set_led(ledflag[i].light);/*点亮LED灯*/
			unset_led(ledflag[i].backlight);/*关闭背光灯*/
		}else{
			unset_led(ledflag[i].light);
			set_led(ledflag[i].backlight);
		}
	}
	//事件组
	led_event_group = xEventGroupCreate();

	xTaskCreate(led_control_task, "led_control_task", 4 * 1024,
					NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
	return MDF_OK;
}

static void led_control_task(void * arg)
{
	EventBits_t uxBits;
	while(true)
	{
		//死等事件组：CONNECTED_BIT | ESPTOUCH_DONE_BIT
		uxBits = xEventGroupWaitBits(led_event_group, LED_FLAG_CHANGE, true, false, portMAX_DELAY);
		if (uxBits & LED_FLAG_CHANGE)
		{
			for(int i = 0; i < KEY_MAX; i++) {
				if(ledflag[i].led_flage)
				{
					set_led(ledflag[i].light);/*点亮LED灯*/
					unset_led(ledflag[i].backlight);/*关闭背光灯*/
				}else{
					unset_led(ledflag[i].light);
					set_led(ledflag[i].backlight);
				}
			}
		}
	}
}