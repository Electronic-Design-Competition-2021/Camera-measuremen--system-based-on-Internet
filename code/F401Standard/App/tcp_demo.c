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
uint8_t udp_rx_buf[8000];			//udp单次接收缓存
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

uint16_t x_max = 0;
uint8_t length = 0;
uint16_t x1_max,x2_max;

void Rx_udp(uint8 * buff1,uint8 * buff2)
{
	uint16 len=0;
	uint32_t rec_len = 0;
	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;
	
	switch(getSn_SR(SOCK_UDPS1))                                                /*获取socket的状态*/
	{
		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
			socket(SOCK_UDPS1,Sn_MR_UDP,5000,0);         
			/*初始化socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket初始化完成*/
			//HAL_Delay(10);
			/*接收来自Client的数据*/
			udp_establish_flag = 1;
		
			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*清接收中断*/
			}
			
			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*接收到数据*/
			{	
				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//接收缓存
				
				if(remote_ip[3] == 44)
				{
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						memcpy(&x_max,udp_rx_buf+2,2);
						
						//接收到测量数据
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4];
						if(sum == udp_rx_buf[5])
						{
							//校验和正确
							memcpy(&x1_max,udp_rx_buf+3,2);
							length = udp_rx_buf[2];		
							
							//切换lcd cs引脚
//							CS_port = LCD_CS_GPIO_Port;
//							CS_pin = LCD_CS_Pin;
						
						}
					}
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

//							//显示帧率
//							static uint32_t n1 ,n1_last= 0;
//							static uint32_t c1_frame_tick = 0;
//							n1 ++;
//							if(HAL_GetTick() - c1_frame_tick >= 1000)
//							{
//								c1_frame_tick = HAL_GetTick();
//								LCD_ShowIntNum(0,0, n1 - n1_last,2,WHITE,BLACK,16);
//								n1_last = n1;
//							}
						
						//显示图像
						LCD_ShowPicture(0,0,128,94,buff1);
						c1_sum_rec_len = 0;
					}
				}
				if(remote_ip[3] == 55)
				{
					if(udp_rx_buf[0] == 0xB5 && udp_rx_buf[1] == 0xB5)
					{
						//接收到测量数据
						uint8_t sum = udp_rx_buf[2]+udp_rx_buf[3]+udp_rx_buf[4];
						if(sum == udp_rx_buf[5])
						{
							//校验和正确
							memcpy(&x2_max,udp_rx_buf+3,2);
							length = udp_rx_buf[2];		
							
							//切换lcd cs引脚
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
			break;
	}
}

//void Rx_udp(uint8 * buff1,uint8 * buff2)
//{   
//	uint16 len=0;
//	uint32_t rec_len = 0;
//	static uint32_t c1_sum_rec_len = 0,c2_sum_rec_len = 0;
//	
//	switch(getSn_SR(SOCK_UDPS1))                                                /*获取socket的状态*/
//	{
//		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
//			socket(SOCK_UDPS1,Sn_MR_UDP,5000,0);         
//			/*初始化socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket初始化完成*/
//			//HAL_Delay(10);
//			/*接收来自Client的数据*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*清接收中断*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*接收到数据*/
//			{	
//				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//接收缓存
//				
//				
//				
//				if(remote_ip[3] == 44)
//				{
//					if(c1_sum_rec_len == 0)
//					{
//						//新的一帧接收
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//定位帧头
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff1,udp_rx_buf+i,rec_len-i);	//从帧头存入图像矩阵
//								c1_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{
//						//继续这一帧的接收
//						memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
//						c1_sum_rec_len += rec_len;
//					}
//					if(c1_sum_rec_len >= 24000)
//					{
//						//切换lcd cs引脚
//						CS_port = LCD_CS_GPIO_Port;
//						CS_pin = LCD_CS_Pin;

//							//显示帧率
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
//						//显示图像
//						LCD_ShowPicture(0,30,128,96,buff1);
//						c1_sum_rec_len = 0;
//					}
//				}
//				else if(remote_ip[3] == 55)
//				{
//					if(c2_sum_rec_len == 0)
//					{
//						//新的一帧接收
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//定位帧头
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff2,udp_rx_buf+i,rec_len-i);	//从帧头存入图像矩阵
//								c2_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{		
//						//继续这一帧的接收
//						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
//						c2_sum_rec_len += rec_len;
//					}
//					if(c2_sum_rec_len >= 24000)
//					{
//						//切换lcd cs引脚
//						CS_port = LCD2_CS_GPIO_Port;
//						CS_pin = LCD2_CS_Pin;

//							//显示帧率
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
//						//显示图像
//						LCD_ShowPicture(0,30,128,96,buff2);
//						c2_sum_rec_len = 0;
//					}
//				}				
//			}
//			break;
//	}
//	
//	switch(getSn_SR(SOCK_UDPS2))                                                /*获取socket的状态*/
//	{
//		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
//			socket(SOCK_UDPS2,Sn_MR_UDP,6000,0);         
//			/*初始化socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket初始化完成*/
//			//HAL_Delay(10);
//			/*接收来自Client的数据*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*清接收中断*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*接收到数据*/
//			{	
//				rec_len = recvfrom(SOCK_UDPS2,udp_rx_buf,len, remote_ip,&remote_port);	//接收缓存
//				
//				
//				
//				if(remote_ip[3] == 44)
//				{
//					if(c1_sum_rec_len == 0)
//					{
//						//新的一帧接收
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//定位帧头
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff1,udp_rx_buf+i,rec_len-i);	//从帧头存入图像矩阵
//								c1_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{
//						//继续这一帧的接收
//						memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
//						c1_sum_rec_len += rec_len;
//					}
//					if(c1_sum_rec_len >= 24000)
//					{
//						//切换lcd cs引脚
//						CS_port = LCD_CS_GPIO_Port;
//						CS_pin = LCD_CS_Pin;

//							//显示帧率
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
//						//显示图像
//						LCD_ShowPicture(0,30,128,96,buff1);
//						c1_sum_rec_len = 0;
//					}
//				}
//				else if(remote_ip[3] == 55)
//				{
//					if(c2_sum_rec_len == 0)
//					{
//						//新的一帧接收
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//定位帧头
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff2,udp_rx_buf+i,rec_len-i);	//从帧头存入图像矩阵
//								c2_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{		
//						//继续这一帧的接收
//						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
//						c2_sum_rec_len += rec_len;
//					}
//					if(c2_sum_rec_len >= 24000)
//					{
//						//切换lcd cs引脚
//						CS_port = LCD2_CS_GPIO_Port;
//						CS_pin = LCD2_CS_Pin;

//							//显示帧率
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
//						//显示图像
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
//	switch(getSn_SR(SOCK_UDPS1))                                                /*获取socket的状态*/
//	{
//		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
//			socket(SOCK_UDPS1,Sn_MR_UDP,local_port,0);         
//			/*初始化socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket初始化完成*/
//			//HAL_Delay(10);
//			/*接收来自Client的数据*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS1) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS1, Sn_IR_RECV);                                     /*清接收中断*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS1))>0)                                    /*接收到数据*/
//			{	
//				rec_len = recvfrom(SOCK_UDPS1,udp_rx_buf,len, remote_ip,&remote_port);	//接收缓存
//				
//				
//				
//				if(remote_ip[3] == 44)
//				{
//					if(c1_sum_rec_len == 0)
//					{
//						//新的一帧接收
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//定位帧头
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff1,udp_rx_buf+i,rec_len-i);	//从帧头存入图像矩阵
//								c1_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{
//						//继续这一帧的接收
//						memcpy(buff1+c1_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
//						c1_sum_rec_len += rec_len;
//					}
//					if(c1_sum_rec_len >= 24000)
//					{
//						//切换lcd cs引脚
//						CS_port = LCD_CS_GPIO_Port;
//						CS_pin = LCD_CS_Pin;

//							//显示帧率
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
//						//显示图像
//						LCD_ShowPicture(0,30,128,96,buff1);
//						c1_sum_rec_len = 0;
//					}
//				}
//				else if(remote_ip[3] == 55)
//				{
//					if(c2_sum_rec_len == 0)
//					{
//						//新的一帧接收
//						uint8_t i = 0;
//						for(i = 0;i < rec_len;i++)
//						{
//							//定位帧头
//							if(udp_rx_buf[i] == 0xA5 && udp_rx_buf[i+1] == 0xA5)
//							{
//								memcpy(buff2,udp_rx_buf+i,rec_len-i);	//从帧头存入图像矩阵
//								c2_sum_rec_len = rec_len-i;
//								break;
//							}
//						}
//					}
//					else		
//					{		
//						//继续这一帧的接收
//						memcpy(buff2+c2_sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
//						c2_sum_rec_len += rec_len;
//					}
//					if(c2_sum_rec_len >= 24000)
//					{
//						//切换lcd cs引脚
//						CS_port = LCD2_CS_GPIO_Port;
//						CS_pin = LCD2_CS_Pin;

//							//显示帧率
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
//						//显示图像
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
//	switch(getSn_SR(SOCK_UDPS2))                                                /*获取socket的状态*/
//	{
//		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
//			socket(SOCK_UDPS2,Sn_MR_UDP,local_port1,0);         
//			/*初始化socket*/
//		  break;
//		
//		case SOCK_UDP:                                                           /*socket初始化完成*/
//			//HAL_Delay(10);
//			/*接收来自Client的数据*/
//			udp_establish_flag = 1;
//		
//			if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
//			{
//				setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*清接收中断*/
//			}
//			
//			if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*接收到数据*/
//			{	
//				
//				static uint32_t n = 0,m = 0;
//				rec_len = recvfrom(SOCK_UDPS2,udp_rx_buf,len, remote_ip,&remote_port);	//接收缓存
//				//rec_len = recvfrom(SOCK_UDPS2,buff+sum_rec_len,len, remote_ip,&remote_port);
//				if(udp_rx_buf[0] == 0xA5 && udp_rx_buf[1] == 0xA5)
//					sum_rec_len = 0;
//				memcpy(buff+sum_rec_len,udp_rx_buf,rec_len);	//累加存入图像矩阵
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
//						LCD_ShowIntNum(0,0, n - n_last,3,WHITE,BLUE,16);//显示帧率
//						n_last = n;
//					}
//					CS_port = LCD_CS_GPIO_Port;
//					CS_pin=LCD_CS_Pin;
//					LCD_ShowPicture(0,30,128,96,buff);
//					sum_rec_len = 0;
//					//LCD_ShowIntNum(0,0,n,6,WHITE,BLUE,16);//计数
//				}

//				
//			}	
//			break;
//	}

//}
//头部对应像素为三个“0x43 0xc6"  (每个像素16位)，
//void adj_frame(uint8_t *buf)
//{
//	uint32_t i = 0;
//	while(buf[i++] != 0x43)
//	{
//		buf[i++] = 
//	}
//}

