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
#include "driver/uart.h"
//UART1
#define RX1_BUF_SIZE 		(1024)
#define TX1_BUF_SIZE 		(512)
#define TXD1_PIN 			(GPIO_NUM_5)
#define RXD1_PIN 			(GPIO_NUM_4)

void uart_init(void)
{
	//串口配置结构体
	uart_config_t uart1_config,uart2_config;
	//串口参数配置->uart1
	uart1_config.baud_rate = 115200;					//波特率
	uart1_config.data_bits = UART_DATA_8_BITS;			//数据位
	uart1_config.parity = UART_PARITY_DISABLE;			//校验位
	uart1_config.stop_bits = UART_STOP_BITS_1;			//停止位
	uart1_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	//硬件流控
	uart_param_config(UART_NUM_1, &uart1_config);		//设置串口
	//IO映射-> T:IO4  R:IO5
	uart_set_pin(UART_NUM_1, TXD1_PIN, RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(UART_NUM_1, RX1_BUF_SIZE * 2, TX1_BUF_SIZE * 2, 0, NULL, 0);
}

static void uart1_rx_task(void *arg)
{
	uint8_t* data = (uint8_t*) malloc(RX1_BUF_SIZE+1);
	for(;;)
	{
        //获取串口1接收的数据
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX1_BUF_SIZE, 10 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
			//将接收到的数据发出去
			uart_write_bytes(UART_NUM_1, (char *)data, rxBytes);
        }
    }
    free(data);
}
void app_main()
{
	printf("hello Mr.Hu\n");
	ESP_ERROR_CHECK( nvs_flash_init() );
	uart_init();
	//创建串口1接收任务
	xTaskCreate(uart1_rx_task, "uart1_rx_task", 1024*2, NULL, 10, NULL);
}

