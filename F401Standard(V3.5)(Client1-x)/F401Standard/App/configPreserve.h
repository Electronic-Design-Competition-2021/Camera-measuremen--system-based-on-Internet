#ifndef __MOTORCOL_H
#define __MOTORCOL_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/*------------------------------------------
 			  电机启动默认值				
------------------------------------------*/
#define DEFAULT_SPEED	 (10)	   
#define DEFAULT_POSITION (11000)
#define DEFAULT_POWER    (200)
/*------------------------------------------
 				电机结构体				
------------------------------------------*/
typedef struct
{
	        float Offset;	  //允许偏差量
 			float CurPos;
			float PrevPos;
			float CurAcc;
			float PrevSpeed;

	volatile float SetXPos;	  //设定位置
	volatile float SetYPos;	  //设定位置
	volatile float SetSpeed;  //设定速度
	
	volatile float CurXPos;	  //当前位置
	volatile float CurYPos;	  //当前位置
	volatile float CurSpeed;  //当前速度矢量

	volatile int32_t  PWM;	      //PWM
	volatile uint8_t  ShootFlag;
	volatile uint8_t  AdjustFlag;
	volatile uint8_t  ErrFlag;

	volatile uint32_t SetMaxPos;	  //软件设定最大位置
	volatile uint32_t SetMaxPower;	  //软件设定最大力量
	volatile int32_t  SetMaxSpeed;	  //软件设定最大速度
		
}M1TypeDef,M2TypeDef;

void MotorMove(int32_t pwm1,int32_t pwm2);


void Motor_Run(void);


void Motor_Init(void);



#define User_PWM_Set1(v) TIM4->CCR3=(v)
#define User_PWM_Set2(v) TIM4->CCR4=(v)

#define User_Motor_MAX 1000

#endif
