#include "bsp_platform.h"
#include "tim.h"
#include "pid.h"
#include "usart.h"
#include "steeringengine.h"

float PITCH,YAW;

void Platform_Run(void)
{
	EngineMove(YAW,PITCH);
}



