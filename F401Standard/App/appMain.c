/************APP***************/
#include "appMain.h"
#include "appMenu.h"
#include "udp_demo.h"

#include "string.h"
#include "usart.h"
//#include "pid.h"
//#include "fifo.h"
//#include "bluetooth.h"
/***********BSP***************/
#include "bsp_gpio.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_spi.h"
#include "bsp_lcd_init.h"
#include "bsp_lcd.h"
#include "pic.h"
#include "spi.h"
#include "bsp_ec11.h"

//#include "bsp_24l01.h"
//#include "bsp_platform.h"
//#include "steeringengine.h"
//#include "motorcol.h"
//#include "bsp_imu.h"
//#include "bsp_flash.h"
/***********LIB*****************/
#include "menu.h"
#include "Oscilloscope.h"

/***********UDP****************/
#include "socket.h"
#include "w5500.h"
#include "W5500_conf.h"

uint8_t txbuf[5]={0,1,2,3,4};

void appInit(void)
{
	/***********************************************************************************/
	spi1_Init();	//spi1初始化
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//初始化SPI

	CS_port=LCD2_CS_GPIO_Port;//PortA
	CS_pin=LCD2_CS_Pin|LCD_CS_Pin;//片选两个LCD
	
	LCD_Init();			//LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色
	//显示每一行的标题
//	LCD_ShowString(6,100,"A:",GREEN,BLACK,12,0);		//Angle
//	LCD_ShowString(70,100,"L:",GREEN,BLACK,12,0);		//Length
	User_Delay(100);
	menuInit(&GUI,menu_main);	//初始化主菜单
	
	/*******************************W5500 Config****************************************/
	User_GPIO_WritePin(W5500_CS_GPIO_Port,W5500_CS_Pin,GPIO_PIN_SET);//CS拉高
	User_GPIO_WritePin(W5500_RST_GPIO_Port,W5500_RST_Pin,GPIO_PIN_SET);//RST拉高
	
	HAL_Delay(100);
	reset_w5500();											/*硬复位W5500*/

	set_w5500_mac();										/*配置MAC地址*/
	set_w5500_ip();											/*配置IP地址*/
	socket_buf_init(txsize, rxsize);		/*初始化8个Socket的发送接收缓存大小*/
	/***********************************************************************************/
	// 预留1秒时间关闭看门狗，按下按键则关闭看门狗
                       
}

uint32_t ledTick;
uint32_t lcdTick;
uint32_t ec11Tick;

uint8_t c1_img_Buf[26000];//接收Client1图像缓冲区
uint8_t c2_img_Buf[26000];//接收Client2图像缓冲区

extern uint32_t x_max_tick;//获取捕获到最大值的时间

void appMain(void)
{
	while(1)
	{
		CS_port=LCD2_CS_GPIO_Port;
		CS_pin=LCD2_CS_Pin|LCD_CS_Pin;//片选两个LCD
		menuRefresh(&GUI);			//刷新GUI:菜单在appMenu.c

		Rx_udp(c1_img_Buf,c2_img_Buf);/*接收Client1&Client2的数据*/

		if(User_GetTick() - lcdTick >= 	50)
		{
			lcdTick = User_GetTick();
			menuRefreshFlagSet(&GUI);	//menu刷新标志位置位
		}	
		
		if(User_GetTick() - ec11Tick >= 10)
		{
			ec11Tick = User_GetTick();
			EC11_ScanButton();  
		}		
		
		if(User_GetTick() - ledTick >= 	500)
		{
			ledTick = User_GetTick();
			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED闪烁
		}
	}
}
