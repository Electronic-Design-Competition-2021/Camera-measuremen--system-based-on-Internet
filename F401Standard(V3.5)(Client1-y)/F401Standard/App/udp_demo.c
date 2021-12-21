#include <stdio.h>
#include <string.h>

#include "udp_demo.h"
#include "W5500_conf.h"
#include "w5500.h"
#include "socket.h"

#include "bsp_lcd.h"
#include "appMenu.h"
#include "openmv.h"
volatile uint8_t udp_establish_flag = 0;

void Tx_udp(uint8 * buff,uint32 len)
{     
	switch(getSn_SR(SOCK_UDPS))                                                /*获取socket的状态*/
	{
		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
			socket(SOCK_UDPS,Sn_MR_UDP,local_port,0);                              /*初始化socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket初始化完成*/
			udp_establish_flag = 1;
			sendto(SOCK_UDPS,buff,len, remote_ip, remote_port);//发送到服务端
			break;
	}
}

uint8_t udp_rx_buf[32];//服务端发来的指令

uint8  rx_remote_ip[4];											/*接收远端IP地址*/
uint16 rx_remote_port;

extern uint8_t measure_flag;
extern uint8_t udp_start_flag;	//图像发送开启标志位
extern uint8_t openmv_mode_flag;
uint8_t osc_flag = 0;
//默认端口号7000

void Rx_udp(void)
{
	
	uint16 len=0;
	switch(getSn_SR(SOCK_UDPS2))                                                /*获取socket的状态*/
	{
		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
			socket(SOCK_UDPS2,Sn_MR_UDP,7000,0);         
			/*初始化socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket初始化完成*/
			/*接收来自服务器的一键启动命令*/
			if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*清接收中断*/
			}
			
			if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*接收到数据*/
			{	
				len = len>32?32:len;
				recvfrom(SOCK_UDPS2,udp_rx_buf,len, rx_remote_ip,&rx_remote_port);	//接收缓存
				
				if(rx_remote_ip[3] == 66)
				{
					//收到来自服务器的命令
					if(udp_rx_buf[0] == 0x11)
					{
						measure_flag = 1;		//收到开始测量指令
						menuSkip(&GUI,menu_StartMeasure);	//跳转至menu_DebugVideo,开始传输图像
					}
					if(udp_rx_buf[0] == 0x12)
					{
						measure_flag = 0;		//结束测量
						menuReturn(&GUI);		//返回菜单
					}
					
					if(udp_rx_buf[0] == 0x13)
					{
						openmv_mode_flag = !openmv_mode_flag;
						if(openmv_mode_flag)
						{
							OPENMV_SetDispMode(OPENMV_MODE_BIN);//二值图像
						}
						else
						{
							OPENMV_SetDispMode(OPENMV_MODE_COL);//彩色图像
						}					
					}
					if(udp_rx_buf[0] == 0x14)
					{
						osc_flag = 1;		//开启示波器
					}
					if(udp_rx_buf[0] == 0x15)
					{
						osc_flag = 0;		//关闭示波器
					}
					
				}
			}
			break;
	}
}
