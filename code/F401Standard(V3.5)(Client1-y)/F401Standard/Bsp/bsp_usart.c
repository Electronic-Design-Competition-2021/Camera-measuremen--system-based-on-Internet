#include "bsp_usart.h"
#include "fifo.h"
#include "usart.h"
#include <stdarg.h>
#include "bsp_imu.h"

uint8_t usart1_Buf[2][U1_BUF_LENGHT];
uint8_t u1_Fifo_Buf[U1_FIFO_BUF_LENGTH];
fifo_s_t u1_Fifo;

uint8_t usart2_Buf[2][U2_BUF_LENGHT];
uint8_t u2_Fifo_Buf[U2_FIFO_BUF_LENGTH];
fifo_s_t u2_Fifo;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

void usart1_init(void)
{

	fifo_s_init(&u1_Fifo, u1_Fifo_Buf, U1_FIFO_BUF_LENGTH);	//将u1_Fifo_Buf初始化为fifo缓冲区
	
    //enable the DMA transfer for the receiver and tramsmit request
    //使能DMA串口接收和发送
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAR);
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAT);

    //enalbe idle interrupt
    //使能空闲中断
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    //disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart1_rx);
    
    while(hdma_usart1_rx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart1_rx);
    }

	//DMA_LISR 寄存器用于管理数据流 0~3，而 DMA_HISR 用于管理数据流 4~7,TCIF为传输完成标志
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, DMA_LISR_TCIF2);	//usart1 rx 为DMA2 stream2

    hdma_usart1_rx.Instance->PAR = (uint32_t) & (USART1->DR);
    //memory buffer 1
    //内存缓冲区1
    hdma_usart1_rx.Instance->M0AR = (uint32_t)(usart1_Buf[0]);
    //memory buffer 2
    //内存缓冲区2
    hdma_usart1_rx.Instance->M1AR = (uint32_t)(usart1_Buf[1]);
    //data length
    //数据长度
    __HAL_DMA_SET_COUNTER(&hdma_usart1_rx, U1_BUF_LENGHT);

    //enable double memory buffer
    //使能双缓冲区
    SET_BIT(hdma_usart1_rx.Instance->CR, DMA_SxCR_DBM);

    //enable DMA
    //使能DMA接收
    __HAL_DMA_ENABLE(&hdma_usart1_rx);


	/* 设置DMA发送 */
	
    //disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart1_tx);

    while(hdma_usart1_tx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart1_tx);
    }

    hdma_usart1_tx.Instance->PAR = (uint32_t) & (USART1->DR);

}

void usart1_SendData(uint8_t *data, uint16_t len)
{
	if(hdma_usart1_tx.Instance->CR & DMA_SxCR_EN)
	{
		//判断DMA是否正在传输
		//等待DMA传输完成
		while(__HAL_DMA_GET_FLAG(&hdma_usart1_tx, DMA_FLAG_TCIF3_7) == 0){};	//usart1 tx 为DMA2 stream7
	}
	//disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart1_tx);

    while(hdma_usart1_tx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart1_tx);
    }
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_HISR_TCIF7);

    hdma_usart1_tx.Instance->M0AR = (uint32_t)(data);
    __HAL_DMA_SET_COUNTER(&hdma_usart1_tx, len);

    __HAL_DMA_ENABLE(&hdma_usart1_tx);
}


/**
  * @brief          蓝牙数据发送
  * @note 			该函数实现DMA方式的printf，并且按照蓝牙协议将数据打包发送至蓝牙上位机
  * @param[in]      与printf的用法相同
  * @retval         none
  */

uint8_t printf_buf[128];
void User_printf(const char *format,...)
{
	uint32_t length;
	va_list args;
	va_start(args, format);
	length = vsnprintf((char*)(printf_buf), sizeof(printf_buf), (char*)format, args);//第二个参数：最大数据长度
	va_end(args);
	usart1_SendData(printf_buf,length);
}

//串口中断：空闲中断 + DMA双缓冲区
void USART1_IRQHandler(void)
{
    static volatile uint8_t res;
    if(USART1->SR & UART_FLAG_IDLE)	//发生空闲中断
    {
        __HAL_UART_CLEAR_PEFLAG(&huart1);	//清除中断标志位

        static uint16_t this_time_rx_len = 0;

        if ((huart1.hdmarx->Instance->CR & DMA_SxCR_CT) == RESET)	//数据在缓存区0
        {
			//失能DMA
            __HAL_DMA_DISABLE(huart1.hdmarx);
			
			//获取接收到的数据长度,长度 = 设定长度 - 剩余长度
            this_time_rx_len = U1_BUF_LENGHT - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
			
			//设定DMA传输字节数：512字节
            __HAL_DMA_SET_COUNTER(huart1.hdmarx, U1_BUF_LENGHT);
			
			//将下次接收数据的buff设为usart1_buf[1]
            huart1.hdmarx->Instance->CR |= DMA_SxCR_CT;
			
			//使能DMA
            __HAL_DMA_ENABLE(huart1.hdmarx);
			
			//将数据存入fifo中
            fifo_s_puts(&u1_Fifo, (char*)usart1_Buf[0], this_time_rx_len);
        }
        else		//如果缓冲区1有数据
        {
            __HAL_DMA_DISABLE(huart1.hdmarx);
            this_time_rx_len = U1_BUF_LENGHT - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
            __HAL_DMA_SET_COUNTER(huart1.hdmarx, U1_BUF_LENGHT);
			//将下次接收数据的buff设为usart6_buf[0]
            huart1.hdmarx->Instance->CR &= ~(DMA_SxCR_CT);
            __HAL_DMA_ENABLE(huart1.hdmarx);
            fifo_s_puts(&u1_Fifo, (char*)usart1_Buf[1], this_time_rx_len);
        }
    }
}

void usart2_init(void)
{

	fifo_s_init(&u2_Fifo, u2_Fifo_Buf, U2_FIFO_BUF_LENGTH);	//将u2_Fifo_Buf初始化为fifo缓冲区
	
    //enable the DMA transfer for the receiver and tramsmit request
    //使能DMA串口接收和发送
    SET_BIT(huart2.Instance->CR3, USART_CR3_DMAR);
    SET_BIT(huart2.Instance->CR3, USART_CR3_DMAT);

    //enalbe idle interrupt
    //使能空闲中断
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

    //disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart2_rx);
    
    while(hdma_usart2_rx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart2_rx);
    }

	//DMA_LISR 寄存器用于管理数据流 0~3，而 DMA_HISR 用于管理数据流 4~7,TCIF为传输完成标志
    __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, DMA_HISR_TCIF5);	//usart2 rx 为DMA1 stream5

    hdma_usart2_rx.Instance->PAR = (uint32_t) & (USART2->DR);
    //memory buffer 1
    //内存缓冲区1
    hdma_usart2_rx.Instance->M0AR = (uint32_t)(usart2_Buf[0]);
    //memory buffer 2
    //内存缓冲区2
    hdma_usart2_rx.Instance->M1AR = (uint32_t)(usart2_Buf[1]);
    //data length
    //数据长度
    __HAL_DMA_SET_COUNTER(&hdma_usart2_rx, U2_BUF_LENGHT);

    //enable double memory buffer
    //使能双缓冲区
    SET_BIT(hdma_usart2_rx.Instance->CR, DMA_SxCR_DBM);

    //enable DMA
    //使能DMA接收
    __HAL_DMA_ENABLE(&hdma_usart2_rx);


	/* 设置DMA发送 */
	
    //disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&hdma_usart2_tx);

    while(hdma_usart2_tx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart2_tx);
    }

    hdma_usart2_tx.Instance->PAR = (uint32_t) & (USART2->DR);

}

#include "openmv.h"

//串口中断：空闲中断 + DMA双缓冲区
//接收OpenMV发送的色块中心坐标
void USART2_IRQHandler(void)
{
    static volatile uint8_t res;
    if(USART2->SR & UART_FLAG_IDLE)	//发生空闲中断
    {
        __HAL_UART_CLEAR_PEFLAG(&huart2);	//清除中断标志位

        static uint16_t this_time_rx_len = 0;

        if ((huart2.hdmarx->Instance->CR & DMA_SxCR_CT) == RESET)	//数据在缓存区0
        {
			//失能DMA
            __HAL_DMA_DISABLE(huart2.hdmarx);
			
			//获取接收到的数据长度,长度 = 设定长度 - 剩余长度
            this_time_rx_len = U2_BUF_LENGHT - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
			
			//设定DMA传输字节数：512字节
            __HAL_DMA_SET_COUNTER(huart2.hdmarx, U2_BUF_LENGHT);
			
			//将下次接收数据的buff设为usart1_buf[1]
            huart2.hdmarx->Instance->CR |= DMA_SxCR_CT;
			
			//使能DMA
            __HAL_DMA_ENABLE(huart2.hdmarx);
			
			//将数据存入fifo中
            fifo_s_puts(&u2_Fifo, (char*)usart2_Buf[0], this_time_rx_len);
        }
        else		//如果缓冲区1有数据
        {
            __HAL_DMA_DISABLE(huart2.hdmarx);
            this_time_rx_len = U2_BUF_LENGHT - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
            __HAL_DMA_SET_COUNTER(huart2.hdmarx, U2_BUF_LENGHT);
			//将下次接收数据的buff设为usart2_buf[0]
            huart2.hdmarx->Instance->CR &= ~(DMA_SxCR_CT);
            __HAL_DMA_ENABLE(huart2.hdmarx);
            fifo_s_puts(&u2_Fifo, (char*)usart2_Buf[1], this_time_rx_len);
        }
		OpenMV_DataProcess(&u2_Fifo);	//因主函数任务过多，防止数据处理不及时导致测量周期出现误差
    }
}




//串口6中断服务程序：陀螺仪模块数据接收
//注意,读取USARTx->SR能避免莫名其妙的错误
//void USART6_IRQHandler(void)                	//串口1中断服务程序
//{		
//	uint8_t ch;
//	if(USART6->SR & UART_FLAG_RXNE)	//发生空闲中断
//    {
//		ch = USART6->DR;	                    //读取接收到的数据
//		packet_decode(ch);                                      //解析数据
//	}
//}


//    if(huart1.Instance->SR & UART_FLAG_RXNE)	//接收到数据
//    {
////        __HAL_UART_CLEAR_PEFLAG(&huart4);		//清除标志位
//		data_temp = huart1.Instance->DR;	//接收数据，同时清除标志位
//		if(push_flag == 0)
//			bt_rx_buf[0][this_time_rx_len++] = data_temp;	//接收数据
//		else
//			push_flag = 0;	//丢掉这一字节
//		if(this_time_rx_len == BT_FRAME_LEN)	//如果接收到了定长数据
//		{
//			this_time_rx_len = 0;
//			//处理蓝牙接收数据
//			if(bt_rx_buf[0][0] == 0XA5 && bt_rx_buf[0][BT_FRAME_LEN - 1] == 0X5A)
//			{
//				for(i = 1;i<BT_FRAME_LEN-2;i++)
//				{
//					frame_sum += bt_rx_buf[0][i];		//累加校验和
//					*bt_frame_p++ = bt_rx_buf[0][i];	//接收数据：4个4字节int类型
//				}
//				bt_frame_p = (uint8_t *)bt_frame;				//重置数据地址
//				if(frame_sum == bt_rx_buf[0][BT_FRAME_LEN - 2])	//判断校验和
//				{
//					bt_rx_flag = 1;		//接收完成标志位置位
//				}				
//			}
//			else
//			{
//				push_flag = 1;	//下一字节丢弃
//			}
//		}
//    }




