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
#include "led.h"
#include "key.h"


void app_main()
{
	printf("hello Mr.Hu\n");
	ESP_ERROR_CHECK( nvs_flash_init() );
	//设置GPIO输出
	xTaskCreate(key_process, "key_process", 2048, NULL, 10, NULL);
	xTaskCreate(flash_led, "flash led  ", 2048, NULL, 10, NULL);
}

