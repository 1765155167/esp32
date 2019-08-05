#include "kalman_math.h"

static const char *TAG = "get_started";

// #define STD_LOCAL_LOG_LEVEL STD_LOG_DEBUG

kalman_t *mdf_build_kalman(float Q, float R, float init_value)
{
    kalman_t *kalman = MDF_MALLOC(sizeof(kalman_t));
    kalman->Q = Q;
    kalman->R = R;
    kalman->x_last = init_value;
    return kalman;
}

float mdf_calcu_kalman(kalman_t *k, float raw)
{
    k->x_mid = k->x_last;                       		//x_last=x(k-1|k-1),x_mid=x(k|k-1)
    k->p_mid = k->p_last + k->Q;                     	//p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=噪声

    /*
     *  卡尔曼滤波的五个重要公式
     */
    k->kg = k->p_mid / (k->p_mid + k->R);                //kg为kalman filter，R 为噪声
    k->x_now = k->x_mid + k->kg * (raw - k->x_mid);  	 //估计出的最优值
    k->p_now = (1 - k->kg) * k->p_mid;                 	 //最优值对应的covariance
    k->p_last = k->p_now;                     			 //更新covariance 值
    k->x_last = k->x_now;                     			 //更新系统状态值

    return k->x_now;
}

void mdf_free_kalman(kalman_t *k)
{
    MDF_FREE(k);
}

uint64_t mdf_up_aline(uint64_t num, uint64_t aline)
{
    if(num % aline == 0)
        return num;
    
    uint64_t mod = num % aline;
    num = num + (aline - mod);
    return num;
}