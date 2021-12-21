#include "openmv.h"
#include "usart.h"


unsigned char RXBUF[4];
unsigned char RXCNT=0;
unsigned char RXOVER;
void USART2_IRQHandler(void)
{
	uint8_t temp = 0;
	if(USART2->SR & UART_FLAG_RXNE)
	{
		__HAL_UART_CLEAR_PEFLAG(&huart6);		//清除标志位
		temp = USART2->DR;
			if(temp == '\n')
			{
				RXOVER = 1;
				RXCNT = 0;
			}
			else
			{
				RXBUF[RXCNT] = temp;
				RXCNT ++;
				if(RXCNT == 4)
				{
					RXCNT = 0;
				}
			}
	}

}

