#include "angle_length.h"
#include "openmv.h"
#include "main.h"
#include "bsp_delay.h"
#include "math.h"
extern uint16_t blob_x_pos;//节点串口接收坐标
extern uint16_t blob_y_pos;

uint16_t circle;//周期

uint8_t angle; //摆动偏角

uint8_t circle_begin_flag = 0;
uint16_t Last_tick;

void Get_circle(void)//节点计算周期
{
	if(blob_x_pos >= center_x_min && blob_x_pos <= center_x_max)
	{
		if(circle_begin_flag == 0)//首次触发，记录下起始时间
		{
			Last_tick = User_GetTick();
			circle_begin_flag = 1;
		}
		else if(circle_begin_flag == 1)//非首次触发，计算两次到达平衡位置的时间差
		{
			circle = 2*(User_GetTick()-Last_tick);
		}
		
	}
}

//void Get_length(void)//终端计算摆长l
//{
//	length = (circle*circle)*9.8/(4*(3.1415)*(3.1415));//circle为任一节点测得周期
//}

//方案2
extern uint16_t blob_x_buf[3];		//历史三次数据
uint16_t x_max = 0;
uint16_t x_max_temp = 0;	//由接收中断实时更新
uint16_t get_x_seconds = 0;//捕获次数
uint8_t x_max_capture_flag = 0;

uint32_t x_max_tick = 0;	//捕捉最大值的时间
uint32_t x_max_last_tick = 0;	//上一次捕捉最大值的时间

float length;//长度l

//变量初始化：keil开启三级编译优化后，变量初值变成了255、65535等（然而把编译优化关了，SPI2接收的图像会花），所以出此下策
void vari_init(void)
{
	x_max = 0;
	length = 0;
	x_max_temp = 0;
	get_x_seconds = 0;
	x_max_capture_flag = 0;
	x_max_tick = 0;
	x_max_last_tick = 0;
}

uint8_t Get_Length(void);//声明
/***************************************************************************/
uint8_t Measure_Xmax_Length(void) //节点获取x向最大坐标和绳长度
{
	if(blob_x_buf[0] > blob_x_buf[1]&&blob_x_buf[1] > blob_x_buf[2] && x_max_capture_flag == 0)
	{
		x_max_capture_flag = 1;	
	}
	else if(blob_x_buf[0] < blob_x_buf[1]&&blob_x_buf[1] < blob_x_buf[2] && x_max_capture_flag == 1)
	{
		x_max_capture_flag = 2;
	}

	if(x_max_capture_flag == 2)
	{
		//找到最大值
		x_max = x_max_temp;
		x_max_temp = 0;
		x_max_capture_flag = 0;
		get_x_seconds++;
				
		return 1+Get_Length();	//计算长度:若获得长度：返回2，否则返回1
	}
	return 0;
}
/***************************************************************************/
//k = (1/(2pi))^2 = 0.02533
//g = 9.8
//k_length = k*g = 0.2482f
const float k_length = 0.2482f;
float length_temp = 0;		//长度临时变量：单位cm
float T = 0;				//周期：单位0.1s
uint8_t Get_Length(void)
{
	uint8_t res = 0;
	if(x_max_last_tick != 0)
	{
		T = (x_max_tick - x_max_last_tick)/100.0;		//计算周期，单位0.1s
		length_temp = k_length*T*T;			//单位cm
		
		if(length_temp <= 180 && length_temp >= 20)
		{
			length = length_temp;
			res = 1;
		}
	}
	x_max_last_tick = x_max_tick;		//更新历史值
	return res;
}
/***************************************************************************/

//方案1
//extern uint16_t blob_x_buf[5];
//uint16_t x_max;
//uint16_t get_x_seconds = 0;//捕获次数
//void Get_X_Max(void)//节点获取x向最大坐标
//{
//	if(blob_x_buf[0]<blob_x_buf[2]  &&  blob_x_buf[1]<blob_x_buf[2] && blob_x_buf[3]<blob_x_buf[2]&&blob_x_buf[4]<blob_x_buf[2])
//	{
//		x_max = blob_x_buf[2];//找出拐点 
//		get_x_seconds++;
//	}
//}

//uint16_t x_max_1;
//uint16_t x_max_2;
//void Get_Angle(void)//终端计算摆动平面偏转角度
//{
//	angle = atan(x_max_2/x_max_1);
//}


///*角度为0度或90度时，角度求解函数*/
///*parameter:                
//axis_x_low-------->最低点X坐标，应为0
//axis_y_low-------->最低点Y坐标
//axis_x_high------->最高点X坐标
//axis_y_high------->最高点X坐标
//*/

//extern uint16_t blob_x;
//extern uint16_t temp_blob_y;
//float length;
////	uint16_t y_intercept;//y轴截距
//float length_calc(uint16_t axis_x_low,uint8_t axis_y_low,uint16_t axis_x_high,uint8_t axis_y_high)
//{
//	float Slope,intercept_Slope;//斜率，长度,截距斜率
//	int16_t Vertical_bisector_x,Vertical_bisector_y;//垂直平分线上X与Y点坐标
//	int16_t y_intercept;//y轴截距
//	Slope=(axis_y_high-axis_y_low)/(axis_x_high-axis_x_low);//最高坐标和最低坐标之间斜率
//	intercept_Slope=-(1/Slope);//垂直平分线斜率
//	Vertical_bisector_x=(axis_x_high+axis_x_low)/2;//X
//	Vertical_bisector_y=(axis_y_high+axis_y_low)/2;//Y
//	y_intercept=-intercept_Slope*Vertical_bisector_x+Vertical_bisector_y;//y轴截距
//	length=y_intercept-axis_y_low;//长度
//	return length;
//}

