#ifndef __BSP_EC11_H__
#define __BSP_EC11_H__
#include "struct_typedef.h"

void ec11_Init(void);
int32_t EC11_getCount(void);
uint8_t EC11_getButtonFlag(void);
void EC11_ScanButton(void);
#endif

