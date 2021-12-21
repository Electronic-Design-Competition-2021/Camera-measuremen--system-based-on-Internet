/**
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  * @file      	appMenu.c/h
  * @brief     	多级菜单（用户层）
  * @note      	每个菜单界面以一个独立函数的形式被调用，新建菜单仅需新建一个函数，
				通过调用menuSkip实现菜单跳转，相关函数见menu.c/h
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     July-1-2021     doee            1. 完成
  * @blog		https://blog.csdn.net/weixin_44578655?spm=1001.2101.3001.5343
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  */
#include "appMenu.h"
#include "main.h"
#include "stdio.h"
#include "bsp_lcd.h"
#include "bsp_lcd_init.h"
//#include "bluetooth.h"
//#include "bsp_flash.h"
//#include "bsp_24l01.h"
#include "menu.h"
#include "Oscilloscope.h"
#include "bsp_ec11.h"
#include "math.h"
#include "udp_demo.h"
#include "string.h"

menu_s_t GUI;	//定义多级菜单

uint8_t lcdBuf[64];
//extern uint16_t write_data[FLASH_DATA_LENGHT];
//extern uint16_t after_write_data[FLASH_DATA_LENGHT];

float Configs[LINE_NUM];	//参数数组
int8_t line_cursor;		//行光标
int8_t screen_flag;		//菜单标号
int8_t back_flag;


uint8_t start_buf[3] = {0x11,0x11,0x11};
uint8_t done_buf[3]  = {0x12,0x12,0x12};
uint8_t Bin_buf[3] 	 = {0x13,0x13,0x13};
uint8_t Osc_on_buf[3] 	 = {0x14,0x14,0x14};
uint8_t Osc_off_buf[3] 	 = {0x15,0x15,0x15,};

uint8_t start_measure_flag = 0;
uint32_t measure_tick = 0;//测量标志位
uint8_t beep_flag = 0;//蜂鸣器标志位
uint32_t beep_tick = 0;


extern volatile int ec11_num;

extern uint8_t ang_rx_count;	//角度数据接收计数
extern uint8_t c1_len_rx_count,c2_len_rx_count;	//长度数据接收计数


#define LEN_MEASURE_NUM  10
#define ANGLE_MEASURE_NUM  5

extern float angle_buf[ANGLE_MEASURE_NUM];	//原始角度数据（5次）
extern float c1_length_raw_buff[LEN_MEASURE_NUM],c2_length_raw_buff[LEN_MEASURE_NUM];	//原始长度数据（10次）
extern uint8_t ang_measure_done_flag;		//angle测量完成标志位
extern uint8_t c1_len_measure_done_flag;	//c1_len测量完成标志
extern uint8_t c2_len_measure_done_flag;	//c2_len测量完成标志

//测量结果
extern float angle;
extern float c1_length,c2_length;


//主界面（初始界面）
void menu_main(void)
{
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//重新初始化SPI1	
	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11编码器数值
	static uint8_t selected_flag = 0 ;	//行选中
	if(GUI.init_flag)
	{
		//发生界面切换,进行页面初始化
		GUI.init_flag = 0;	//标志位清零
		LCD_Fill(0,112,LCD_W,LCD_H,BLACK);	//初始颜色:黑色
		LCD_ShowString(6,100,"A:",GREEN,BLACK,12,0);		//角度
		LCD_ShowString(70,100,"L:",GREEN,BLACK,12,0);		//长度
		LCD_ShowString(6,112,"k_lmin",GREEN,BLACK,12,0);		//第一问
		LCD_ShowString(70,112,"k_lmax",GREEN,BLACK,12,0);		//第二问
		LCD_ShowString(6,124,"k_a0",GREEN,BLACK,12,0);			//第三问
		LCD_ShowString(70,124,"k_a9",GREEN,BLACK,12,0);			//第四问
		LCD_ShowString(6,136,"k_a1",GREEN,BLACK,12,0);			//第五问
		LCD_ShowString(70,136,"k_a4",GREEN,BLACK,12,0);			//第六问
		LCD_ShowString(6,148,"OSC",GREEN,BLACK,12,0);			//示波器
		LCD_ShowString(70,148,"BIN:",GREEN,BLACK,12,0);			//二值化

		ec11_count = EC11_getCount();	//获取EC11计数
		last_ec11_count = ec11_count;	//记录初始EC11计数
		line_cursor = 0;				//初始光标归零
//		LCD_Fill(0,112+(line_cursor)*12,COLUMN0,112+(line_cursor+1)*12,RED);	//在指定行标识红色色块
	}
	
	ec11_count = EC11_getCount();	//获取EC11计数
	
	if(selected_flag == 0)
	{
		if(ec11_count != last_ec11_count)
		{
			if(ec11_count > last_ec11_count)
				line_cursor -= 1;
			else
				line_cursor += 1;
			
			line_cursor = line_cursor > 7 ? 7 : line_cursor;		//移动到头
			line_cursor = line_cursor < 0 ? 0 : line_cursor;		//	
			LCD_Fill(0,112,COLUMN0,LCD_H,BLACK);							//清空标识
			LCD_Fill(64,112,COLUMN1,LCD_H,BLACK);							//清空标识
			
			//标记色块
			if(line_cursor % 2 == 0)
				LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//在指定行标识红色色块
			else
				LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//在指定行标识红色色块		
		}
		
		last_ec11_count = ec11_count;
		
		if(EC11_getButtonFlag())	//按键被按下
		{
			if(line_cursor == 6)
			{
				Tx_udp(Osc_on_buf,3);		//发送示波器指令
				HAL_Delay(12);
				Tx_udp(Osc_on_buf,3);		//发送示波器指令
				
				menuSkip(&GUI,menu_osc);
				return;
			}
			else if(line_cursor == 7)
			{
				Tx_udp(Bin_buf,3);		//发送二值化指令
				return;
			}
			else
			{
				start_measure_flag = 1;				//开始测量
				
				measure_tick = HAL_GetTick();		//记录开始时间
								//标志位清零
				ang_measure_done_flag = 0;
				c1_len_measure_done_flag = 0;
				c2_len_measure_done_flag = 0;

				//数据清零
				ang_rx_count = 0;		//角度接收5次
				c1_len_rx_count = 0;	//长度接收10次
				c2_len_rx_count = 0;
				angle = 0;
				c1_length = 0;
				c2_length = 0;
				memset(angle_buf,0,ANGLE_MEASURE_NUM);
				memset(c1_length_raw_buff,0,LEN_MEASURE_NUM);
				memset(c2_length_raw_buff,0,LEN_MEASURE_NUM);
				
				//发送开始指令
				Tx_udp(start_buf,3);				
				HAL_Delay(12);		//delay
				Tx_udp(start_buf,3);				//发送开始指令(发送两次，防止接收不到)
				
			}
			
			selected_flag=1;	//行选中 or 未选中
			
			if(selected_flag)
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,WHITE);	//在指定行标识白色色块
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,WHITE);	//在指定行标识白色色块
			}
			else
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//在指定行标识白色色块
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//在指定行标识白色色块	
			}				
		}	
	}
	else							//异常时打断测量
	{
		if(EC11_getButtonFlag())	//按键被按下
		{
			
				start_measure_flag = 0;				//结束测量
				
				measure_tick = HAL_GetTick();		//记录开始时间
								//标志位清零
				ang_measure_done_flag = 0;
				c1_len_measure_done_flag = 0;
				c2_len_measure_done_flag = 0;

				//数据清零
				ang_rx_count = 0;		//角度接收5次
				c1_len_rx_count = 0;	//长度接收10次
				c2_len_rx_count = 0;
				angle = 0;
				c1_length = 0;
				c2_length = 0;
				memset(angle_buf,0,ANGLE_MEASURE_NUM);
				memset(c1_length_raw_buff,0,LEN_MEASURE_NUM);
				memset(c2_length_raw_buff,0,LEN_MEASURE_NUM);
				LCD_ShowIntNum(COLUMN0+36,112+(line_cursor/2)*12,(HAL_GetTick() - measure_tick)/1000,2,BLACK,BLACK,12);//清除计时
				//发送开始指令
				Tx_udp(done_buf,3);				
				HAL_Delay(12);		//delay
				Tx_udp(done_buf,3);				//发送开始指令(发送两次，防止接收不到)
				
			
			
			selected_flag=0;	//行选中 or 未选中
			
			if(selected_flag)
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,WHITE);	//在指定行标识白色色块
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,WHITE);	//在指定行标识白色色块
			}
			else
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//在指定行标识白色色块
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//在指定行标识白色色块	
			}				
		}	
	}
	if(start_measure_flag == 1)
	{
		//三个待测数据都测量完成，提前结束
		if(ang_measure_done_flag && c1_len_measure_done_flag && c2_len_measure_done_flag)
		{
			//标志位清零
			ang_measure_done_flag = 0;
			c1_len_measure_done_flag = 0;
			c2_len_measure_done_flag = 0;
			
			//发送结束指令
			Tx_udp(done_buf,3);				
			HAL_Delay(12);
			Tx_udp(done_buf,3);				//发送结束指令(发送两次)
			
			//显示角度
			LCD_ShowFloatNum1(18,100, angle,4,1,WHITE,BLACK,12);	
			
			//根据角度判断选用哪一侧的长度数据
			if(angle > 45)
				LCD_ShowFloatNum1(82,100, c1_length,4,1,WHITE,BLACK,12);		//显示长度
			else
				LCD_ShowFloatNum1(82,100, c2_length,4,1,WHITE,BLACK,12);		//显示长度
			
			beep_flag = 1;		/*开始声音报警*/
			start_measure_flag = 0;
		}
		//达到最大时间，结束测量
		else if(HAL_GetTick() - measure_tick >= 28000)		//28s
		{
			Tx_udp(done_buf,3);				//发送结束指令
			HAL_Delay(12);
			Tx_udp(done_buf,3);				//发送结束指令(发送两次)
			
			//90度或0度的情况：长度测量完成，但角度无法正确测量
			if(ang_measure_done_flag == 0 && (c1_len_measure_done_flag == 1 || c2_len_measure_done_flag == 1))
			{
				
				int8_t delta_cnt = c1_len_rx_count - c2_len_rx_count;		//错误次数
				if(delta_cnt >= 2)
				{
					angle = 85;		//校准为为5度（覆盖0-10度）
				}
				else if(delta_cnt <= -2)
				{
					angle = 5;		//校准为85度（覆盖80-90度）
				}
				
				//显示角度
				LCD_ShowFloatNum1(18,100, angle,4,1,WHITE,BLACK,12);	
				
				//根据角度判断选用哪一侧的长度数据
				if(angle > 45)
					LCD_ShowFloatNum1(82,100, c1_length,4,1,WHITE,BLACK,12);		//显示长度
				else
					LCD_ShowFloatNum1(82,100, c2_length,4,1,WHITE,BLACK,12);		//显示长度		
				

			}
			//其他异常情况
			else if(ang_measure_done_flag == 1 && (c1_len_measure_done_flag == 0 || c2_len_measure_done_flag == 0))
			{
				LCD_ShowFloatNum1(82,100, c1_length,4,1,WHITE,BLACK,12);		//显示长度
				LCD_ShowFloatNum1(82,100, c2_length,4,1,WHITE,BLACK,12);		//显示长度
			}
//			else
//			{
//				//显示err
//				LCD_ShowString(18,100, "err",WHITE,BLACK,12,0);
//				LCD_ShowString(82,100, "err",WHITE,BLACK,12,0);		
//			}
			beep_flag = 1;		/*开始声音报警*/
			start_measure_flag = 0;
		}
		else
		{
			//显示计时
			if(line_cursor % 2 == 0)
				LCD_ShowIntNum(COLUMN0+36,112+(line_cursor/2)*12,(HAL_GetTick() - measure_tick)/1000,2,YELLOW,BLACK,12);
			else
				LCD_ShowIntNum(COLUMN1+36,112+(line_cursor/2)*12,(HAL_GetTick() - measure_tick)/1000,2,YELLOW,BLACK,12);
		}
	}
	
	if(beep_flag == 1)		/*声音报警*/
	{
		static uint8_t n = 0;
		if(HAL_GetTick() - beep_tick >= 200)	//间隔200ms
		{
			beep_tick = HAL_GetTick();
			HAL_GPIO_TogglePin(LED_BEEP_GPIO_Port,LED_BEEP_Pin);
			n++;
		}
		if(n == 6)		//响3次
		{
			n = 0;
			HAL_GPIO_WritePin(LED_BEEP_GPIO_Port,LED_BEEP_Pin,GPIO_PIN_SET);
			//关闭计时
//			if(line_cursor % 2 == 0)
//				LCD_Fill(COLUMN0+36,112+(line_cursor/2)*12,COLUMN0+36+18,112+((line_cursor/2)+1)*12,BLACK);	//在指定行标识白色色块
//			else
//				LCD_Fill(COLUMN1+36,112+(line_cursor/2)*12,COLUMN1+36+18,112+((line_cursor/2)+1)*12,BLACK);	//在指定行标识白色色块	
			beep_flag = 0;		//结束声音报警
			selected_flag = 0;
			
			if(line_cursor % 2 == 0)
				LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//在指定行标识红色色块
			else
				LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//在指定行标识红色色块
		}
	}
}

//进入示波器模式时，x1_max为节点1实时发送的x坐标
extern uint16_t x1_max;
extern uint16_t x2_max;
uint8_t anolog_mode = 0;	//模拟模式标志

//定义示波器窗口
osc_window osc_win;
uint32_t osc_x = 0;

//0.707f
void menu_osc(void)
{
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//重新初始化SPI1
	CS_pin=LCD2_CS_Pin;	//片选LCD2
	if(GUI.init_flag)
	{
		//发生界面切换
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//清屏
		
		GUI.init_flag = 0;
		osc_init(&osc_win,0,112,128,48,1);		//示波器初始化,模式1(手动幅值模式)
		osc_y_boundary_set(&osc_win,50,150);		//设置幅值范围:0-120
		anolog_mode = 1;	//进入模拟模式
		
		

	}

	osc_refresh(&osc_win,osc_x++,x1_max);	//刷新示波器串口（以屏幕像素点为单位）
	
	//画目标，坐标轴实际中心（100，160）
	
	int16_t x_aim = x2_max-160;
	int16_t y_aim = x1_max-100;
	int16_t z_aim = -20;		//定值
	
	static uint8_t screen_x = 50;
	static uint8_t screen_y = 50;
	
	//绘制坐标系：原点(50,50)  x轴终点(15,75) y轴终点(110,50) z轴终点(50,5)
	LCD_DrawLine(50,50,25,75,WHITE);	//x轴
	LCD_DrawLine(50,50,110,50,WHITE);	//y轴
	LCD_DrawLine(50,50,50,5,WHITE);		//z轴
	LCD_DrawLine(25,75,30,65,WHITE);	//x轴箭头
	LCD_DrawLine(25,75,35,75,WHITE);
	LCD_DrawLine(110,50,100,44,WHITE);	//y轴箭头
	LCD_DrawLine(110,50,100,56,WHITE);
	LCD_DrawLine(50,5,44,15,WHITE);		//z轴箭头
	LCD_DrawLine(50,5,56,15,WHITE);
	//清空上次目标
	LCD_Fill(screen_x-2,screen_y-2,screen_x+2,screen_y+2,BLACK);
	//清空上次摆线
	LCD_DrawLine(50,5,screen_x,screen_y,BLACK);
	//计算屏幕坐标
	screen_x = (y_aim-0.707f*x_aim)*0.6f + 50;
	screen_y = (z_aim-0.707f*x_aim)*0.6f + 50;
//	LCD_ShowFloatNum1(x_aim,y_aim,);
	//投影屏幕坐标（x未变）
	uint8_t screen_y_shadow = (-0.707f*x_aim)*0.6f + 50;
	
	LCD_Fill(screen_x-2,screen_y-2,screen_x+2,screen_y+2,RED);
	
	//绘制摆线
	LCD_DrawLine(50,5,screen_x,screen_y,YELLOW);
	
	//绘制投影
	LCD_Fill(screen_x-1,screen_y_shadow-1,screen_x+1,screen_y_shadow+1,BRRED);	
	
	if(EC11_getButtonFlag())
	{
		Tx_udp(Osc_off_buf,3);	//发送关闭示波器指令
		HAL_Delay(12);
		Tx_udp(Osc_off_buf,3);	//发送关闭示波器指令
		
		anolog_mode = 0;		//退出模拟模式
		menuReturn(&GUI);	//返回
	}
}

