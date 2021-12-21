#include "angle_length.h"
#include "openmv.h"
#include "main.h"
#include "bsp_delay.h"
#include "math.h"
extern uint16_t blob_x_pos;//�ڵ㴮�ڽ�������
extern uint16_t blob_y_pos;

uint16_t circle;//����

uint8_t angle; //�ڶ�ƫ��

uint8_t circle_begin_flag = 0;
uint16_t Last_tick;

void Get_circle(void)//�ڵ��������
{
	if(blob_x_pos >= center_x_min && blob_x_pos <= center_x_max)
	{
		if(circle_begin_flag == 0)//�״δ�������¼����ʼʱ��
		{
			Last_tick = User_GetTick();
			circle_begin_flag = 1;
		}
		else if(circle_begin_flag == 1)//���״δ������������ε���ƽ��λ�õ�ʱ���
		{
			circle = 2*(User_GetTick()-Last_tick);
		}
		
	}
}

//void Get_length(void)//�ն˼���ڳ�l
//{
//	length = (circle*circle)*9.8/(4*(3.1415)*(3.1415));//circleΪ��һ�ڵ�������
//}

//����2
extern uint16_t blob_x_buf[3];		//��ʷ��������
uint16_t x_max = 0;
uint16_t x_max_temp = 0;	//�ɽ����ж�ʵʱ����
uint16_t get_x_seconds = 0;//�������
uint8_t x_max_capture_flag = 0;

uint32_t x_max_tick = 0;	//��׽���ֵ��ʱ��
uint32_t x_max_last_tick = 0;	//��һ�β�׽���ֵ��ʱ��

float length;//����l

//������ʼ����keil�������������Ż��󣬱�����ֵ�����255��65535�ȣ�Ȼ���ѱ����Ż����ˣ�SPI2���յ�ͼ��Ứ�������Գ����²�
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

uint8_t Get_Length(void);//����
/***************************************************************************/
uint8_t Measure_Xmax_Length(void) //�ڵ��ȡx����������������
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
		//�ҵ����ֵ
		x_max = x_max_temp;
		x_max_temp = 0;
		x_max_capture_flag = 0;
		get_x_seconds++;
				
		return 1+Get_Length();	//���㳤��:����ó��ȣ�����2�����򷵻�1
	}
	return 0;
}
/***************************************************************************/
//k = (1/(2pi))^2 = 0.02533
//g = 9.8
//k_length = k*g = 0.2482f
const float k_length = 0.2482f;
float length_temp = 0;		//������ʱ��������λcm
float T = 0;				//���ڣ���λ0.1s
uint8_t Get_Length(void)
{
	uint8_t res = 0;
	if(x_max_last_tick != 0)
	{
		T = (x_max_tick - x_max_last_tick)/100.0;		//�������ڣ���λ0.1s
		length_temp = k_length*T*T;			//��λcm
		
		if(length_temp <= 180 && length_temp >= 20)
		{
			length = length_temp;
			res = 1;
		}
	}
	x_max_last_tick = x_max_tick;		//������ʷֵ
	return res;
}
/***************************************************************************/

//����1
//extern uint16_t blob_x_buf[5];
//uint16_t x_max;
//uint16_t get_x_seconds = 0;//�������
//void Get_X_Max(void)//�ڵ��ȡx���������
//{
//	if(blob_x_buf[0]<blob_x_buf[2]  &&  blob_x_buf[1]<blob_x_buf[2] && blob_x_buf[3]<blob_x_buf[2]&&blob_x_buf[4]<blob_x_buf[2])
//	{
//		x_max = blob_x_buf[2];//�ҳ��յ� 
//		get_x_seconds++;
//	}
//}

//uint16_t x_max_1;
//uint16_t x_max_2;
//void Get_Angle(void)//�ն˼���ڶ�ƽ��ƫת�Ƕ�
//{
//	angle = atan(x_max_2/x_max_1);
//}


///*�Ƕ�Ϊ0�Ȼ�90��ʱ���Ƕ���⺯��*/
///*parameter:                
//axis_x_low-------->��͵�X���꣬ӦΪ0
//axis_y_low-------->��͵�Y����
//axis_x_high------->��ߵ�X����
//axis_y_high------->��ߵ�X����
//*/

//extern uint16_t blob_x;
//extern uint16_t temp_blob_y;
//float length;
////	uint16_t y_intercept;//y��ؾ�
//float length_calc(uint16_t axis_x_low,uint8_t axis_y_low,uint16_t axis_x_high,uint8_t axis_y_high)
//{
//	float Slope,intercept_Slope;//б�ʣ�����,�ؾ�б��
//	int16_t Vertical_bisector_x,Vertical_bisector_y;//��ֱƽ������X��Y������
//	int16_t y_intercept;//y��ؾ�
//	Slope=(axis_y_high-axis_y_low)/(axis_x_high-axis_x_low);//���������������֮��б��
//	intercept_Slope=-(1/Slope);//��ֱƽ����б��
//	Vertical_bisector_x=(axis_x_high+axis_x_low)/2;//X
//	Vertical_bisector_y=(axis_y_high+axis_y_low)/2;//Y
//	y_intercept=-intercept_Slope*Vertical_bisector_x+Vertical_bisector_y;//y��ؾ�
//	length=y_intercept-axis_y_low;//����
//	return length;
//}

