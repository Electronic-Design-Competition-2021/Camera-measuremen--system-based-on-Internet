#include "fifo.h"
#include "spi.h"
#include "bsp_spi.h"
//uint8_t spi_Buf[2][SPI2_BUF_LENGHT];
//uint8_t spi2_Fifo_Buf[SPI2_FIFO_BUF_LENGTH];
//fifo_s_t spi2_Fifo;
uint8_t img_Buf[26000];
uint32_t spi2_rx_Count = 0;
uint8_t spi2_rx_Flag = 0;
void spi1_Init(void)
{
	__HAL_SPI_ENABLE(&hspi1);	//使能SPI1
	SET_BIT(hspi1.Instance->CR2, SPI_CR2_TXDMAEN);	//开启spi1的DMA

}
void spi2_Init(void)
{
	//	fifo_s_init(&spi2_Fifo, spi2_Fifo_Buf, SPI2_FIFO_BUF_LENGTH);	//将spi2_Fifo_Buf初始化为fifo缓冲区
	__HAL_SPI_ENABLE(&hspi2);	//使能SPI2	
	__HAL_SPI_ENABLE_IT(&hspi2,SPI_IT_RXNE);
	//	SET_BIT(hspi2.Instance->CR2, SPI_CR2_RXDMAEN);	//开启spi2的接收DMA

}


//SPI2接收中断

//用于捕获openmv驱动spi显示屏的图像数据

//盗版openmv：
//对应像素为三个“0x43 0xc6"  (每个像素16位)，尾部像素为三个"0xC5 0xDB"


//正版openmv:
//"0x43" "0xC5" "0x43" "0xC5" "0x43" "0xC5"
uint8_t spi2_RX_Process = 0;

//void SPI2_IRQHandler(void)
//{
//	uint8_t rx_temp = 0;
//	if(__HAL_SPI_GET_FLAG(&hspi2,SPI_FLAG_RXNE) != RESET)
//	{
////		if(spi2_rx_Flag == 1)
////			return;		//若上一帧数据还未处理完，则不接收
//		
//		if(spi2_RX_Process >= 6)
//		{
//			rx_temp = SPI2->DR;
//			//帧头接收完毕，开始接收图像数据
//			img_Buf[spi2_rx_Count++] = rx_temp;	//接收一字节数据;
//			if(spi2_rx_Count >= 24576)	//限制接收大小
//			{
//				spi2_rx_Flag = 1;
//				spi2_rx_Count = 0;
//				spi2_RX_Process = 0;	//重新接收
//			}
//			
//			if(spi2_RX_Process == 6)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process++;
//			}
//			else if(spi2_RX_Process == 7)
//			{
//				if(rx_temp == 0xDB)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 8)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 9)
//			{
//				if(rx_temp == 0xDB)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 10)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 11)
//			{
//				if(rx_temp == 0xDB)
//				{
//					//图像接收完毕
//					spi2_rx_Flag = 1;
//					spi2_rx_Count=0;
//					spi2_RX_Process = 0;
//				}
//				else
//					spi2_RX_Process = 6;
//			}
//		}
//		else 
//		{
//			rx_temp = SPI2->DR;	//接收一字节数据
//			
//			if(spi2_RX_Process == 0)
//			{
//				if(rx_temp == 0x43)
//					spi2_RX_Process++;
//			}
//			else if(spi2_RX_Process == 1)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 2)
//			{
//				if(rx_temp == 0x43)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 3)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 4)
//			{
//				if(rx_temp == 0x43)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 5)
//			{
//				if(rx_temp == 0xc5)
//				{
//					spi2_RX_Process = 6;
//				}
//				else
//					spi2_RX_Process = 0;
//			}
//		}
//	}
//}
//void SPI2_IRQHandler(void)
//{
//	uint8_t rx_temp = 0;
//	if(__HAL_SPI_GET_FLAG(&hspi2,SPI_FLAG_RXNE) != RESET)
//	{
////		if(spi2_rx_Flag == 1)
////			return;		//若上一帧数据还未处理完，则不接收
//		
//		if(spi2_RX_Process >= 6)
//		{
//			rx_temp = SPI2->DR;
//			//帧头接收完毕，开始接收图像数据
//			img_Buf[spi2_rx_Count++] = rx_temp;	//接收一字节数据;
//			if(spi2_rx_Count >= 24576)	//限制接收大小
//			{
//				spi2_rx_Flag = 1;
//				spi2_rx_Count = 0;
//				spi2_RX_Process = 0;	//重新接收
//			}
//			
//			if(spi2_RX_Process == 6)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process++;
//			}
//			else if(spi2_RX_Process == 7)
//			{
//				if(rx_temp == 0xDB)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 8)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 9)
//			{
//				if(rx_temp == 0xDB)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 10)
//			{
//				if(rx_temp == 0xc5)
//					spi2_RX_Process ++;
//				else
//					spi2_RX_Process = 6;
//			}
//			else if(spi2_RX_Process == 11)
//			{
//				if(rx_temp == 0xDB)
//				{
//					//图像接收完毕
//					spi2_rx_Flag = 1;
//					spi2_rx_Count=0;
//					spi2_RX_Process = 0;
//				}
//				else
//					spi2_RX_Process = 6;
//			}
//		}
//		else 
//		{
//			rx_temp = SPI2->DR;	//接收一字节数据
//			
//			if(spi2_RX_Process == 0)
//			{
//				if(rx_temp == 0x21)
//					spi2_RX_Process++;
//			}
//			else if(spi2_RX_Process == 1)
//			{
//				if(rx_temp == 0xe2)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 2)
//			{
//				if(rx_temp == 0xa1)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 3)
//			{
//				if(rx_temp == 0xe2)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 4)
//			{
//				if(rx_temp == 0xa1)
//					spi2_RX_Process++;
//				else
//					spi2_RX_Process = 0;
//			}
//			else if(spi2_RX_Process == 5)
//			{
//				if(rx_temp == 0xe2)
//				{
//					spi2_RX_Process = 6;
//				}
//				else
//					spi2_RX_Process = 0;
//			}
//		}
//	}
//}

void SPI2_IRQHandler(void)
{
	
	uint8_t rx_temp = 0;
	if(__HAL_SPI_GET_FLAG(&hspi2,SPI_FLAG_RXNE) != RESET)
	{
//		if(spi2_rx_Flag == 1)
//			return;		//若上一帧数据还未处理完，则不接收
		rx_temp = SPI2->DR;
		if(spi2_RX_Process >= 6)
		{
			//帧头接收完毕，开始接收图像数据
			img_Buf[spi2_rx_Count++] = rx_temp;	//接收一字节数据;
			if(spi2_rx_Count >= 24576)	//限制接收大小
			{
				spi2_rx_Flag = 1;
				spi2_rx_Count = 0;
				spi2_RX_Process = 0;	//重新接收
			}
			
			if(spi2_RX_Process == 6)
			{
				if(rx_temp == 0xc5)
					spi2_RX_Process++;
			}
			else if(spi2_RX_Process == 7)
			{
				if(rx_temp == 0xDB)
					spi2_RX_Process ++;
				else
					spi2_RX_Process = 6;
			}
			else if(spi2_RX_Process == 8)
			{
				if(rx_temp == 0xc5)
					spi2_RX_Process ++;
				else
					spi2_RX_Process = 6;
			}
			else if(spi2_RX_Process == 9)
			{
				if(rx_temp == 0xDB)
					spi2_RX_Process ++;
				else
					spi2_RX_Process = 6;
			}
			else if(spi2_RX_Process == 10)
			{
				if(rx_temp == 0xc5)
					spi2_RX_Process ++;
				else
					spi2_RX_Process = 6;
			}
			else if(spi2_RX_Process == 11)
			{
				if(rx_temp == 0xDB)
				{
					//图像接收完毕
					spi2_rx_Flag = 1;
					spi2_rx_Count=0;
					spi2_RX_Process = 0;
				}
				else
					spi2_RX_Process = 6;
			}
		}
		else 
		{	
			//rx_temp = SPI2->DR;
			if(spi2_RX_Process == 0)
			{
				if(rx_temp == 0x43)
					spi2_RX_Process++;
			}
			else if(spi2_RX_Process == 1)
			{
				if(rx_temp == 0xc6)
					spi2_RX_Process++;
				else
					spi2_RX_Process = 0;
			}
			else if(spi2_RX_Process == 2)
			{
				if(rx_temp == 0x43)
					spi2_RX_Process++;
				else
					spi2_RX_Process = 0;
			}
			else if(spi2_RX_Process == 3)
			{
				if(rx_temp == 0xc6)
					spi2_RX_Process++;
				else
					spi2_RX_Process = 0;
			}
			else if(spi2_RX_Process == 4)
			{
				if(rx_temp == 0x43)
					spi2_RX_Process++;
				else
					spi2_RX_Process = 0;
			}
			else if(spi2_RX_Process == 5)
			{
				if(rx_temp == 0xc6)
				{
					spi2_RX_Process = 6;
				}
				else
					spi2_RX_Process = 0;
			}
		}
	}
}

