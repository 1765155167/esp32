#ifndef _PWM_H_
#define _PWM_H_

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1
#define LEDC_HS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_HS_CH1_GPIO       32


#define LEDC_TEST_DUTY         5000 /* max 2^13 - 1 */
#define LEDC_TEST_FADE_TIME    3000 /* speed */

void pwm_init(void);
void pwm_test(void * arg);
#endif
