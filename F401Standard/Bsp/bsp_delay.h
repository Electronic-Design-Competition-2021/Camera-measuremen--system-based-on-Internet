#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#include "main.h"

#define User_Delay(n) 	HAL_Delay(n)
#define User_GetTick()	HAL_GetTick()

#endif
