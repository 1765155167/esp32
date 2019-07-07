#include "pwm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

ledc_timer_config_t ledc_timer = { 
		.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
		.freq_hz = 5000,                      // frequency of PWM signal
		.speed_mode = LEDC_HS_MODE,           // timer mode
		.timer_num = LEDC_HS_TIMER            // timer index
};

ledc_channel_config_t ledc_channel = { 
	.channel =LEDC_HS_CH1_CHANNEL, 
	.duty = 0, 
	.gpio_num = LEDC_HS_CH1_GPIO, 
	.speed_mode =LEDC_HS_MODE, 
	.timer_sel = LEDC_HS_TIMER 
};

void pwm_init(void)
{
	// Set configuration of timer0 for high speed channels
	ledc_timer_config(&ledc_timer);
	// Set LED Controller with previously prepared configuration
	ledc_channel_config(&ledc_channel);
	// Initialize fade service.
	ledc_fade_func_install(0);
}