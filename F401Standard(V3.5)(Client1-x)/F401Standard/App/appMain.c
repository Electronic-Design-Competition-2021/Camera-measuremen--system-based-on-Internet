/*************APP****************/
#include "appMain.h"
#include "appMenu.h"
#include "angle_length.h"
#include "openmv.h"
#include "udp_demo.h"

#include "string.h"
#include "usart.h"
//#include "pid.h"
//#include "fifo.h"
//#include "bluetooth.h"
//#include "bsp_24l01.h"
//#include "bsp_platform.h"
//#include "steeringengine.h"
//#include "motorcol.h"
//#include "bsp_imu.h"
/************BSP***************/
#include "bsp_gpio.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_lcd_init.h"
#include "bsp_lcd.h"
#include "bsp_flash.h"
#include "pic.h"
#include "bsp_spi.h"
#include "spi.h"
#include "iwdg.h"
#include "bsp_ec11.h"
/*************LIB****************/
#include "menu.h"
#include "Oscilloscope.h"

/************W5500***************/
#include "w5500.h"
#include "W5500_conf.h"
#include "socket.h"


extern uint16_t after_write_data[FLASH_DATA_LENGHT];
extern int8_t line_cursor;		//行光标
extern float Configs[LINE_NUM];	//参数数组

uint8_t txbuf[5]={0,1,2,3,4};

extern uint32_t spi2_rx_Count;
extern uint8_t spi2_rx_Flag;
void spi1DMASendByte(uint8_t *cmar,uint16_t cndtr);

void appInit(void)
{
	usart2_init();	//初始化串口2
	spi1_Init();	//spi1初始化
	spi2_Init();	//spi2初始化

	ec11_Init();//EC11初始化

	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//初始化SPI
	LCD_Init();			//LCD初始化
	User_Delay(100);
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色

	menuInit(&GUI,menu_main);	//初始化主菜单

	//读flash中的参数
	flash_read(USER_FLASH_ADDRESS, (uint32_t *)Configs, (FLASH_DATA_LENGHT + 3)/ 4);

	/***************************W5500 Init*************************************/
	User_GPIO_WritePin(W5500_CS_GPIO_Port,W5500_CS_Pin,GPIO_PIN_SET);
	User_GPIO_WritePin(W5500_RST_GPIO_Port,W5500_RST_Pin,GPIO_PIN_SET);
	
	HAL_Delay(100);
	reset_w5500();											/*硬复位W5500*/

	set_w5500_mac();										/*配置MAC地址*/
	set_w5500_ip();											/*配置IP地址*/
	socket_buf_init(txsize, rxsize);		/*初始化8个Socket的发送接收缓存大小*/
	/************************************************************************/
	
}

uint32_t ledTick;
uint32_t lcdTick;
uint32_t ec11Tick;
uint32_t netSendTick = 0;
uint32_t UDP_rx_Tick = 0;

uint8_t udp_measure_flag = 0;	
uint8_t udp_start_flag = 0;	//图像发送开启标志位
uint8_t debug_mode_flag = 0;

extern uint8_t img_Buf[26000];
uint8_t img_Data[24000];


uint8_t measure_flag = 0;//启动标志位
uint32_t get_pos_tick = 0;
extern uint16_t x_max;

uint8_t UDP_tx_buf[12];		//UDP发送缓存
uint8_t tx_buf[7];
extern float length;	//长度l
extern uint32_t x_max_tick ;	//捕捉最大值的时间
extern uint8_t osc_flag;
extern uint16_t blob_x_pos;
void appMain(void)
{
	vari_init();	//变量初始化
//	menuSkip(&GUI,menu_StartMeasure);	//跳转至menu_StartMeasure,开始测量（调试用）
	while(1)
	{
		menuRefresh(&GUI);			//刷新GUI:菜单在appMenu.c
		OpenMV_DataProcess(&u2_Fifo);
		
		//开始测量：由终端控制
		if(measure_flag == 1)
		{
			if(User_GetTick() - get_pos_tick >= 100)
			{
				get_pos_tick = User_GetTick();
				
				if(Measure_Xmax_Length() == 2)		/*测量最大值*/
				{
					UDP_tx_buf[0] = 0xB5;
					UDP_tx_buf[1] = 0xB5;
					
//					UDP_tx_buf[2] = length;			//测量得到的L
					memcpy(&UDP_tx_buf[2],&length,4);	//测量得到的L
					memcpy(&UDP_tx_buf[6],&x_max,2);	//测量得到的x_max
					
					
					//校验和
					UDP_tx_buf[8] = UDP_tx_buf[2]+UDP_tx_buf[3]+UDP_tx_buf[4]+
									UDP_tx_buf[5]+UDP_tx_buf[6]+UDP_tx_buf[7];
					
					Tx_udp(UDP_tx_buf,9);//发送到服务端
					HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED闪烁
				}
			}
		}
		
		//LCD_ShowIntNum(80,130,x_max_tick,6,WHITE,BLACK,12);//节点显示获取到最大值所用时间
		/*******************************接收来自服务端UDP数据******************************/
		if(User_GetTick() - UDP_rx_Tick >= 100)
		{
			UDP_rx_Tick = User_GetTick();
			Rx_udp();	//接收来自服务端UDP数据
		}
		
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
		/******************************发送图像和示波器实时坐标**************************************/
		if(HAL_GetTick() - netSendTick >= 100)
		{
			netSendTick = HAL_GetTick();
			
			//发送示波器实时坐标
			if(osc_flag == 1)
			{
				UDP_tx_buf[0] = 0xC5;
				UDP_tx_buf[1] = 0xC5;
				memcpy(&UDP_tx_buf[2],&blob_x_pos,2);	//实时坐标
				//校验和
				UDP_tx_buf[4] = UDP_tx_buf[2]+UDP_tx_buf[3];
				UDP_tx_buf[5] = 0;		//冗余字节，构成偶数，防止图像像素错位
				Tx_udp(UDP_tx_buf,6);//发送到服务端
			}
			
			/*****************一帧图像分三次发到服务端****************************/
			if(udp_start_flag ==  1)		//在GUI界面（appmenu.c）被置位
			{
				static uint8_t n = 0;
				
				if(n == 0)	//加入帧头，用于接收端定位
				{		
					memcpy(img_Data,img_Buf,24000);	//定住这一帧，防止接收端出现断层
					img_Data[0] = 0xA5;
					img_Data[1] = 0xA5;
				}
				
				Tx_udp(img_Data + (n++)*8000,8000);		//发送测试拍摄图像(分三次，每次8000字节)
				if (n > 2)//发送完一帧图像之后，发送一次时间
				{
					n = 0;
//					tx_buf[0] = 0xC5;
//					tx_buf[1] = 0xC5;
//					memcpy(&tx_buf[2],&x_max_tick,4);//获取到捕获到最大值的时间
//					tx_buf[6]=tx_buf[2]+tx_buf[3]+tx_buf[4]+tx_buf[5];
//					Tx_udp(tx_buf,7);//发送到服务端
				}
			}
	
		}

	}
	
}
