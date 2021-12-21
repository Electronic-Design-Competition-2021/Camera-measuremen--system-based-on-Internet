/**
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  * @file      	bsp_gpio.c/h
  * @brief     	gpio 板级支持（stm32f401 HAL库）
  * @note      	
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     July-1-2021     doee            1. 完成
  * @blog		https://blog.csdn.net/weixin_44578655?spm=1001.2101.3001.5343
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  */
  
#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "main.h"

#define User_GPIO_ReadPin(Port,Pin) 		HAL_GPIO_ReadPin(Port,Pin)
#define User_GPIO_WritePin(Port,Pin,Sta) 	HAL_GPIO_WritePin(Port,Pin,Sta)
#define User_GPIO_TogglePin(Port,Pin)	 	HAL_GPIO_TogglePin(Port,Pin)

#endif
