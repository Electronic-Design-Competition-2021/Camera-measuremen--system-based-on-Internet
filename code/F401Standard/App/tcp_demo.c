#include <stdio.h>
#include <string.h>
#include "tcp_demo.h"
#include "W5500_conf.h"
#include "w5500.h"
#include "socket.h"
#include "bsp_lcd_init.h"
#include "bsp_lcd.h"


volatile uint8_t udp_establish_flag = 0;
//uint8_t img_Buf1[13000],img_Buf2[13000];

uint8_t udp_Tx_buf[3];
uint8_t udp_rx_buf[8000];			//udp���ν��ջ���
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

uint16_t x_max = 0;
uint8_t length = 0;
uint16_t x1_max,x2_max;

void Rx_udp(uint8 * buff1,uint8 * buff2)
{
	uint16 len=0;
	uint32_t rec_len = 0;
	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;
	
	switch(getSn_SR(SOCK_UDPS1))                                                /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
			socket(SOCK_UDPS1,Sn_MR_UDP,5000,0);         
			/*��ʼ��socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket��ʼ�����*/
			//HAL_Delay(10);
			/*��������Client������*/
			udp_establish_flag = 1;
		
			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*������ж�*/
			}
			
			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*���յ�����*/
			{	
				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//���ջ���
				
				if(remote_ip[3] == 44)
				{
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						memcpy(&x_max,udp_rx_buf+2,2);
						
						//���յ���������
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4];
						if(sum == udp_rx_buf[5])
						{
							//У�����ȷ
							memcpy(&x1_max,udp_rx_buf+3,2);
							length = udp_rx_buf[2];		
							
							//�л�lcd cs����
//							CS_port = LCD_CS_GPIO_Port;
//							CS_pin = LCD_CS_Pin;
						
						}
					}
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

//							//��ʾ֡��
//							static uint32_t n1 ,n1_last= 0;
//							static uint32_t c1_frame_tick = 0;
//							n1 ++;
//							if(HAL_GetTick() - c1_frame_tick >= 1000)
//							{
//								c1_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n1 - n1_last,2,WHITE,BLACK,16);
//								n1_last = n1;
//							}
						
						//��ʾͼ��
						LCD_ShowPicture(0,0,128,94,buff1);
						c1_sum_rec_len = 0;
					}
				}
				if(remote_ip[3] == 55)
				{
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						//���յ���������
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4];
						if(sum == udp_rx_buf[5])
						{
							//У�����ȷ
							memcpy(&x2_max,udp_rx_buf+3,2);
							length = udp_rx_buf[2];		
							
							//�л�lcd cs����
//							CS_port = LCD2_CS_GPIO_Port;
//							CS_pin = LCD2_CS_Pin;
//							LCD_ShowIntNum(18,100, x2_max,3,WHITE,BLACK,12);
//							LCD_ShowIntNum(82,100, length,3,WHITE,BLACK,12);
						}
					}
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
			break;
	}
}

//void Rx_udp(uint8 * buff1,uint8 * buff2)
//{   
//	uint16 len=0;
//	uint32_t rec_len = 0;
//	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;
//	
//	switch(getSn_SR(SOCK_UDPS1))                                                /*��ȡsocket��״̬*/
//	{
//		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
//			socket(SOCK_UDPS1,Sn_MR_UDP,5000,0);         
//			/*��ʼ��socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket��ʼ�����*/
//			//HAL_Delay(10);
//			/*��������Client������*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*������ж�*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*���յ�����*/
//			{	
//				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//���ջ���
//				
//				
//				
//				if(remote_ip[3] == 44)
//				{
//					if(c1_sum_rec_len == 0)
//					{
//						//�µ�һ֡����
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//��λ֡ͷ
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff1,udp_rx_buf+i,rec_len-i);	//��֡ͷ����ͼ�����
//								c1_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{
//						//������һ֡�Ľ���
//						memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//						c1_sum_rec_len += rec_len;
//					}
//					if(c1_sum_rec_len >= 24000)
//					{
//						//�л�lcd cs����
//						CS_port = LCD_CS_GPIO_Port;
//						CS_pin = LCD_CS_Pin;

//							//��ʾ֡��
//							static uint32_t n1 ,n1_last= 0;
//							static uint32_t c1_frame_tick = 0;
//							n1 ++;
//							if(HAL_GetTick() - c1_frame_tick >= 1000)
//							{
//								c1_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n1 - n1_last,3,WHITE,BLUE,16);
//								n1_last = n1;
//							}
//						
//						//��ʾͼ��
//						LCD_ShowPicture(0,30,128,96,buff1);
//						c1_sum_rec_len = 0;
//					}
//				}
//				else if(remote_ip[3] == 55)
//				{
//					if(c2_sum_rec_len == 0)
//					{
//						//�µ�һ֡����
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//��λ֡ͷ
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff2,udp_rx_buf+i,rec_len-i);	//��֡ͷ����ͼ�����
//								c2_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{		
//						//������һ֡�Ľ���
//						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//						c2_sum_rec_len += rec_len;
//					}
//					if(c2_sum_rec_len >= 24000)
//					{
//						//�л�lcd cs����
//						CS_port = LCD2_CS_GPIO_Port;
//						CS_pin = LCD2_CS_Pin;

//							//��ʾ֡��
//							static uint32_t n2 ,n2_last= 0;
//							static uint32_t c2_frame_tick = 0;
//							
//							n2 ++;
//							if(HAL_GetTick() - c2_frame_tick >= 1000)
//							{
//								c2_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n2 - n2_last,3,WHITE,BLUE,16);
//								n2_last = n2;
//							}
//						
//						//��ʾͼ��
//						LCD_ShowPicture(0,30,128,96,buff2);
//						c2_sum_rec_len = 0;
//					}
//				}				
//			}
//			break;
//	}
//	
//	switch(getSn_SR(SOCK_UDPS2))                                                /*��ȡsocket��״̬*/
//	{
//		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
//			socket(SOCK_UDPS2,Sn_MR_UDP,6000,0);         
//			/*��ʼ��socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket��ʼ�����*/
//			//HAL_Delay(10);
//			/*��������Client������*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*������ж�*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*���յ�����*/
//			{	
//				rec_len = recvfrom(SOCK_UDPS2,udp_rx_buf,len, remote_ip,&remote_port);	//���ջ���
//				
//				
//				
//				if(remote_ip[3] == 44)
//				{
//					if(c1_sum_rec_len == 0)
//					{
//						//�µ�һ֡����
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//��λ֡ͷ
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff1,udp_rx_buf+i,rec_len-i);	//��֡ͷ����ͼ�����
//								c1_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{
//						//������һ֡�Ľ���
//						memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//						c1_sum_rec_len += rec_len;
//					}
//					if(c1_sum_rec_len >= 24000)
//					{
//						//�л�lcd cs����
//						CS_port = LCD_CS_GPIO_Port;
//						CS_pin = LCD_CS_Pin;

//							//��ʾ֡��
//							static uint32_t n1 ,n1_last= 0;
//							static uint32_t c1_frame_tick = 0;
//							n1 ++;
//							if(HAL_GetTick() - c1_frame_tick >= 1000)
//							{
//								c1_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n1 - n1_last,3,WHITE,BLUE,16);
//								n1_last = n1;
//							}
//						
//						//��ʾͼ��
//						LCD_ShowPicture(0,30,128,96,buff1);
//						c1_sum_rec_len = 0;
//					}
//				}
//				else if(remote_ip[3] == 55)
//				{
//					if(c2_sum_rec_len == 0)
//					{
//						//�µ�һ֡����
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//��λ֡ͷ
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff2,udp_rx_buf+i,rec_len-i);	//��֡ͷ����ͼ�����
//								c2_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{		
//						//������һ֡�Ľ���
//						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//						c2_sum_rec_len += rec_len;
//					}
//					if(c2_sum_rec_len >= 24000)
//					{
//						//�л�lcd cs����
//						CS_port = LCD2_CS_GPIO_Port;
//						CS_pin = LCD2_CS_Pin;

//							//��ʾ֡��
//							static uint32_t n2 ,n2_last= 0;
//							static uint32_t c2_frame_tick = 0;
//							
//							n2 ++;
//							if(HAL_GetTick() - c2_frame_tick >= 1000)
//							{
//								c2_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n2 - n2_last,3,WHITE,BLUE,16);
//								n2_last = n2;
//							}
//						
//						//��ʾͼ��
//						LCD_ShowPicture(0,30,128,96,buff2);
//						c2_sum_rec_len = 0;
//					}
//				}				
//			}
//			break;
//	}
//}


//void Rx2_udp(uint8 * buff1,uint8 * buff2)
//{   
//	uint16 len=0;
//	uint32_t rec_len = 0;
//	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;
//	
//	switch(getSn_SR(SOCK_UDPS1))                                                /*��ȡsocket��״̬*/
//	{
//		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
//			socket(SOCK_UDPS1,Sn_MR_UDP,local_port,0);         
//			/*��ʼ��socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket��ʼ�����*/
//			//HAL_Delay(10);
//			/*��������Client������*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*������ж�*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*���յ�����*/
//			{	
//				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//���ջ���
//				
//				
//				
//				if(remote_ip[3] == 44)
//				{
//					if(c1_sum_rec_len == 0)
//					{
//						//�µ�һ֡����
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//��λ֡ͷ
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff1,udp_rx_buf+i,rec_len-i);	//��֡ͷ����ͼ�����
//								c1_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{
//						//������һ֡�Ľ���
//						memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//						c1_sum_rec_len += rec_len;
//					}
//					if(c1_sum_rec_len >= 24000)
//					{
//						//�л�lcd cs����
//						CS_port = LCD_CS_GPIO_Port;
//						CS_pin = LCD_CS_Pin;

//							//��ʾ֡��
//							static uint32_t n1 ,n1_last= 0;
//							static uint32_t c1_frame_tick = 0;
//							n1 ++;
//							if(HAL_GetTick() - c1_frame_tick >= 1000)
//							{
//								c1_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n1 - n1_last,3,WHITE,BLUE,16);
//								n1_last = n1;
//							}
//						
//						//��ʾͼ��
//						LCD_ShowPicture(0,30,128,96,buff1);
//						c1_sum_rec_len = 0;
//					}
//				}
//				else if(remote_ip[3] == 55)
//				{
//					if(c2_sum_rec_len == 0)
//					{
//						//�µ�һ֡����
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//��λ֡ͷ
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff2,udp_rx_buf+i,rec_len-i);	//��֡ͷ����ͼ�����
//								c2_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{		
//						//������һ֡�Ľ���
//						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//						c2_sum_rec_len += rec_len;
//					}
//					if(c2_sum_rec_len >= 24000)
//					{
//						//�л�lcd cs����
//						CS_port = LCD2_CS_GPIO_Port;
//						CS_pin = LCD2_CS_Pin;

//							//��ʾ֡��
//							static uint32_t n2 ,n2_last= 0;
//							static uint32_t c2_frame_tick = 0;
//							
//							n2 ++;
//							if(HAL_GetTick() - c2_frame_tick >= 1000)
//							{
//								c2_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n2 - n2_last,3,WHITE,BLUE,16);
//								n2_last = n2;
//							}
//						
//						//��ʾͼ��
//						LCD_ShowPicture(0,30,128,96,buff2);
//						c2_sum_rec_len = 0;
//					}
//				}				
//			}
//			break;
//	}
//}


//void Rx_udp2(uint8 * buff)
//{   
//	uint16 i;
////	uint8 Buff[2048];  	
//	uint16 len=0;
//	static uint32_t rec_len = 0;
//	static uint32_t sum_rec_len = 0;
//	
//	switch(getSn_SR(SOCK_UDPS2))                                                /*��ȡsocket��״̬*/
//	{
//		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
//			socket(SOCK_UDPS2,Sn_MR_UDP,local_port1,0);         
//			/*��ʼ��socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket��ʼ�����*/
//			//HAL_Delay(10);
//			/*��������Client������*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*������ж�*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*���յ�����*/
//			{	
//				
//				static uint32_t n = 0,m = 0;
//				rec_len = recvfrom(SOCK_UDPS2,udp_rx_buf,len, remote_ip,&remote_port);	//���ջ���
//				//rec_len = recvfrom(SOCK_UDPS2,buff+sum_rec_len,len, remote_ip,&remote_port);
//				if(udp_rx_buf[0] == 0xA5 && udp_rx_buf[1] == 0xA5)
//					sum_rec_len = 0;
//				memcpy(buff+sum_rec_len,udp_rx_buf,rec_len);	//�ۼӴ���ͼ�����
//				sum_rec_len += rec_len;
//				
////				LCD_ShowIntNum(0,80+20*((m++)%3),rec_len,4,WHITE,BLUE,16);
////				LCD_ShowIntNum(50,80+20*(m%3),n,3,WHITE,BLUE,16);

//				if(sum_rec_len >= 24000)
//				{
//					n++;
//					if(HAL_GetTick() - frame_tick >= 1000)
//					{
//						frame_tick = HAL_GetTick();
//						LCD_ShowIntNum(0,0, n - n_last,3,WHITE,BLUE,16);//��ʾ֡��
//						n_last = n;
//					}
//					CS_port = LCD_CS_GPIO_Port;
//					CS_pin=LCD_CS_Pin;
//					LCD_ShowPicture(0,30,128,96,buff);
//					sum_rec_len = 0;
//					//LCD_ShowIntNum(0,0,n,6,WHITE,BLUE,16);//����
//				}

//				
//			}	
//			break;
//	}

//}
//ͷ����Ӧ����Ϊ������0x43 0xc6"  (ÿ������16λ)��
//void adj_frame(uint8_t *buf)
//{
//	uint32_t i = 0;
//	while(buf[i++] != 0x43)
//	{
//		buf[i++] = 
//	}
//}

