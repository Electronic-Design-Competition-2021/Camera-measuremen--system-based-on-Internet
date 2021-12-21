#ifndef __MOTORCOL_H
#define __MOTORCOL_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/*------------------------------------------
 			  �������Ĭ��ֵ				
------------------------------------------*/
#define DEFAULT_SPEED	 (10)	   
#define DEFAULT_POSITION (11000)
#define DEFAULT_POWER    (200)
/*------------------------------------------
 				����ṹ��				
------------------------------------------*/
typedef struct
{
	        float Offset;	  //����ƫ����
 			float CurPos;
			float PrevPos;
			float CurAcc;
			float PrevSpeed;

	volatile float SetXPos;	  //�趨λ��
	volatile float SetYPos;	  //�趨λ��
	volatile float SetSpeed;  //�趨�ٶ�
	
	volatile float CurXPos;	  //��ǰλ��
	volatile float CurYPos;	  //��ǰλ��
	volatile float CurSpeed;  //��ǰ�ٶ�ʸ��

	volatile int32_t  PWM;	      //PWM
	volatile uint8_t  ShootFlag;
	volatile uint8_t  AdjustFlag;
	volatile uint8_t  ErrFlag;

	volatile uint32_t SetMaxPos;	  //����趨���λ��
	volatile uint32_t SetMaxPower;	  //����趨�������
	volatile int32_t  SetMaxSpeed;	  //����趨����ٶ�
		
}M1TypeDef,M2TypeDef;

void MotorMove(int32_t pwm1,int32_t pwm2);


void Motor_Run(void);


void Motor_Init(void);



#define User_PWM_Set1(v) TIM4->CCR3=(v)
#define User_PWM_Set2(v) TIM4->CCR4=(v)

#define User_Motor_MAX 1000

#endif
