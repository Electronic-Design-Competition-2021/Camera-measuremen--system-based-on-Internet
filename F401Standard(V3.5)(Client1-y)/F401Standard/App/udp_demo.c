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
	switch(getSn_SR(SOCK_UDPS))                                                /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
			socket(SOCK_UDPS,Sn_MR_UDP,local_port,0);                              /*��ʼ��socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket��ʼ�����*/
			udp_establish_flag = 1;
			sendto(SOCK_UDPS,buff,len, remote_ip, remote_port);//���͵������
			break;
	}
}

uint8_t udp_rx_buf[32];//����˷�����ָ��

uint8  rx_remote_ip[4];											/*����Զ��IP��ַ*/
uint16 rx_remote_port;

extern uint8_t measure_flag;
extern uint8_t udp_start_flag;	//ͼ���Ϳ�����־λ
extern uint8_t openmv_mode_flag;
uint8_t osc_flag = 0;
//Ĭ�϶˿ں�7000

void Rx_udp(void)
{
	
	uint16 len=0;
	switch(getSn_SR(SOCK_UDPS2))                                                /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
			socket(SOCK_UDPS2,Sn_MR_UDP,7000,0);         
			/*��ʼ��socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket��ʼ�����*/
			/*�������Է�������һ����������*/
			if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*������ж�*/
			}
			
			if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*���յ�����*/
			{	
				len = len>32?32:len;
				recvfrom(SOCK_UDPS2,udp_rx_buf,len, rx_remote_ip,&rx_remote_port);	//���ջ���
				
				if(rx_remote_ip[3] == 66)
				{
					//�յ����Է�����������
					if(udp_rx_buf[0] == 0x11)
					{
						measure_flag = 1;		//�յ���ʼ����ָ��
						menuSkip(&GUI,menu_StartMeasure);	//��ת��menu_DebugVideo,��ʼ����ͼ��
					}
					if(udp_rx_buf[0] == 0x12)
					{
						measure_flag = 0;		//��������
						menuReturn(&GUI);		//���ز˵�
					}
					
					if(udp_rx_buf[0] == 0x13)
					{
						openmv_mode_flag = !openmv_mode_flag;
						if(openmv_mode_flag)
						{
							OPENMV_SetDispMode(OPENMV_MODE_BIN);//��ֵͼ��
						}
						else
						{
							OPENMV_SetDispMode(OPENMV_MODE_COL);//��ɫͼ��
						}					
					}
					if(udp_rx_buf[0] == 0x14)
					{
						osc_flag = 1;		//����ʾ����
					}
					if(udp_rx_buf[0] == 0x15)
					{
						osc_flag = 0;		//�ر�ʾ����
					}
					
				}
			}
			break;
	}
}
