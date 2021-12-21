#include <stdio.h>
#include <string.h>
#include <math.h>
#include "udp_demo.h"
#include "W5500_conf.h"
#include "w5500.h"
#include "socket.h"
#include "bsp_lcd_init.h"
#include "bsp_lcd.h"
#include "filter.h"
/***********************用户变量区**************************/

//150cm对应周期为2.5s，10次为25s，题目要求测量时间小于30S
#define LEN_MEASURE_NUM  10
#define ANGLE_MEASURE_NUM  5

#define x1_base  100
#define x2_base  160


float angle_buf[ANGLE_MEASURE_NUM];	//原始角度数据
float angle = 0,angle_raw = 0;		//处理后的角度数据,原始角度数据
uint8_t ang_rx_count = 0;	//数据接收计数

volatile uint8_t udp_establish_flag = 0;

uint16_t x1_max = 0,x2_max = 0;//抓取的最高点位置坐标

float c1_length_raw = 0,c2_length_raw = 0;//摆线长度原始数据
float c1_length = 0,c2_length = 0;	//处理后的length数据
	

uint32_t c1_max_tick=0,c2_max_tick=0;//获取捕获到最大值的时间

uint8_t udp_Tx_buf[3];         //向客户端发送指令缓存数组
uint8_t udp_rx_buf[8000];			//udp单次接收缓存

float c1_length_raw_buff[LEN_MEASURE_NUM],c2_length_raw_buff[LEN_MEASURE_NUM];	//长度原始数据

uint8_t c1_len_rx_count = 0,c2_len_rx_count = 0;	//数据接收计数

uint8_t ang_measure_done_flag = 0;	//angle测量完成标志位
uint8_t c1_len_measure_done_flag = 0;	//c1_len测量完成标志
uint8_t c2_len_measure_done_flag = 0;	//c2_len测量完成标志

extern uint8_t anolog_mode;		//模拟模式标志位




/*******************客户端IP地址**************************/
uint8  c1_ip[4]={192,168,112,44};
uint8  c2_ip[4]={192,168,112,55};

//同时向两节点发送指令：端口号7000
void Tx_udp(uint8 * buff,uint32 len)
{
	switch(getSn_SR(SOCK_UDPS2))                                                /*获取socket的状态*/
	{
		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
			socket(SOCK_UDPS2,Sn_MR_UDP,7000,0);                              /*初始化socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket初始化完成*/
			sendto(SOCK_UDPS2,buff,len, c1_ip, 7000);
			sendto(SOCK_UDPS2,buff,len, c2_ip, 7000);	
		  break;
	}
}

//接收两节点发来的数据
void Rx_udp(uint8 * buff1,uint8 * buff2)
{
	uint16 len=0;
	uint32_t rec_len = 0;
	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;//将三次图像数据累加
	
	switch(getSn_SR(SOCK_UDPS1))                                                /*获取socket的状态*/
	{
		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
			socket(SOCK_UDPS1,Sn_MR_UDP,5000,0);         
			/*初始化socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket初始化完成*/
			/*接收来自Client的数据*/
			udp_establish_flag = 1;
		
			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*清接收中断*/
			}
			
			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*接收到数据*/
			{	
				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//接收缓存
				//Client1
				if(remote_ip[3] == 44)
				{
					/***示波器数据***/
					if(udp_rx_buf[0] == 0xC5 && udp_rx_buf[1] == 0xC5)
					{
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3];	
						if(sum == udp_rx_buf[4])
						{
							memcpy(&x1_max,udp_rx_buf+2,2);	//接收x_max，此时x1_max表示实时坐标
						}
						return;
					}
					/****************************LCD1测量数据*******************************************/
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						//切换lcd cs引脚
						CS_port = LCD_CS_GPIO_Port;
						CS_pin = LCD_CS_Pin;	

						
						//接收到测量数据
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4]+
										udp_rx_buf[5]+udp_rx_buf[6]+udp_rx_buf[7];
						

						if(sum == udp_rx_buf[8])
						{
							//校验和正确
							memcpy(&c1_length_raw,udp_rx_buf+2,4);	//接收x节点测量的length
							memcpy(&x1_max,udp_rx_buf+6,2);	//接收x_max
							
							//计算和存储角度数据
							if(ang_measure_done_flag == 0)	//判断是否已测量完成
							{
								
								c1_max_tick = HAL_GetTick();	//记录时间
								
								int32_t delta_tick = 0;		//时间差
								delta_tick = c1_max_tick -c2_max_tick;
								if(delta_tick <= 300 && delta_tick >= -300)	//判断时间差是否在范围内
								{

									angle_raw = atan2f(x1_max-x1_base,x2_max-x2_base)*57.3f;	//计算角度
									ang_rx_count++;
								
									LCD_ShowIntNum(100,0, ang_rx_count,2,GREEN,BLACK,16);	//显示角度接收计数（右上角）
								
									float angle_temp = 0;
									for(uint8_t i = 0;i<ang_rx_count;i++)
									{
										//由大到小排列
										if(angle_raw > angle_buf[i])
										{
											angle_temp = angle_buf[i];
											angle_buf[i] = angle_raw;	//插入新数据
											
											for(uint8_t j = ang_rx_count;j>i+1;j--)
											{
												//旧数据向后移位
												angle_buf[j] = angle_buf[j-1];
											}
											angle_buf[i+1] = angle_temp;
											break;	//插入完成，退出循环
										}
									}							
								}
							
							}
							
							//计算和存储长度数据
							if(c1_len_measure_done_flag == 0)	//判断是否已测量完成
							{
								
								c1_length_raw -= 5.5f;	//静态误差
								
								c1_len_rx_count++;
								float length_temp = 0;		//临时变量
								
								for(uint8_t i = 0;i<c1_len_rx_count;i++)
								{
								//由大到小排列
								if(c1_length_raw > c1_length_raw_buff[i])
								{
									length_temp = c1_length_raw_buff[i];
									c1_length_raw_buff[i] = c1_length_raw;	//插入新数据
									
									for(uint8_t j = c1_len_rx_count;j>i+1;j--)
									{
										//旧数据向后移位
										c1_length_raw_buff[j] = c1_length_raw_buff[j-1];
									}
									c1_length_raw_buff[i+1] = length_temp;
									break;	//插入完成，退出循环
								}
							}
								LCD_ShowIntNum(0,0, c1_len_rx_count,2,GREEN,BLACK,16);	//显示接收计数
							}
							
							//角度测量完成
							if(ang_rx_count >= ANGLE_MEASURE_NUM)
							{
								
								angle = angle_buf[2];	//取中位数
								ang_measure_done_flag = 1;	//角度测量完成
								
								//LCD_ShowFloatNum1(30,100, angle,4,1,WHITE,BLACK,24);
								memset(angle_buf,0,ANGLE_MEASURE_NUM);
								if(angle < 5)
								{
									angle = 5;
								}
								else if(angle > 85)
								{
									angle = 85;
								}
								/* 测量0度和90度时，会有一侧的节点捕捉不到最大值，
								出现这种情况时，c1_len_rx_count 和 c2_len_rx_count会不等，且相差较大
								当二者差值大于规定值时，判断为5度或85度 */
								
								int8_t delta_cnt = c1_len_rx_count - c2_len_rx_count;		//错误次数
								if(delta_cnt >= 2)
								{
									angle = 85;		//校准为为5度（覆盖0-10度）
								}
								else if(delta_cnt <= -2)
								{
									angle = 5;		//校准为85度（覆盖80-90度）
								}
								
							}
							
							
							//长度测量完成
							if(c1_len_rx_count >= LEN_MEASURE_NUM)
							{
								
								//长度测量完成
								c1_length = (c1_length_raw_buff[4]+c1_length_raw_buff[5])/2.0f;	//取两个中位数的均值
								memset(c1_length_raw_buff,0,LEN_MEASURE_NUM);
	
								c1_len_measure_done_flag = 1;	//测量完成标志位置位
								//LCD_ShowFloatNum1(30,124, c1_length,4,1,WHITE,BLACK,24);
								
							}
						}
						return;
					}
					
					/**********************************LCD1图像***********************************/
					if(udp_rx_buf[0] == 0xA5 && udp_rx_buf[1] == 0xA5)
					{
						c1_sum_rec_len = 0;
					}
					memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
					c1_sum_rec_len += rec_len;
					if(c1_sum_rec_len >= 24000)
					{
						//切换lcd cs引脚
						CS_port = LCD_CS_GPIO_Port;
						CS_pin = LCD_CS_Pin;
						
						
						//显示图像
						LCD_ShowPicture(0,0,128,94,buff1);
						c1_sum_rec_len = 0;
					}
					
				}
				//Client2
				else if(remote_ip[3] == 55)
				{
					/***示波器数据***/
					if(udp_rx_buf[0] == 0xC5 && udp_rx_buf[1] == 0xC5)
					{
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3];	
						if(sum == udp_rx_buf[4])
						{
							memcpy(&x2_max,udp_rx_buf+2,2);	//接收x_max，此时x2_max表示实时坐标
						}
					}					
					/****************************LCD2测量数据*******************************************/
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						//切换lcd cs引脚
						CS_port = LCD2_CS_GPIO_Port;
						CS_pin = LCD2_CS_Pin;
						
						//接收到测量数据
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4]+
										udp_rx_buf[5]+udp_rx_buf[6]+udp_rx_buf[7];
						
						if(sum == udp_rx_buf[8])
						{
							//校验和正确
							memcpy(&c2_length_raw,udp_rx_buf+2,4);	//接收x节点测量的length
							memcpy(&x2_max,udp_rx_buf+6,2);	//接收x_max
							
							
							//计算和存储角度数据
							if(ang_measure_done_flag == 0)
							{
								c2_max_tick = HAL_GetTick();	//记录时间
								
								int32_t delta_tick = 0;		//时间差
								delta_tick = c1_max_tick -c2_max_tick;
								if(delta_tick <= 300 && delta_tick >= -300)	//判断时间差是否在范围内
								{
								angle_raw = atan2f(x1_max-x1_base,x2_max-x2_base)*57.3f;	//计算角度
								ang_rx_count++;
								
								LCD_ShowIntNum(100,0, ang_rx_count,2,GREEN,BLACK,16);	//显示角度接收计数
								float angle_temp = 0;
								for(uint8_t i = 0;i<ang_rx_count;i++)
								{
									//由大到小排列
									if(angle_raw > angle_buf[i])
									{
										angle_temp = angle_buf[i];
										angle_buf[i] = angle_raw;	//插入新数据
										
										for(uint8_t j = ang_rx_count;j>i+1;j--)
										{
											//旧数据向后移位
											angle_buf[j] = angle_buf[j-1];
										}
										angle_buf[i+1] = angle_temp;
										break;	//插入完成，退出循环
									}
								}							
							}
							
								
							}
							
							//计算和存储长度数据
							if(c2_len_measure_done_flag == 0)
							{
								c2_length_raw -= 5.5f;	//静态误差
								
								c2_len_rx_count++;
								float length_temp = 0;		//临时变量
								
								for(uint8_t i = 0;i<c2_len_rx_count;i++)
								{
									//由大到小排列
									if(c2_length_raw > c2_length_raw_buff[i])
									{
										length_temp = c2_length_raw_buff[i];
										c2_length_raw_buff[i] = c2_length_raw;	//插入新数据
										
										for(uint8_t j = c2_len_rx_count;j>i+1;j--)
										{
											//旧数据向后移位
											c2_length_raw_buff[j] = c2_length_raw_buff[j-1];
										}
										c2_length_raw_buff[i+1] = length_temp;
										break;	//插入完成，退出循环
									}
								}
								LCD_ShowIntNum(0,0, c2_len_rx_count,2,GREEN,BLACK,16);	//显示接收计数
							}
							
							//角度测量完成
							if(ang_rx_count >= ANGLE_MEASURE_NUM)
							{
								//角度测量完成
								angle = angle_buf[2];	//取中位数
								ang_measure_done_flag = 1;	//角度测量完成
								
								//LCD_ShowFloatNum1(30,100, angle,4,1,WHITE,BLACK,24);
								memset(angle_buf,0,ANGLE_MEASURE_NUM);
								
								
								/* 测量0度和90度时，会有一侧的节点捕捉不到最大值，
								出现这种情况时，c1_len_rx_count 和 c2_len_rx_count会不等，且相差较大
								当二者差值大于规定值时，判断为5度或85度 */
								
								int8_t delta_cnt = c1_len_rx_count - c2_len_rx_count;		//错误次数
								if(delta_cnt >= 2)
								{
									angle = 5;		//校准为为5度（覆盖0-10度）
								}
								else if(delta_cnt <= -2)
								{
									angle = 85;		//校准为85度（覆盖80-90度）
								}
							}
							
							
							//长度测量完成
							if(c2_len_rx_count >= LEN_MEASURE_NUM)
							{
								
								//长度测量完成
								c2_length = (c2_length_raw_buff[4]+c2_length_raw_buff[5])/2.0f;	//取两个中位数的均值
								memset(c2_length_raw_buff,0,LEN_MEASURE_NUM);		
								c2_len_measure_done_flag = 1;	//测量完成标志位置位
								
								//LCD_ShowFloatNum1(30,124, c2_length,4,1,WHITE,BLACK,24);//显示长度
								
							}

							
							

							
													
						}
						return;
					}
					
					if(anolog_mode == 0)	//进入模拟模式时，停止刷新图像
					{
						/**********************************LCD2图像***********************************/
						if(udp_rx_buf[0] == 0xA5 && udp_rx_buf[1] == 0xA5)
						{
							c2_sum_rec_len = 0;
						}
						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
						c2_sum_rec_len += rec_len;
						if(c2_sum_rec_len >= 24000)
						{
							//切换lcd cs引脚
							CS_port = LCD2_CS_GPIO_Port;
							CS_pin = LCD2_CS_Pin;

								//显示帧率
	//							static uint32_t n2 ,n2_last= 0;
	//							static uint32_t c2_frame_tick = 0;
	//							
	//							n2 ++;
	//							if(HAL_GetTick() - c2_frame_tick >= 1000)
	//							{
	//								c2_frame_tick = HAL_GetTick();
	//								LCD_ShowIntNum(0,0, n2 - n2_last,2,WHITE,BLACK,16);
	//								n2_last = n2;
	//							}
							
							//显示图像
							LCD_ShowPicture(0,0,128,94,buff2);
							c2_sum_rec_len = 0;
						}
					}
				}
		
			}
	}
}

