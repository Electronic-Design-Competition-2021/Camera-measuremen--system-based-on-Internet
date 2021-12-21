#include "bsp_ec11.h"
#include "tim.h"
#include "bsp_delay.h"
#include "bsp_gpio.h"

void ec11_Init(void)
{
	
}


int ec11_num = 0;
_Bool Rotary;
uint32_t Last_EXTI3_Tim = 0;

void EXTI3_IRQHandler(void)
{
	uint8_t step = 0;
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
		if(User_GetTick() - Last_EXTI3_Tim >= 5)//EC11消抖
		{
			step = 200/(User_GetTick() - Last_EXTI3_Tim);	//步进值跟随旋转速度变化，旋转速度越大，步进值越大,最大200/5
			if(step == 0)
				step = 1;		//最小步进值为1
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15) != GPIO_PIN_RESET)
				ec11_num += step;	
			else
				ec11_num -= step;
			Last_EXTI3_Tim = User_GetTick();
		}
	}
}	
uint8_t button_flag;

void EC11_ScanButton(void)
{
	static uint8_t downFlag = 0;
	if(User_GPIO_ReadPin(BUTTON_GPIO_Port,BUTTON_Pin) == 0 && downFlag == 0)
	{
		button_flag = 1;
		downFlag = 1;
	}
	else if(User_GPIO_ReadPin(BUTTON_GPIO_Port,BUTTON_Pin) == 1)
	{
		downFlag = 0;
	}
}
uint8_t EC11_getButtonFlag(void)
{
	if(button_flag)
	{
		button_flag = 0;	//清零
		return 1;
	}
	else
		return 0;
}

int32_t EC11_getCount(void)
{
	return ec11_num;
}
