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
//int16_t USART_RX_STA=0;       //接收状态标记
unsigned char RXOVER;

//uint8_t i = 0;
uint8_t OpenMV_RX_Buf[8];//缓冲区
uint8_t OpenMV_RX_Cksum;//校验和
uint16_t blob_x = 0;//色块中心x坐标,记得改成结构体
uint8_t	 blob_y = 0;//色块中心y坐标
//_Bool OpenMV_RX_flag;

float Angle;
float Lengh=0;
uint8_t y;//解算在y轴的长度
uint16_t x;//解算在x轴的长度


uint8_t OpenMV_tx_buf[OpenMV_TX_LENGTH];
uint8_t OpenMVProcBuf[OpenMV_RX_LENGTH];
//OpenMVDataType OpenMVData;

//void openmv_DataUnPack(uint8_t *OpenMV_RX_Buf)//用于和OpenMV通信
//{
//	uint8_t Length = OpenMV_RX_Buf[1];//获得长度
//	if(OpenMV_RX_Buf[0] == 0XA5 && OpenMV_RX_Buf[Length-1] == 0X5A)//判断帧头帧尾
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

void OpenMV_SendLabThr(uint8_t dat[6])//给openmv,发送6字节数据(阈值)
{
	uint8_t i = 0;
	memset(openmv_tx_buf,0,11); //长度为11
	openmv_tx_buf[0] = 0xA5;
	openmv_tx_buf[1] = 0x0B;
	openmv_tx_buf[2] = 0x21;
	for( i = 0;i < 2; i ++ )    //送入数据
	{
		openmv_tx_buf[i+3] = dat[i];
	}
	for( i = 2;i < 6; i ++ )    //送入数据
	{
		openmv_tx_buf[i+3] = dat[i]+128;
	}
	for(i = 1;i<9;i++)  //计算校验和
	{
		openmv_tx_buf[9] += openmv_tx_buf[i];
	}
	
	openmv_tx_buf[10] = 0x5A;
	usart2_SendData(openmv_tx_buf,11);	//改成任意串口即 可
}   // Modified by kontornl, current coding style


void OPENMV_SetDispMode(uint8_t send_type)	//调节OpenMV显示为二值化/色彩画面
{
	uint8_t i = 0;
	
	memset(openmv_tx_buf,0,6); //长度为6
	openmv_tx_buf[0] = 0xA5;
	openmv_tx_buf[1] = 0x06;
	openmv_tx_buf[2] = 0x01;
	openmv_tx_buf[3] = send_type;
	for(i = 1;i<4;i++)  //计算校验和
	{
		openmv_tx_buf[4] += openmv_tx_buf[i];
	}
	openmv_tx_buf[5] = 0x5A;
	usart2_SendData(openmv_tx_buf,6);	//改成任意串口即可
}   // Modified by kontornl, current coding style

void usart2_SendData(uint8_t *data, uint32_t length)
{
	HAL_UART_Transmit(&huart2 , data, length, 0xff);
}

uint16_t blob_x_pos = 0;
uint16_t blob_y_pos = 0;

uint16_t blob_x_buf[3];			//缓存历史数据，用于查找最大x值

//定义在angle_length.c中
extern uint16_t x_max_temp;
extern uint32_t x_max_tick;		//捕捉最大值的时间
extern uint8_t x_max_capture_flag;		//捕捉最大值的时间

void OpenMV_DataUnPack(uint8_t *data)
{
	blob_x_buf[2] = blob_x_buf[1];
	blob_x_buf[1] = blob_x_buf[0];
	
	
	memcpy(&blob_x_pos,data+2,2);	//x坐标位置：第2、3字节（从零开始数）
	memcpy(&blob_y_pos,data+4,1);	//y坐标位置：第4字节（从零开始）
	
	if(blob_x_pos > 320)
		blob_x_pos = 0;
	if(blob_y_pos > 240)
		blob_y_pos = 0;
	
	blob_x_buf[0] = 320-blob_x_pos;

	if(x_max_capture_flag == 1)
	{
		if(blob_x_buf[0] > x_max_temp)
		{
			x_max_temp = blob_x_buf[0];				//更新最大值
			x_max_tick = HAL_GetTick();		//获取当前系统时间
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
			//step1：定位帧头
			while(head != 0xA5)
			{
				if(fifo_s_gets(p_fifo,(char*)&head,1) == -1)	//查找帧头
					return;	//已全部取出,未发现帧头，直接返回
			}
			proc_step = 1;	//找到帧头，进行step2
		}
		if(proc_step == 1)
		{
			//step2：读出数据
			if(fifo_s_used(p_fifo) >= (OpenMV_RX_LENGTH - 1))	//判断fifo中的数据长度
			{
				fifo_s_gets(p_fifo,(char*)&OpenMVProcBuf,OpenMV_RX_LENGTH - 3);	//读出数据
				proc_step = 2;	//读出数据完成，进行step3
			}
			else
				return;	//数据还未接收完，退出
		}
		if(proc_step == 2)
		{
			//step3：计算校验和 & 判断帧尾
			if(fifo_s_used(p_fifo) >= 2)
			{
				fifo_s_gets(p_fifo,(char*)&adj_sum,1);	//读出校验和
				i = OpenMV_RX_LENGTH - 3;	// 2021.7.7注释（更改）
				while(i -- )
					sum += OpenMVProcBuf[i];	//计算校验和
				fifo_s_gets(p_fifo,(char*)&end,1);	//读出帧尾
				if(sum == adj_sum && end == 0x5A)
					proc_step = 3;	//校验和&帧尾正确，进行step4
				else
				{
					proc_step = 0;	//错误，返回
					return;
				}
			}
			else
				return;
		}
		if(proc_step == 3)
		{
			proc_step = 0;	//处理完成，重新接收
//			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED闪烁
			OpenMV_DataUnPack(OpenMVProcBuf);//解析数据
//				bolb_max();
		}
	}
}


