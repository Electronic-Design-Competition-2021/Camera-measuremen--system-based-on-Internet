#include "steeringengine.h"
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "bsp_pwm.h"
#include "tim.h"
#include "pid.h"


pid_type_def M1PID;//PID结构体
pid_type_def M2PID;

fp32 pid[3];//参数初始化

int period1 = 20000;

void User_PWM_IN1_2Init(void)
{
	__HAL_TIM_ENABLE_IT(&htim4,TIM_CHANNEL_1);//开启中断
	__HAL_TIM_ENABLE_IT(&htim4,TIM_CHANNEL_2);
	
	PID_init(&M1PID,PID_POSITION,pid,10000,10000);
	PID_init(&M2PID,PID_POSITION,pid,10000,10000);
}



void User_PWM_Set1(int32_t v)
{
	if(v >= PWM_MAX_1) v = PWM_MAX_1;
	if(v <= PWM_MIN_1) v = PWM_MIN_1;
	
	TIM4->CCR1 = v;
}

void User_PWM_Set2(int32_t v)
{
	if(v >= PWM_MAX_1) v = PWM_MAX_1;
	if(v <= PWM_MIN_1) v = PWM_MIN_1;
	
	TIM4->CCR2 = v;
}

void EngineMove(int32_t pwm1,int32_t pwm2)//舵机底层驱动
{	
		User_PWM_Set1(1200+pwm1);
	
		User_PWM_Set2(1200-pwm2);
}
//a8 b15 b14 c13 14 15 b5
