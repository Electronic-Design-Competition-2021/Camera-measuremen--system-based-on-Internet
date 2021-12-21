#ifndef __STEERINGENINE_H__
#define __STEERINGENINE_H__

#include "main.h"

typedef struct
{
	float coordinate;
	volatile int32_t  PWM;
}PWM_IN1_TypeDef,PWM_IN2_TypeDef;

void EngineMove(int32_t pwm1,int32_t pwm2);
void User_PWM_Set1(int32_t v);
void User_PWM_Set2(int32_t v);
void User_PWM_IN1_2Init(void);

#endif
