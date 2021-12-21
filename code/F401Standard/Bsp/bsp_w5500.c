#include "bsp_w5500.h"
//引脚
/*  */
 /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
	PB5     ------> SPI1_CS
	PB12    ------> SPI1_RST
 */
typedef  unsigned char SOCKET;

//unsigned int Timer2_Counter;

unsigned int S0_SendOK=1;
unsigned int S0_TimeOut=0;

unsigned short S0_Port=5000;

unsigned char S_Data_Buffer[2048];
/******************************* W5500 Write Operation *******************************/
/* Write W5500 Common Register a byte */
void Write_1_Byte(unsigned short reg, unsigned char dat)
{
	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	
	SPI1_ReadWriteByte(reg);

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM1|RWB_WRITE|COMMON_R));
	 

	/* Write 1 byte */
	SPI1_ReadWriteByte(dat);
	 

	/* Set W5500 SCS High */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
}

/* Write W5500 Common Register 2 bytes */
void Write_2_Byte(unsigned short reg, unsigned short dat)
{
	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM2|RWB_WRITE|COMMON_R));
	 

	/* Write 2 bytes */
	SPI1_ReadWriteByte(dat/256);
	 
	SPI1_ReadWriteByte(dat);
	 

	/* Set W5500 SCS High */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
}

/* Write W5500 Common Register n bytes */
void Write_Bytes(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((VDM|RWB_WRITE|COMMON_R));
	 

	/* Write n bytes */
	for(i=0;i<size;i++)
	{
		SPI1_ReadWriteByte(*dat_ptr);
		 

		dat_ptr++;
	}

	/* Set W5500 SCS High */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
}

/* Write W5500 Socket Register 1 byte */
void Write_SOCK_1_Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM1|RWB_WRITE|(s*0x20+0x08)));

	 

	/* Write 1 byte */
	SPI1_ReadWriteByte(dat);
	 

	/* Set W5500 SCS High */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
}

/* Write W5500 Socket Register 2 byte */
void Write_SOCK_2_Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM2|RWB_WRITE|(s*0x20+0x08)));
	 

	/* Write 2 bytes */
	SPI1_ReadWriteByte(dat/256);
	 
	SPI1_ReadWriteByte(dat);
	 

	/* Set W5500 SCS High */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
}

/* Write W5500 Socket Register 2 byte */
void Write_SOCK_4_Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM4|RWB_WRITE|(s*0x20+0x08)));
	 

	/* Write 4 bytes */
	SPI1_ReadWriteByte(*dat_ptr);
	 

	dat_ptr++;
	SPI1_ReadWriteByte(*dat_ptr);
	 

	dat_ptr++;
	SPI1_ReadWriteByte(*dat_ptr);
	 

	dat_ptr++;
	SPI1_ReadWriteByte(*dat_ptr);
	 

	/* Set W5500 SCS High */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
}

/******************************* W5500 Read Operation *******************************/
/* Read W5500 Common register 1 Byte */
unsigned char Read_1_Byte(unsigned short reg)
{
	unsigned char i;

	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM1|RWB_READ|COMMON_R));
	 

	/* Write a dummy byte */
	i=SPI1_ReadWriteByte(0xff);
	SPI1_ReadWriteByte(0x00);
	 

	/* Read 1 byte */
	i=SPI1_ReadWriteByte(0xff);

	/* Set W5500 SCS High*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
	return i;
}

/* Read W5500 Socket register 1 Byte */
unsigned char Read_SOCK_1_Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM1|RWB_READ|(s*0x20+0x08)));
	 

	/* Write a dummy byte */
	i=SPI1_ReadWriteByte(0xff);
	SPI1_ReadWriteByte(0x00);
	 

	/* Read 1 byte */
	i=SPI1_ReadWriteByte(0xff);

	/* Set W5500 SCS High*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
	return i;
}

/* Read W5500 Socket register 2 Bytes (short) */
unsigned short Read_SOCK_2_Byte(SOCKET s, unsigned short reg)
{
	unsigned short i;

/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
	
	/* Write Address */
	SPI1_ReadWriteByte(reg/256);
	 
	SPI1_ReadWriteByte(reg);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((FDM2|RWB_READ|(s*0x20+0x08)));
	 

	/* Write a dummy byte */
	SPI1_ReadWriteByte(0xff);
	SPI1_ReadWriteByte(0x00);
	 
	SPI1_ReadWriteByte(0xff);

	SPI1_ReadWriteByte(0x00);
	 
	i*=256;
	i+=SPI1_ReadWriteByte(0xff);

	/* Set W5500 SCS High*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
	return i;
}

/******************** Read data from W5500 Socket data RX Buffer *******************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=Read_SOCK_2_Byte(s,Sn_RX_RSR);
	if(rx_size==0)		/* if no data received, return */
		return 0;
	if(rx_size>1460)
		rx_size=1460;

	offset=Read_SOCK_2_Byte(s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);		/* Calculate the real physical address */

	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);

	/* Write Address */
	SPI1_ReadWriteByte(offset/256);
	 
	SPI1_ReadWriteByte(offset);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((VDM|RWB_READ|(s*0x20+0x18)));

	 
	
	
	j=SPI1_ReadWriteByte(0xff);
	
	if((offset+rx_size)<S_RX_SIZE)
	{
		/* Read Data */
		for(i=0;i<rx_size;i++)
		{
			SPI1_ReadWriteByte(0x00);
			 
			j=SPI1_ReadWriteByte(0xff);
			*dat_ptr=j;
			dat_ptr++;
		}
	}
	else
	{
		offset=S_RX_SIZE-offset;
		for(i=0;i<offset;i++)
		{
			SPI1_ReadWriteByte(0x00);
			 
			j=SPI1_ReadWriteByte(0xff);
			*dat_ptr=j;
			dat_ptr++;
		}
		/* Set W5500 SCS High*/
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);

		/* Set W5500 SCS Low */
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
		/* Write Address */
		SPI1_ReadWriteByte(0x00);
		 
		SPI1_ReadWriteByte(0x00);
		 
		/* Write Control Byte */
		SPI1_ReadWriteByte((VDM|RWB_READ|(s*0x20+0x18)));
		 
		j=SPI1_ReadWriteByte(0xff);
		/* Read Data */
		for(;i<rx_size;i++)
		{
			SPI1_ReadWriteByte(0x00);
			 
			j=SPI1_ReadWriteByte(0xff);
			*dat_ptr=j;
			dat_ptr++;
		}
	}
	/* Set W5500 SCS High*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);

	/* Update offset*/
	offset1+=rx_size;
	Write_SOCK_2_Byte(s, Sn_RX_RD, offset1);
	Write_SOCK_1_Byte(s, Sn_CR, RECV);					/* Write RECV Command */
	return rx_size;
}

/******************** Write data to W5500 Socket data TX Buffer *******************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	offset=Read_SOCK_2_Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);		/* Calculate the real physical address */

	/* Set W5500 SCS Low */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);

	/* Write Address */
	SPI1_ReadWriteByte(offset/256);
	 
	SPI1_ReadWriteByte(offset);
	 

	/* Write Control Byte */
	SPI1_ReadWriteByte((VDM|RWB_WRITE|(s*0x20+0x10)));

	 

	if((offset+size)<S_TX_SIZE)
	{
		/* Write Data */
		for(i=0;i<size;i++)
		{
			SPI1_ReadWriteByte(*dat_ptr);
			 
			dat_ptr++;
		}
	}
	else
	{
		offset=S_TX_SIZE-offset;
		for(i=0;i<offset;i++)
		{
			SPI1_ReadWriteByte(*dat_ptr);
			 
	
			dat_ptr++;
		}
		/* Set W5500 SCS High*/
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);

		/* Set W5500 SCS Low */
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET);
		/* Write Address */
		SPI1_ReadWriteByte(0x00);

		 
		SPI1_ReadWriteByte(0x00);
		 
		/* Write Control Byte */
		SPI1_ReadWriteByte((VDM|RWB_WRITE|(s*0x20+0x10)));
		 
		/* Read Data */
		for(;i<size;i++)
		{
			SPI1_ReadWriteByte(*dat_ptr);
			 
		
			dat_ptr++;
		}
	}
	/* Set W5500 SCS High*/
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);

	/* Updata offset */
	offset1+=size;
	Write_SOCK_2_Byte(s, Sn_TX_WR, offset1);
	Write_SOCK_1_Byte(s, Sn_CR, SEND);					/* Write SEND Command */
}

/*********************** Set Socket n in TCP Client mode ************************/
unsigned int Socket_Connect(SOCKET s)
{
	/* Set Socket n in TCP mode */
	Write_SOCK_1_Byte(s,Sn_MR,MR_TCP);
	/* Open Socket n */
	Write_SOCK_1_Byte(s,Sn_CR,OPEN);
	HAL_Delay(5);
	//Delay(5);	/* Wait for a moment */
	if(Read_SOCK_1_Byte(s,Sn_SR)!=SOCK_INIT)
	{
		Write_SOCK_1_Byte(s,Sn_CR,CLOSE);		/* Close Socket n */
		return FALSE;
	}

	/* Set Socket n in Server mode */
	Write_SOCK_1_Byte(s,Sn_CR,CONNECT);
	return TRUE;
}
//***************************************************************************************

/* W5500 configuration */
void W5500_Configuration(void)
{
	unsigned char array[6];
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12,GPIO_PIN_SET);//RST
	HAL_Delay(200);
	while((Read_1_Byte(PHYCFGR)&LINK)==0); 		/* Waiting for Ethernet Link */

	Write_1_Byte(MR, RST);
	HAL_Delay(10);

	/* Set Gateway IP as: 192.168.0.1 */
	array[0]=192;
	array[1]=168;
	array[2]=112;
	array[3]=1;
	Write_Bytes(GAR, array, 4);

	/* Set Subnet Mask as: 255.255.255.0 */
	array[0]=255;
	array[1]=255;
	array[2]=255;
	array[3]=0;
	Write_Bytes(SUBR, array, 4);

	/* Set MAC Address as: 0x48,0x53,0x00,0x57,0x55,0x00 */
	array[0]=0x48;
	array[1]=0x53;
	array[2]=0x00;
	array[3]=0x57;
	array[4]=0x55;
	array[5]=0x00;
	Write_Bytes(SHAR, array, 6);

	/* Set W5500 IP as: 192.168.0.20 */
	array[0]=192;
	array[1]=168;
	array[2]=112;
	array[3]=20;
	Write_Bytes(SIPR, array, 4);
}

void Socket0_Config(void)
{
	unsigned char array[4];

	/* set Socket n Port Number */
	Write_SOCK_2_Byte(0, Sn_PORT, S0_Port);

	/* Set Socket n Detination IP as 192.168.0.30 */
	array[0]=192;
	array[1]=168;
	array[2]=112;
	array[3]=88;
	Write_SOCK_4_Byte(0, Sn_DIPR, array);

	/* set Socket n Destination Port Number as 5000 */
	Write_SOCK_2_Byte(0, Sn_DPORTR, 5000);

	/* Set Maximum Segment Size as 1460 */
	Write_SOCK_2_Byte(0, Sn_MSSR, 1460);

	/* Set RX Buffer Size as 2K */
	Write_SOCK_1_Byte(0,Sn_RXBUF_SIZE, 0x02);
	/* Set TX Buffer Size as 2K */
	Write_SOCK_1_Byte(0,Sn_TXBUF_SIZE, 0x02);
}

/********* Process IR Register ********/
void Process_IR(void)
{
	unsigned char i;

	i=Read_1_Byte(SIR);
	if(i&0x01)
	{
		i=Read_SOCK_1_Byte(0,Sn_IR);
		Write_SOCK_1_Byte(0,Sn_IR,i);		/* Clear IR flag */

		if(i&IR_CON)		/* TCP CONNECT established */
		{
//			GPIO_ResetBits(GPIOC, D_OUTPUT2);		/* Turn on LED */
		}
		if(i&IR_DISCON)		/* TCP Disconnect */
		{
			Write_SOCK_1_Byte(0,Sn_CR,CLOSE);		/* Close Socket 0 */
//			GPIO_SetBits(GPIOC, D_OUTPUT2); 		/* Turn off LED */
		}
		if(i&IR_TIMEOUT)
		{
			S0_TimeOut=1;
			Write_SOCK_1_Byte(0,Sn_CR,CLOSE);		/* Close Socket 0 */
//			GPIO_SetBits(GPIOC, D_OUTPUT2); 		/* Turn off LED */
		}
		if(i&IR_SEND_OK)
		{
			S0_SendOK=1;
		}
	}
}

/****************************** Detect Gateway **********************************/
/* If Gateway detected, retuen true                                             */
/* If no Gateway detected, return false                                         */
/* If no need to detect Gateway, skip this procedure                            */
/********************************************************************************/
unsigned int Detect_Gateway(void)
{
	unsigned char array[4];

	/* Set Socket 0 detsination IP as SIP+1 */
	array[0]=192+1;
	array[1]=168+1;
	array[2]=0+1;
	array[3]=20+1;
	Write_SOCK_4_Byte(0,Sn_DIPR, array);

	/* Set Socket 0 in TCP mode */
	Write_SOCK_1_Byte(0,Sn_MR,MR_TCP);
	/* Open Socket 0 */
	Write_SOCK_1_Byte(0,Sn_CR,OPEN);

	HAL_Delay(5);	/* Wait for a moment */
	if(Read_SOCK_1_Byte(0,Sn_SR)!=SOCK_INIT)
	{
		Write_SOCK_1_Byte(0,Sn_CR,CLOSE);		/* Close Socket 0 */
		return FALSE;
	}
	
	Write_SOCK_1_Byte(0,Sn_CR,CONNECT);			/* Initial a TCP CONNECT */

	do
	{
		Process_IR();
		if(S0_TimeOut)
		{
			S0_TimeOut=0;
			return FALSE;		/* No Gateway detected */
		}
		else if(Read_SOCK_1_Byte(0,Sn_DHAR)!=0xff)
		{
			Write_SOCK_1_Byte(0,Sn_CR,CLOSE);		/* Close Socket 0 */
			return TRUE;							/* Gateway detected */
		}
	}while(1);
}

/********** Loop Back test *********/
void Process_LoopBack(void)
{
	unsigned short i;

//	/* Read Data from RX buffer */
//	i=Read_SOCK_Data_Buffer(0, S_Data_Buffer);
//	if(i==0)
//		return;

//	/* Write Data to TX Buffer */
	

	if(S0_SendOK==1)
	{
		S0_SendOK=0;
		unsigned char dat[] = "ABCDEFG\r\n";
		Write_SOCK_Data_Buffer(0, dat, sizeof(dat));
	}
	else
	{
		S0_TimeOut=0;
	}
	while((S0_SendOK==0)&&(S0_TimeOut==0))
		Process_IR();
}

void SPI1_w5500_ReInit(uint8_t SPI_BaudRatePrescaler)
{
	__HAL_SPI_DISABLE(&hspi1);	//失能SPI
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
	
	SPI1->CR1&=0XFFC7;//位3-5清零，用来设置波特率
	SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI1速度 
	
//	SPI1->CR1 &= ~SPI_CR1_DFF;
//	SPI1->CR1 |= (SPI_DATASIZE_8BIT & SPI_CR1_DFF);
	SPI1->CR1 &= ~SPI_CR1_CPOL;	//清空寄存器CPOL位
	SPI1->CR1 |= (SPI_POLARITY_LOW & SPI_CR1_CPOL); // 设置CPOL = LOW
	SPI1->CR1 &= ~SPI_CR1_CPHA;	//清空寄存器CPHA位
	SPI1->CR1 |= (SPI_PHASE_1EDGE & SPI_CR1_CPHA);// 设置CPHA = 1EDAGE
	
	__HAL_SPI_ENABLE(&hspi1);	//使能SPI
}


