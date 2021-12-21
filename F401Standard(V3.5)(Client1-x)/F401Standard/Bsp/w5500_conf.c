/*
**************************************************************************************************
* @file    		w5500_conf.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief  		配置MCU，移植W5500程序需要修改的文件，配置W5500的MAC和IP地址
**************************************************************************************************
*/
#include <stdio.h> 
#include <string.h>

#include "w5500_conf.h"
#include "bsp_gpio.h"
#include "spi.h"
#include "w5500.h"

#define  __CLIENT_1__  1
#define  __CLIENT_2__  2

CONFIG_MSG  ConfigMsg;																	/*配置结构体*/
EEPROM_MSG_STR EEPROM_MSG;															/*EEPROM存储信息结构体*/

/*定义MAC地址,如果多块W5500网络适配板在同一现场工作，请使用不同的MAC地址*/

#ifdef __CLIENT_1__
	uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x13};	/*定义默认IP信息*/
	uint8 local_ip[4]  ={192,168,112,44};		/*定义W5500默认IP地址*/
#elif __CLIENT_2__
	uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x14};	/*定义默认IP信息*/
	uint8 local_ip[4]  ={192,168,112,55};		/*定义W5500默认IP地址*/
#endif
										
uint8 subnet[4]    ={255,255,255,0};										/*定义W5500默认子网掩码*/
uint8 gateway[4]   ={192,168,112,1};											/*定义W5500默认网关*/
uint8 dns_server[4]={114,114,114,114};									/*定义W5500默认DNS*/

uint16 local_port=5000;	                       					/*定义本地端口*/

/*定义远端IP信息*/
uint8  remote_ip[4]={192,168,112,66};											/*远端IP地址*/
uint16 remote_port=5000;																/*远端端口号*/

/*IP配置方法选择，请自行选择*/
uint8	ip_from=IP_FROM_DEFINE;				

uint8   dhcp_ok   = 0;													   			/*dhcp成功获取IP*/
uint32	ms        = 0;															  	/*毫秒计数*/
uint32	dhcp_time = 0;															  	/*DHCP运行计数*/
volatile uint8	ntptimer  = 0;															  	/*NPT秒计数*/

/**
*@brief		配置W5500的IP地址
*@param		无
*@return	无
*/

void set_w5500_ip(void)
{	
		
   /*复制定义的配置信息到配置结构体*/
	memcpy(ConfigMsg.mac, mac, 6);
	memcpy(ConfigMsg.lip,local_ip,4);
	memcpy(ConfigMsg.sub,subnet,4);
	memcpy(ConfigMsg.gw,gateway,4);
	memcpy(ConfigMsg.dns,dns_server,4);
	if(ip_from==IP_FROM_DEFINE)	{}
//		printf(" 使用定义的IP信息配置W5500\r\n");
	
		
	/*以下配置信息，根据需要选用*/	
	ConfigMsg.sw_ver[0]=FW_VER_HIGH;
	ConfigMsg.sw_ver[1]=FW_VER_LOW;	

	/*将IP配置信息写入W5500相应寄存器*/	
	setSUBR(ConfigMsg.sub);
	setGAR(ConfigMsg.gw);
	setSIPR(ConfigMsg.lip);
	
	getSIPR (local_ip);			
	//printf(" W5500 IP地址   : %d.%d.%d.%d\r\n", local_ip[0],local_ip[1],local_ip[2],local_ip[3]);
	getSUBR(subnet);
	//printf(" W5500 子网掩码 : %d.%d.%d.%d\r\n", subnet[0],subnet[1],subnet[2],subnet[3]);
	getGAR(gateway);
	//printf(" W5500 网关     : %d.%d.%d.%d\r\n", gateway[0],gateway[1],gateway[2],gateway[3]);
}

/**
*@brief		配置W5500的MAC地址
*@param		无
*@return	无
*/
void set_w5500_mac(void)
{
	memcpy(ConfigMsg.mac, mac, 6);
	setSHAR(ConfigMsg.mac);	/**/
}



/**
*@brief		W5500片选信号设置函数
*@param		val: 为“0”表示片选端口为低，为“1”表示片选端口为高
*@return	无
*/
void wiz_cs(uint8_t val)
{
	if (val == LOW) 
	{
		User_GPIO_WritePin(W5500_CS_GPIO_Port,W5500_CS_Pin,GPIO_PIN_RESET);
	}
	else if (val == HIGH)
	{
		User_GPIO_WritePin(W5500_CS_GPIO_Port,W5500_CS_Pin,GPIO_PIN_SET);
	}
}

/**
*@brief		设置W5500的片选端口SCSn为低
*@param		无
*@return	无
*/
void iinchip_csoff(void)
{
	wiz_cs(LOW);
}

/**
*@brief		设置W5500的片选端口SCSn为高
*@param		无
*@return	无
*/
void iinchip_cson(void)
{	
   wiz_cs(HIGH);
}

void delay_us(uint32_t n)
{
	int16_t m;
	while(n--)
	{
		m = 80;
		while(m--);
	}
}
/**
*@brief		W5500复位设置函数
*@param		无
*@return	无
*/
void reset_w5500(void)
{
	User_GPIO_WritePin(W5500_RST_GPIO_Port,W5500_RST_Pin,GPIO_PIN_RESET);
	delay_us(100);
	
	User_GPIO_WritePin(W5500_RST_GPIO_Port,W5500_RST_Pin,GPIO_PIN_SET);
	HAL_Delay(1600);
}

uint8_t SPI_SendByte(uint8_t byte)
{
	while (__HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_TXE) == RESET){}//等待发送区空
	
	SPI1->DR = byte;
	  
	while(__HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_BSY) != RESET){};	//判断是否发送完成，发送完成时，BUSY被置0 		    		
	
	while (__HAL_SPI_GET_FLAG(&hspi1,SPI_FLAG_RXNE) == RESET){} //等待接收完一个byte  
	  
	return SPI1->DR;	
}

/**
*@brief		STM32 SPI1读写8位数据
*@param		dat：写入的8位数据
*@return	无
*/
uint8  IINCHIP_SpiSendData(uint8 dat)
{
   return(SPI_SendByte(dat));
}

/**
*@brief		写入一个8位数据到W5500
*@param		addrbsb: 写入数据的地址
*@param   data：写入的8位数据
*@return	无
*/
void IINCHIP_WRITE( uint32 addrbsb,  uint8 data)
{
   iinchip_csoff();                              		
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);	
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4);  
   IINCHIP_SpiSendData(data);                   
   iinchip_cson();                            
}

/**
*@brief		从W5500读出一个8位数据
*@param		addrbsb: 写入数据的地址
*@param   data：从写入的地址处读取到的8位数据
*@return	无
*/
uint8 IINCHIP_READ(uint32 addrbsb)
{
   uint8 data = 0;
   iinchip_csoff();                            
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8))    ;
   data = IINCHIP_SpiSendData(0x00);            
   iinchip_cson();                               
   return data;    
}

/**
*@brief		向W5500写入len字节数据
*@param		addrbsb: 写入数据的地址
*@param   buf：写入字符串
*@param   len：字符串长度
*@return	len：返回字符串长度
*/
uint16 wiz_write_buf(uint32 addrbsb,uint8* buf,uint16 len)
{
   uint16 idx = 0;
   //if(len == 0) printf("Unexpected2 length 0\r\n");
   iinchip_csoff();                               
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4); 
   for(idx = 0; idx < len; idx++)
   {
     IINCHIP_SpiSendData(buf[idx]);
   }
   iinchip_cson();                           
   return len;  
}

/**
*@brief		从W5500读出len字节数据
*@param		addrbsb: 读取数据的地址
*@param 	buf：存放读取数据
*@param		len：字符串长度
*@return	len：返回字符串长度
*/
uint16 wiz_read_buf(uint32 addrbsb, uint8* buf,uint16 len)
{
  uint16 idx = 0;
//  if(len == 0)
//  {
//    printf("Unexpected2 length 0\r\n");
//  }
  iinchip_csoff();                                
  IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
  IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
  IINCHIP_SpiSendData( (addrbsb & 0x000000F8));    
  for(idx = 0; idx < len; idx++)                   
  {
    buf[idx] = IINCHIP_SpiSendData(0x00);
  }
  iinchip_cson();                                  
  return len;
}

