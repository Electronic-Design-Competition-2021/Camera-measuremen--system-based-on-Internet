#include "motorcol.h"
#include "stdio.h"
#include "math.h"
#include "pid.h"
#include "stdlib.h"
#include "main.h"
#include "tim.h"
#include "bsp_delay.h"
#include "bsp_pwm.h"
#include "usart.h"
#include "bluetooth.h"
#include "gpio.h"

_Bool PWM_IN1_flag = 0;
_Bool PWM_IN2_flag = 0;
_Bool PWM_IN3_flag = 0;
_Bool PWM_IN4_flag = 0;
_Bool Toggle_flag  = 0;
float PWM_IN1_val  = 0;
float PWM_IN2_val  = 0;
float PWM_IN3_val  = 0;
float PWM_IN4_val  = 0;





/*------------------------------------------
 函数功能:电机初始化

------------------------------------------*/
int32_t M1_PWM , M2_PWM;


void Motor_Init(void)
{
	__HAL_TIM_ENABLE(&htim4);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
}


void Motor_Run(void)
{
	MotorMove(M1_PWM,M2_PWM);
}
/*------------------------------------------
 函数功能:电机方向控制

------------------------------------------*/


void Motor1(int32_t v)
{
	if(v >= 0)
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_RESET);
		User_PWM_Set1(v);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET);
		User_PWM_Set1(User_Motor_MAX + v);
	}
}
	
void Motor2(int32_t v)
{
	if(v >= 0)
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_RESET);
		User_PWM_Set2(v);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_SET);
		User_PWM_Set2(User_Motor_MAX + v);
	}
}


/*------------------------------------------
 函数功能:电机底层驱动函数
 函数说明:

------------------------------------------*/
void MotorMove(int32_t pwm1,int32_t pwm2)
{
	Motor1(pwm1);
	Motor2(pwm2);
}


/*------------------------------------------
 函数功能:电机底层驱动函数
 函数说明:外部中断获取电机转速
	PA8  —> MOTOR1
	PB15 —> MOTOR2
------------------------------------------*/
int32_t Motor1_val;//解码器获取电机转速
int32_t Motor2_val;


void EXTI9_5_IRQHandler(void)
{
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_8) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
	
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) != GPIO_PIN_RESET)
	{
		
	}
	else
	{
		
	}
  }
}
/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI4_IRQHandler(void)
{
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	  
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15) == SET)
	{
		
	} 
	else
	{
	
	}
  }
}
