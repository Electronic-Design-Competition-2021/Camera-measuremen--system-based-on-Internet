#include "openmv.h"
#include "usart.h"
#include "string.h"
#include "bsp_lcd.h"
#include "fifo.h"
#include "bsp_usart.h"
#include "bluetooth.h"
#include "fifo.h"
#include "math.h"
#include "angle_length.h"

unsigned char RXCNT=0;
//int16_t USART_RX_STA=0;       //����״̬���
unsigned char RXOVER;

//uint8_t i = 0;
uint8_t OpenMV_RX_Buf[8];//������
uint8_t OpenMV_RX_Cksum;//У���
uint16_t blob_x = 0;//ɫ������x����,�ǵøĳɽṹ��
uint8_t	 blob_y = 0;//ɫ������y����
//_Bool OpenMV_RX_flag;

float Angle;
float Lengh=0;
uint8_t y;//������y��ĳ���
uint16_t x;//������x��ĳ���


uint8_t OpenMV_tx_buf[OpenMV_TX_LENGTH];
uint8_t OpenMVProcBuf[OpenMV_RX_LENGTH];
//OpenMVDataType OpenMVData;

//void openmv_DataUnPack(uint8_t *OpenMV_RX_Buf)//���ں�OpenMVͨ��
//{
//	uint8_t Length = OpenMV_RX_Buf[1];//��ó���
//	if(OpenMV_RX_Buf[0] == 0XA5 && OpenMV_RX_Buf[Length-1] == 0X5A)//�ж�֡ͷ֡β
//	{
//		for(uint8_t i = 1;i < Length - 2;i++)
//		{
//			OpenMV_RX_Cksum += OpenMV_RX_Buf[i];
//		}
//		if(OpenMV_RX_Cksum == OpenMV_RX_Buf[Length-2])
//		{
//			switch (OpenMV_RX_Buf[2])
//				{
//				  case 0X24:
//					memcpy(&blob_x,&OpenMV_RX_Buf[0]+3,2);
//					blob_y = OpenMV_RX_Buf[5];
//				}
//		}
//	}
//}
//		
uint8_t openmv_tx_buf[64];

void OpenMV_SendLabThr(uint8_t dat[6])//��openmv,����6�ֽ�����(��ֵ)
{
	uint8_t i = 0;
	memset(openmv_tx_buf,0,11); //����Ϊ11
	openmv_tx_buf[0] = 0xA5;
	openmv_tx_buf[1] = 0x0B;
	openmv_tx_buf[2] = 0x21;
	for( i = 0;i < 2; i ++ )    //��������
	{
		openmv_tx_buf[i+3] = dat[i];
	}
	for( i = 2;i < 6; i ++ )    //��������
	{
		openmv_tx_buf[i+3] = dat[i]+128;
	}
	for(i = 1;i<9;i++)  //����У���
	{
		openmv_tx_buf[9] += openmv_tx_buf[i];
	}
	
	openmv_tx_buf[10] = 0x5A;
	usart2_SendData(openmv_tx_buf,11);	//�ĳ����⴮�ڼ� ��
}   // Modified by kontornl, current coding style


void OPENMV_SetDispMode(uint8_t send_type)	//����OpenMV��ʾΪ��ֵ��/ɫ�ʻ���
{
	uint8_t i = 0;
	
	memset(openmv_tx_buf,0,6); //����Ϊ6
	openmv_tx_buf[0] = 0xA5;
	openmv_tx_buf[1] = 0x06;
	openmv_tx_buf[2] = 0x01;
	openmv_tx_buf[3] = send_type;
	for(i = 1;i<4;i++)  //����У���
	{
		openmv_tx_buf[4] += openmv_tx_buf[i];
	}
	openmv_tx_buf[5] = 0x5A;
	usart2_SendData(openmv_tx_buf,6);	//�ĳ����⴮�ڼ���
}   // Modified by kontornl, current coding style

void usart2_SendData(uint8_t *data, uint32_t length)
{
	HAL_UART_Transmit(&huart2 , data, length, 0xff);
}

uint16_t blob_x_pos = 0;
uint16_t blob_y_pos = 0;

uint16_t blob_x_buf[3];			//������ʷ���ݣ����ڲ������xֵ

//������angle_length.c��
extern uint16_t x_max_temp;
extern uint32_t x_max_tick;		//��׽���ֵ��ʱ��
extern uint8_t x_max_capture_flag;		//��׽���ֵ��ʱ��

void OpenMV_DataUnPack(uint8_t *data)
{
	blob_x_buf[2] = blob_x_buf[1];
	blob_x_buf[1] = blob_x_buf[0];
	
	
	memcpy(&blob_x_pos,data+2,2);	//x����λ�ã���2��3�ֽڣ����㿪ʼ����
	memcpy(&blob_y_pos,data+4,1);	//y����λ�ã���4�ֽڣ����㿪ʼ��
	
	if(blob_x_pos > 320)
		blob_x_pos = 0;
	if(blob_y_pos > 240)
		blob_y_pos = 0;
	
	blob_x_buf[0] = 320-blob_x_pos;

	if(x_max_capture_flag == 1)
	{
		if(blob_x_buf[0] > x_max_temp)
		{
			x_max_temp = blob_x_buf[0];				//�������ֵ
			x_max_tick = HAL_GetTick();		//��ȡ��ǰϵͳʱ��
		}
	}
}


void OpenMV_DataProcess(fifo_s_t * p_fifo)
{
	static uint8_t proc_step = 0;
	uint8_t head = 0,end = 0;
	uint8_t i = 0,sum = 0,adj_sum = 0;
	 proc_step = 0;
	if(fifo_s_used(p_fifo))
	{
		if(proc_step == 0)	
		{
			//step1����λ֡ͷ
			while(head != 0xA5)
			{
				if(fifo_s_gets(p_fifo,(char*)&head,1) == -1)	//����֡ͷ
					return;	//��ȫ��ȡ��,δ����֡ͷ��ֱ�ӷ���
			}
			proc_step = 1;	//�ҵ�֡ͷ������step2
		}
		if(proc_step == 1)
		{
			//step2����������
			if(fifo_s_used(p_fifo) >= (OpenMV_RX_LENGTH - 1))	//�ж�fifo�е����ݳ���
			{
				fifo_s_gets(p_fifo,(char*)&OpenMVProcBuf,OpenMV_RX_LENGTH - 3);	//��������
				proc_step = 2;	//����������ɣ�����step3
			}
			else
				return;	//���ݻ�δ�����꣬�˳�
		}
		if(proc_step == 2)
		{
			//step3������У��� & �ж�֡β
			if(fifo_s_used(p_fifo) >= 2)
			{
				fifo_s_gets(p_fifo,(char*)&adj_sum,1);	//����У���
				i = OpenMV_RX_LENGTH - 3;	// 2021.7.7ע�ͣ����ģ�
				while(i -- )
					sum += OpenMVProcBuf[i];	//����У���
				fifo_s_gets(p_fifo,(char*)&end,1);	//����֡β
				if(sum == adj_sum && end == 0x5A)
					proc_step = 3;	//У���&֡β��ȷ������step4
				else
				{
					proc_step = 0;	//���󣬷���
					return;
				}
			}
			else
				return;
		}
		if(proc_step == 3)
		{
			proc_step = 0;	//������ɣ����½���
//			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED��˸
			OpenMV_DataUnPack(OpenMVProcBuf);//��������
//				bolb_max();
		}
	}
}


