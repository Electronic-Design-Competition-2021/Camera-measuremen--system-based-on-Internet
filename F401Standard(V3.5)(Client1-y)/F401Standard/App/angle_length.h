#ifndef __ANGLE_LENGTH_H
#define __ANGLE_LENGTH_H
#include "main.h"

#define center_x_min 225
#define center_x_max 235

void Get_circle(void);
uint8_t Get_Length(void);
uint8_t Measure_Xmax_Length(void);
void Get_Angle(void);
void vari_init(void);
//extern uint16_t bolb_y_low;//最低点纵坐标
//extern uint16_t bolb_x_low;//最低点横坐标
//extern uint16_t bolb_x_high;//最高点横坐标
//extern uint16_t bolb_y_high;//最高点纵坐标
void Get_circle(void);//节点计算周期
//float length_calc(uint16_t axis_x_low,uint8_t axis_y_low,uint16_t axis_x_high,uint8_t axis_y_high);
//float Angle_calc(void);
//float Lengh_calc(void);
//void bolb_max(void);
#endif
