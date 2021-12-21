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
/***********************�û�������**************************/

//150cm��Ӧ����Ϊ2.5s��10��Ϊ25s����ĿҪ�����ʱ��С��30S
#define LEN_MEASURE_NUM  10
#define ANGLE_MEASURE_NUM  5

#define x1_base  100
#define x2_base  160


float angle_buf[ANGLE_MEASURE_NUM];	//ԭʼ�Ƕ�����
float angle = 0,angle_raw = 0;		//�����ĽǶ�����,ԭʼ�Ƕ�����
uint8_t ang_rx_count = 0;	//���ݽ��ռ���

volatile uint8_t udp_establish_flag = 0;

uint16_t x1_max = 0,x2_max = 0;//ץȡ����ߵ�λ������

float c1_length_raw = 0,c2_length_raw = 0;//���߳���ԭʼ����
float c1_length = 0,c2_length = 0;	//������length����
	

uint32_t c1_max_tick=0,c2_max_tick=0;//��ȡ�������ֵ��ʱ��

uint8_t udp_Tx_buf[3];         //��ͻ��˷���ָ�������
uint8_t udp_rx_buf[8000];			//udp���ν��ջ���

float c1_length_raw_buff[LEN_MEASURE_NUM],c2_length_raw_buff[LEN_MEASURE_NUM];	//����ԭʼ����

uint8_t c1_len_rx_count = 0,c2_len_rx_count = 0;	//���ݽ��ռ���

uint8_t ang_measure_done_flag = 0;	//angle������ɱ�־λ
uint8_t c1_len_measure_done_flag = 0;	//c1_len������ɱ�־
uint8_t c2_len_measure_done_flag = 0;	//c2_len������ɱ�־

extern uint8_t anolog_mode;		//ģ��ģʽ��־λ




/*******************�ͻ���IP��ַ**************************/
uint8  c1_ip[4]={192,168,112,44};
uint8  c2_ip[4]={192,168,112,55};

//ͬʱ�����ڵ㷢��ָ��˿ں�7000
void Tx_udp(uint8 * buff,uint32 len)
{
	switch(getSn_SR(SOCK_UDPS2))                                                /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
			socket(SOCK_UDPS2,Sn_MR_UDP,7000,0);                              /*��ʼ��socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket��ʼ�����*/
			sendto(SOCK_UDPS2,buff,len, c1_ip, 7000);
			sendto(SOCK_UDPS2,buff,len, c2_ip, 7000);	
		  break;
	}
}

//�������ڵ㷢��������
void Rx_udp(uint8 * buff1,uint8 * buff2)
{
	uint16 len=0;
	uint32_t rec_len = 0;
	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;//������ͼ�������ۼ�
	
	switch(getSn_SR(SOCK_UDPS1))                                                /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
			socket(SOCK_UDPS1,Sn_MR_UDP,5000,0);         
			/*��ʼ��socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket��ʼ�����*/
			/*��������Client������*/
			udp_establish_flag = 1;
		
			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*������ж�*/
			}
			
			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*���յ�����*/
			{	
				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//���ջ���
				//Client1
				if(remote_ip[3] == 44)
				{
					/***ʾ��������***/
					if(udp_rx_buf[0] == 0xC5 && udp_rx_buf[1] == 0xC5)
					{
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3];	
						if(sum == udp_rx_buf[4])
						{
							memcpy(&x1_max,udp_rx_buf+2,2);	//����x_max����ʱx1_max��ʾʵʱ����
						}
						return;
					}
					/****************************LCD1��������*******************************************/
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						//�л�lcd cs����
						CS_port = LCD_CS_GPIO_Port;
						CS_pin = LCD_CS_Pin;	

						
						//���յ���������
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4]+
										udp_rx_buf[5]+udp_rx_buf[6]+udp_rx_buf[7];
						

						if(sum == udp_rx_buf[8])
						{
							//У�����ȷ
							memcpy(&c1_length_raw,udp_rx_buf+2,4);	//����x�ڵ������length
							memcpy(&x1_max,udp_rx_buf+6,2);	//����x_max
							
							//����ʹ洢�Ƕ�����
							if(ang_measure_done_flag == 0)	//�ж��Ƿ��Ѳ������
							{
								
								c1_max_tick = HAL_GetTick();	//��¼ʱ��
								
								int32_t delta_tick = 0;		//ʱ���
								delta_tick = c1_max_tick -c2_max_tick;
								if(delta_tick <= 300 && delta_tick >= -300)	//�ж�ʱ����Ƿ��ڷ�Χ��
								{

									angle_raw = atan2f(x1_max-x1_base,x2_max-x2_base)*57.3f;	//����Ƕ�
									ang_rx_count++;
								
									LCD_ShowIntNum(100,0, ang_rx_count,2,GREEN,BLACK,16);	//��ʾ�ǶȽ��ռ��������Ͻǣ�
								
									float angle_temp = 0;
									for(uint8_t i = 0;i<ang_rx_count;i++)
									{
										//�ɴ�С����
										if(angle_raw > angle_buf[i])
										{
											angle_temp = angle_buf[i];
											angle_buf[i] = angle_raw;	//����������
											
											for(uint8_t j = ang_rx_count;j>i+1;j--)
											{
												//�����������λ
												angle_buf[j] = angle_buf[j-1];
											}
											angle_buf[i+1] = angle_temp;
											break;	//������ɣ��˳�ѭ��
										}
									}							
								}
							
							}
							
							//����ʹ洢��������
							if(c1_len_measure_done_flag == 0)	//�ж��Ƿ��Ѳ������
							{
								
								c1_length_raw -= 5.5f;	//��̬���
								
								c1_len_rx_count++;
								float length_temp = 0;		//��ʱ����
								
								for(uint8_t i = 0;i<c1_len_rx_count;i++)
								{
								//�ɴ�С����
								if(c1_length_raw > c1_length_raw_buff[i])
								{
									length_temp = c1_length_raw_buff[i];
									c1_length_raw_buff[i] = c1_length_raw;	//����������
									
									for(uint8_t j = c1_len_rx_count;j>i+1;j--)
									{
										//�����������λ
										c1_length_raw_buff[j] = c1_length_raw_buff[j-1];
									}
									c1_length_raw_buff[i+1] = length_temp;
									break;	//������ɣ��˳�ѭ��
								}
							}
								LCD_ShowIntNum(0,0, c1_len_rx_count,2,GREEN,BLACK,16);	//��ʾ���ռ���
							}
							
							//�ǶȲ������
							if(ang_rx_count >= ANGLE_MEASURE_NUM)
							{
								
								angle = angle_buf[2];	//ȡ��λ��
								ang_measure_done_flag = 1;	//�ǶȲ������
								
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
								/* ����0�Ⱥ�90��ʱ������һ��Ľڵ㲶׽�������ֵ��
								�����������ʱ��c1_len_rx_count �� c2_len_rx_count�᲻�ȣ������ϴ�
								�����߲�ֵ���ڹ涨ֵʱ���ж�Ϊ5�Ȼ�85�� */
								
								int8_t delta_cnt = c1_len_rx_count - c2_len_rx_count;		//�������
								if(delta_cnt >= 2)
								{
									angle = 85;		//У׼ΪΪ5�ȣ�����0-10�ȣ�
								}
								else if(delta_cnt <= -2)
								{
									angle = 5;		//У׼Ϊ85�ȣ�����80-90�ȣ�
								}
								
							}
							
							
							//���Ȳ������
							if(c1_len_rx_count >= LEN_MEASURE_NUM)
							{
								
								//���Ȳ������
								c1_length = (c1_length_raw_buff[4]+c1_length_raw_buff[5])/2.0f;	//ȡ������λ���ľ�ֵ
								memset(c1_length_raw_buff,0,LEN_MEASURE_NUM);
	
								c1_len_measure_done_flag = 1;	//������ɱ�־λ��λ
								//LCD_ShowFloatNum1(30,124, c1_length,4,1,WHITE,BLACK,24);
								
							}
						}
						return;
					}
					
					/**********************************LCD1ͼ��***********************************/
					if(udp_rx_buf[0] == 0xA5 && udp_rx_buf[1] == 0xA5)
					{
						c1_sum_rec_len = 0;
					}
					memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
					c1_sum_rec_len += rec_len;
					if(c1_sum_rec_len >= 24000)
					{
						//�л�lcd cs����
						CS_port = LCD_CS_GPIO_Port;
						CS_pin = LCD_CS_Pin;
						
						
						//��ʾͼ��
						LCD_ShowPicture(0,0,128,94,buff1);
						c1_sum_rec_len = 0;
					}
					
				}
				//Client2
				else if(remote_ip[3] == 55)
				{
					/***ʾ��������***/
					if(udp_rx_buf[0] == 0xC5 && udp_rx_buf[1] == 0xC5)
					{
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3];	
						if(sum == udp_rx_buf[4])
						{
							memcpy(&x2_max,udp_rx_buf+2,2);	//����x_max����ʱx2_max��ʾʵʱ����
						}
					}					
					/****************************LCD2��������*******************************************/
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						//�л�lcd cs����
						CS_port = LCD2_CS_GPIO_Port;
						CS_pin = LCD2_CS_Pin;
						
						//���յ���������
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4]+
										udp_rx_buf[5]+udp_rx_buf[6]+udp_rx_buf[7];
						
						if(sum == udp_rx_buf[8])
						{
							//У�����ȷ
							memcpy(&c2_length_raw,udp_rx_buf+2,4);	//����x�ڵ������length
							memcpy(&x2_max,udp_rx_buf+6,2);	//����x_max
							
							
							//����ʹ洢�Ƕ�����
							if(ang_measure_done_flag == 0)
							{
								c2_max_tick = HAL_GetTick();	//��¼ʱ��
								
								int32_t delta_tick = 0;		//ʱ���
								delta_tick = c1_max_tick -c2_max_tick;
								if(delta_tick <= 300 && delta_tick >= -300)	//�ж�ʱ����Ƿ��ڷ�Χ��
								{
								angle_raw = atan2f(x1_max-x1_base,x2_max-x2_base)*57.3f;	//����Ƕ�
								ang_rx_count++;
								
								LCD_ShowIntNum(100,0, ang_rx_count,2,GREEN,BLACK,16);	//��ʾ�ǶȽ��ռ���
								float angle_temp = 0;
								for(uint8_t i = 0;i<ang_rx_count;i++)
								{
									//�ɴ�С����
									if(angle_raw > angle_buf[i])
									{
										angle_temp = angle_buf[i];
										angle_buf[i] = angle_raw;	//����������
										
										for(uint8_t j = ang_rx_count;j>i+1;j--)
										{
											//�����������λ
											angle_buf[j] = angle_buf[j-1];
										}
										angle_buf[i+1] = angle_temp;
										break;	//������ɣ��˳�ѭ��
									}
								}							
							}
							
								
							}
							
							//����ʹ洢��������
							if(c2_len_measure_done_flag == 0)
							{
								c2_length_raw -= 5.5f;	//��̬���
								
								c2_len_rx_count++;
								float length_temp = 0;		//��ʱ����
								
								for(uint8_t i = 0;i<c2_len_rx_count;i++)
								{
									//�ɴ�С����
									if(c2_length_raw > c2_length_raw_buff[i])
									{
										length_temp = c2_length_raw_buff[i];
										c2_length_raw_buff[i] = c2_length_raw;	//����������
										
										for(uint8_t j = c2_len_rx_count;j>i+1;j--)
										{
											//�����������λ
											c2_length_raw_buff[j] = c2_length_raw_buff[j-1];
										}
										c2_length_raw_buff[i+1] = length_temp;
										break;	//������ɣ��˳�ѭ��
									}
								}
								LCD_ShowIntNum(0,0, c2_len_rx_count,2,GREEN,BLACK,16);	//��ʾ���ռ���
							}
							
							//�ǶȲ������
							if(ang_rx_count >= ANGLE_MEASURE_NUM)
							{
								//�ǶȲ������
								angle = angle_buf[2];	//ȡ��λ��
								ang_measure_done_flag = 1;	//�ǶȲ������
								
								//LCD_ShowFloatNum1(30,100, angle,4,1,WHITE,BLACK,24);
								memset(angle_buf,0,ANGLE_MEASURE_NUM);
								
								
								/* ����0�Ⱥ�90��ʱ������һ��Ľڵ㲶׽�������ֵ��
								�����������ʱ��c1_len_rx_count �� c2_len_rx_count�᲻�ȣ������ϴ�
								�����߲�ֵ���ڹ涨ֵʱ���ж�Ϊ5�Ȼ�85�� */
								
								int8_t delta_cnt = c1_len_rx_count - c2_len_rx_count;		//�������
								if(delta_cnt >= 2)
								{
									angle = 5;		//У׼ΪΪ5�ȣ�����0-10�ȣ�
								}
								else if(delta_cnt <= -2)
								{
									angle = 85;		//У׼Ϊ85�ȣ�����80-90�ȣ�
								}
							}
							
							
							//���Ȳ������
							if(c2_len_rx_count >= LEN_MEASURE_NUM)
							{
								
								//���Ȳ������
								c2_length = (c2_length_raw_buff[4]+c2_length_raw_buff[5])/2.0f;	//ȡ������λ���ľ�ֵ
								memset(c2_length_raw_buff,0,LEN_MEASURE_NUM);		
								c2_len_measure_done_flag = 1;	//������ɱ�־λ��λ
								
								//LCD_ShowFloatNum1(30,124, c2_length,4,1,WHITE,BLACK,24);//��ʾ����
								
							}

							
							

							
													
						}
						return;
					}
					
					if(anolog_mode == 0)	//����ģ��ģʽʱ��ֹͣˢ��ͼ��
					{
						/**********************************LCD2ͼ��***********************************/
						if(udp_rx_buf[0] == 0xA5 && udp_rx_buf[1] == 0xA5)
						{
							c2_sum_rec_len = 0;
						}
						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
						c2_sum_rec_len += rec_len;
						if(c2_sum_rec_len >= 24000)
						{
							//�л�lcd cs����
							CS_port = LCD2_CS_GPIO_Port;
							CS_pin = LCD2_CS_Pin;

								//��ʾ֡��
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
							
							//��ʾͼ��
							LCD_ShowPicture(0,0,128,94,buff2);
							c2_sum_rec_len = 0;
						}
					}
				}
		
			}
	}
}

