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
#include "bluetooth.h"
#include "menu.h"
#include "Oscilloscope.h"
#include "bsp_ec11.h"
#include "bsp_flash.h"
//#include "bsp_24l01.h"
#include "bsp_usart.h"
#include "usart.h"
#include "openmv.h"
#include "angle_length.h"


menu_s_t GUI;	//定义多级菜单

uint8_t lcdBuf[64];
uint8_t txxbuf[6]={11,22,33,44,55,66};
extern uint16_t write_data[FLASH_DATA_LENGHT];
extern uint16_t after_write_data[FLASH_DATA_LENGHT];
extern uint8_t	 blob_y;//色块中心y坐标
extern unsigned char RXBUF[6];
extern uint8_t y;//解算在y轴的长度
extern uint16_t bolb_y_low;
// 切换选中行
// shift_flag  0：向下  1：向上
void menu_Shift_Line(uint8_t shift_flag)
{
	if(shift_flag == 0)
	{
		
	}
	else
	{
		
	}
}


const uint8_t LineTitle[LINE_NUM][64] = {

	"Lmin:",				//参数2
	"Lmax:",
	"Amin:",	
	"Amax:",
	"Bmin:",
	"Bmax:",
};


uint8_t buff[64];
uint8_t Configs[6];	//参数数组
int8_t line_cursor;		//行光标
int8_t screen_flag;		//菜单标号
int8_t back_flag;
extern uint8_t first_flag;
//uint8_t selected_flag=0;

extern uint8_t debug_mode_flag;	//视频调试标志位
extern uint8_t spi2_rx_Flag;	//spi2接收完成标志位

uint8_t openmv_mode_flag = 0;		//openmv显示模式标志位


//主界面（初始界面）
void menu_main(void)
{
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//重新初始化SPI1	
	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11编码器数值
	if(GUI.init_flag)
	{
		//发生界面切换,进行页面初始化
		GUI.init_flag = 0;	//标志位清零
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色:黑色
		
		//显示每一行的标题
		LCD_ShowString(COLUMN0,0,"Start UDP",GREEN,BLACK,16,0);	//开启UDP传输
		LCD_ShowString(COLUMN0,16,"OPENMV Configure",GREEN,BLACK,16,0);	//显示每一行的标题
		if(openmv_mode_flag == 1)
			LCD_ShowString(COLUMN0,32,"OV:To Color Mode",GREEN,BLACK,16,0);	//显示每一行的标题
		else
			LCD_ShowString(COLUMN0,32,"OV:To Bin Mode  ",GREEN,BLACK,16,0);	//显示每一行的标题
		
		LCD_ShowString(COLUMN0,48,"Debug Video",GREEN,BLACK,16,0);	//显示每一行的标题
		
		ec11_count = EC11_getCount();	//获取EC11计数
		last_ec11_count = ec11_count;	//记录初始EC11计数
		line_cursor = 0;				//初始光标归零
		LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//在指定行标识红色色块
	}
	
	ec11_count = EC11_getCount();	//获取EC11计数
	if(ec11_count != last_ec11_count)
	{
		if(ec11_count > last_ec11_count)
			line_cursor += 1;
		else
			line_cursor -= 1;
		line_cursor = line_cursor > 3 ? 3 : line_cursor;		//移动到头
		line_cursor = line_cursor < 0 ? 0 : line_cursor;		//	
		LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);							//清空标识
		LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//在指定行标识红色色块
	}
	last_ec11_count = ec11_count;
	if(EC11_getButtonFlag())	//按键被按下
	{
		if(line_cursor == 0)
			menuSkip(&GUI,menu_StartUDP);	//跳转至menuStartUDP
		if(line_cursor == 1)
			menuSkip(&GUI,menu_OPENMV_Conig);	//跳转至menu_OPENMV_Conig
		if(line_cursor == 2)
		{
			openmv_mode_flag = !openmv_mode_flag;
			if(openmv_mode_flag)
			{
				OPENMV_SetDispMode(OPENMV_MODE_BIN);
				LCD_ShowString(COLUMN0,32,"OV:To Color Mode",GREEN,BLACK,16,0);
			}
			else
			{
				OPENMV_SetDispMode(OPENMV_MODE_COL);
				LCD_ShowString(COLUMN0,32,"OV:To Bin Mode  ",GREEN,BLACK,16,0);
			}
		}
		
		if(line_cursor == 3)
		{
			debug_mode_flag = 1;
			menuSkip(&GUI,menu_DebugVideo);	//跳转至menu_DebugVideo
		}
	}
}

extern uint8_t img_Buf[26000];

// 拍摄节点查看抓取的图像（调试用）
void menu_DebugVideo(void)
{
	if(GUI.init_flag)
	{
		GUI.init_flag=0;
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色:黑色
		LCD_ShowString(COLUMN0,16,"NO Video",GREEN,BLACK,16,0);
		LCD_ShowString(COLUMN0,32,"Please Reset",GREEN,BLACK,16,0);
	}
	if(spi2_rx_Flag)
	{
		spi2_rx_Flag = 0;
		//数据接收完成
		LCD_ShowPicture(0,0,128,80,img_Buf);	//显示接收到的视频
		//HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED闪烁
	}		
	if(EC11_getButtonFlag())
	{
		debug_mode_flag = 0;	//按下按键退出图像调试
		menuReturn(&GUI);		//返回上一页
		return;
	}
}

extern uint8_t udp_start_flag;	//图像发送开启标志位
//UDP传输菜单
void menu_StartUDP(void)
{
	if(GUI.init_flag)
	{
		//发生界面切换,进行页面初始化
		GUI.init_flag = 0;	//标志位清零
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色:黑色
		
		//显示每一行的标题
		LCD_ShowString(COLUMN0,16,"UDP Transmitting",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN0,32,"Press to exit",GREEN,BLACK,16,0);	//显示每一行的标题
		udp_start_flag = 1;		//开启UDO传输
	}	
	if(EC11_getButtonFlag())
	{
		udp_start_flag = 0;		//按下按键退出图像传输
		menuReturn(&GUI);		//返回上一页
		return;
	}
}


extern uint8_t measure_flag;
//开始测量菜单
void menu_StartMeasure(void)
{
	if(GUI.init_flag)
	{
		//发生界面切换,进行页面初始化
		GUI.init_flag = 0;	//标志位清零
		LCD_Fill(0,0,LCD_W,LCD_H,GREEN);	//初始颜色:绿色
		
		//显示每一行的标题
		LCD_ShowString(COLUMN0,16,"Measuring",BLACK,GREEN,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN0,32,"Press to exit",BLACK,GREEN,16,0);	//显示每一行的标题
		udp_start_flag = 1;		//开启UDP传输
		measure_flag = 1;		//开启测量
	}
	if(EC11_getButtonFlag())
	{
		//手动退出测量
		measure_flag = 0;		//结束测量
		menuReturn(&GUI);		//返回上一页
		return;
	}
}

//OPENMV参数调节菜单
void menu_OPENMV_Conig(void)
{
	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11编码器数值
	static uint8_t selected_flag = 0 ;	//行选中
	
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//重新初始化SPI1	
	
	if(GUI.init_flag)
	{
		//发生界面切换,进行页面初始化
		GUI.init_flag = 0;	//标志位清零
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色:黑色
		line_cursor = 0;
		LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,RED);	//在指定行标识红色色块
		
		//显示每一行的标题
		LCD_ShowString(COLUMN0,0,"Lmin:",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN1,0,"Lmax:",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN0,16,"Amin:",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN1,16,"Amax:",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN0,32,"Bmin:",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN1,32,"Bmax:",GREEN,BLACK,16,0);	//显示每一行的标题
		LCD_ShowString(COLUMN0,48,"Save",GREEN,BLACK,16,0);		//显示每一行的标题
		LCD_ShowString(COLUMN1,48,"Back",GREEN,BLACK,16,0);		//显示每一行的标题
		//显示每行数据
		LCD_ShowIntNum(COLUMN0+DATA_BIAS,0,Configs[0],3,GREEN,BLACK,16);	//显示每行数据
		LCD_ShowIntNum(COLUMN1+DATA_BIAS,0,Configs[1],3,GREEN,BLACK,16);	//显示每行数据
		LCD_ShowIntNum(COLUMN0+DATA_BIAS,16,Configs[2],3,GREEN,BLACK,16);	//显示每行数据
		LCD_ShowIntNum(COLUMN1+DATA_BIAS,16,Configs[3],3,GREEN,BLACK,16);	//显示每行数据	
		LCD_ShowIntNum(COLUMN0+DATA_BIAS,32,Configs[4],3,GREEN,BLACK,16);	//显示每行数据
		LCD_ShowIntNum(COLUMN1+DATA_BIAS,32,Configs[5],3,GREEN,BLACK,16);	//显示每行数据
		
	}
	ec11_count = EC11_getCount();	//获取EC11计数
	
	if(ec11_count != last_ec11_count)	//编码器旋转
	{
		if(selected_flag == 0) //未选中行
		{
			if(ec11_count > last_ec11_count)
				line_cursor += 1;
			else
				line_cursor -= 1;
			//line_cursor += ec11_count - last_ec11_count;	//光标移动
			line_cursor = line_cursor > 7 ? 7 : line_cursor;	//移动到头
			line_cursor = line_cursor < 0 ? 0: line_cursor;		//移动到头
			
			//清空色块
			LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);
			LCD_Fill(80,0,COLUMN1,LCD_H,BLACK);
			
			//标记色块
			if(line_cursor % 2 == 0)
				LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,RED);	//在指定行标识红色色块
			else
				LCD_Fill(80,(line_cursor/2)*16,COLUMN1,((line_cursor/2)+1)*16,RED);	//在指定行标识红色色块
		}
		if(selected_flag == 1)		// 已选中行
		{
			if(line_cursor < 6)
			{
				//对选中行的参数进行调节
				switch (line_cursor)//精度调节
				{
					case 0:  Configs[0] += (ec11_count - last_ec11_count)*1;break;
					case 1:  Configs[1] += (ec11_count - last_ec11_count)*1;break;
					case 2:  Configs[2] += (ec11_count - last_ec11_count)*1;break;
					case 3:  Configs[3] += (ec11_count - last_ec11_count)*1;break;
					case 4:  Configs[4] += (ec11_count - last_ec11_count)*1;break;
					case 5:  Configs[5] += (ec11_count - last_ec11_count)*1;break;
					default :break;
				}
				Configs[line_cursor] = Configs[line_cursor] >= 255 ? 0 : Configs[line_cursor];	//限幅
				
				if(line_cursor % 2 == 0)
					LCD_ShowIntNum(COLUMN0+DATA_BIAS,(line_cursor/2)*16,Configs[line_cursor],3,GREEN,BLACK,16);			//显示数据
				else
					LCD_ShowIntNum(COLUMN1+DATA_BIAS,(line_cursor/2)*16,Configs[line_cursor],3,GREEN,BLACK,16);			//显示
			}
		}
	}	
	last_ec11_count = ec11_count;
	
	if(EC11_getButtonFlag())	//按键被按下
	{
		if(line_cursor == 6)
		{
			//保存
			flash_erase_address(USER_FLASH_ADDRESS, 1);//擦除扇区
			flash_write_single_address(USER_FLASH_ADDRESS, (uint32_t *)Configs, (FLASH_DATA_LENGHT + 3) / 4);//写flash
			OpenMV_SendLabThr(Configs);	//向OPENMV发送参数
			return;
		}
		if(line_cursor == 7)
		{
			//返回
			menuReturn(&GUI);
			return;
		}
		selected_flag=!selected_flag;	//行选中 or 未选中
		if(selected_flag)
		{
			if(line_cursor % 2 == 0)
				LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,WHITE);	//在指定行标识红色色块
			else
				LCD_Fill(80,(line_cursor/2)*16,COLUMN1,((line_cursor/2)+1)*16,WHITE);	//在指定行标识红色色块
		}
		else
		{
			if(line_cursor % 2 == 0)
				LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,RED);	//在指定行标识红色色块
			else
				LCD_Fill(80,(line_cursor/2)*16,COLUMN1,((line_cursor/2)+1)*16,RED);	//在指定行标识红色色块	
		}				
	}	
}




//void menu1(void)
//{
//	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11编码器数值
//	static uint8_t selected_flag = 0 ;	//行选中
//	uint8_t i = 0;
//	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//重新初始化SPI1	
//	if(GUI.init_flag)
//	{
//		//发生界面切换
//		GUI.init_flag = 0;	//标志位清零
//		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色:黑色
//	//	LCD_ShowString(COLUMN0,0,"GUI DEMO - DOEE",GREEN,BLACK,16,0);	//首行页面标题
//      if(screen_flag==1)
//			{
//				 for(i = 0; i < 4; i ++)
//		   {
//			  LCD_ShowString(COLUMN0,16*(i),LineTitle[i],GREEN,BLACK,16,0);	//显示每一行的标题
//			  LCD_ShowIntNum(COLUMN1,16*(i),Configs[i],4,GREEN,BLACK,16);	//显示每行数据
//			 }
//			 LCD_ShowString(COLUMN0,4*16,"BACK",GREEN,BLACK,16,0);	//末行：跳转至menu2(示波器界面)
//	  	}
//			if(screen_flag==2)
//			{
//				 for(i = 4; i < LINE_NUM; i ++)
//		   {
//			  LCD_ShowString(COLUMN0,16*(i-4),LineTitle[i],GREEN,BLACK,16,0);	//显示每一行的标题
//			  LCD_ShowIntNum(COLUMN1,16*(i-4),Configs[i],4,GREEN,BLACK,16);	//显示每行数据
//		 	 }
//			 LCD_ShowString(COLUMN0,(LINE_NUM-4)*16,"BACK",GREEN,BLACK,16,0);	//末行：跳转至menu2(示波器界面)
//	  	}
//		
//		ec11_count = EC11_getCount();	//获取EC11计数
//		last_ec11_count = ec11_count;	//
//		line_cursor = 0;				//初始光标归零
//	}
//	if(screen_flag==1)//界面1
//	{
//		if(line_cursor==4)//若行光标在返回行
//			 back_flag=1;//返回标志位置1
//		else
//			back_flag=0;
//	}
//	if(screen_flag==2)//界面2
//	{
//		if(line_cursor==2)//若行光标在返回行
//		 {
//			 back_flag=1;
//		 }
//		 else
//			 back_flag=0;
//	}
//	if(EC11_getButtonFlag())	//按键被按下
//	{
//		if(back_flag==1)//返回标志位置1
//		{
//			menuSkip(&GUI,menu4);	//跳转至menu4
//			
//		}
//		if(line_cursor==LINE_NUM)//返回标志位置1
//		{	
//			menuSkip(&GUI,menu4);	//跳转至menu4
//		}
//		else
//		{
//			selected_flag=!selected_flag;	//行选中 or 未选中  or  存数据 切换
//			//if(selected_flag>2)selected_flag=0;
//			if(selected_flag==1) 
//			{
//				LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,WHITE);	//选中为白色
//			}
//			if(selected_flag==0)
//				LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//未选中为红色
//		}
//	}
//	ec11_count = EC11_getCount();	//获取EC11计数
//	
//	if(ec11_count != last_ec11_count)
//	{
//		if(selected_flag == 0) //未选中行
//		{
//			line_cursor += ec11_count - last_ec11_count;
//			
//			if(screen_flag == 1)//界面1
//			{
//			//未选中行，旋转编码器进行 行选择
//			line_cursor = line_cursor > 4 ? 0 : line_cursor;		//最后一行跳转到第一行
//			line_cursor = line_cursor < 0 ? 4 : line_cursor;	//第一行跳转至最后一行
//			}
//			if(screen_flag == 2)//界面2
//			{
//			//未选中行，旋转编码器进行 行选择
//			line_cursor = line_cursor > 2 ? 0 : line_cursor;		//最后一行跳转到第一行
//			line_cursor = line_cursor < 0 ? 2 : line_cursor;	//第一行跳转至最后一行
//			}
//			LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);							//清空标识
//			LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//在指定行标识红色色块
//			if(screen_flag == 3)
//			{
//			line_cursor = line_cursor > 2 ? 0 : line_cursor;		//最后一行跳转到第一行
//			line_cursor = line_cursor < 0 ? 2 : line_cursor;	//第一行跳转至最后一行
//			}
//		}
//		else if(selected_flag == 1)		// 已选中行
//		{
//	
//				if(screen_flag==1)//界面1
//			{
//			//对选中行的参数进行调节
//				switch (line_cursor)//精度调节
//				{
//					case 0:  Configs[0] += (ec11_count - last_ec11_count)*1;break;
//					case 1:  Configs[1] += (ec11_count - last_ec11_count)*1;break;
//					case 2:  Configs[2] += (ec11_count - last_ec11_count)*1;break;
//					case 3:  Configs[3] += (ec11_count - last_ec11_count)*1;break;
//					default :break;
//				}
//			    Configs[line_cursor] = Configs[line_cursor] >= 255 ? 0 : Configs[line_cursor];	//限幅
//			    LCD_ShowIntNum(COLUMN1,(line_cursor)*16,Configs[line_cursor],4,GREEN,BLACK,16);			//显示数据
//			}
//			  if(screen_flag==2)//界面2
//			{
//			//对选中行的参数进行调节
//				switch (line_cursor+4)
//				{
//					case 4:  Configs[4] += (ec11_count - last_ec11_count)*1;break;
//					case 5:  Configs[5] += (ec11_count - last_ec11_count)*1;break;
//					default :break;
//				}
//			Configs[line_cursor+4] = Configs[line_cursor+4] >= 255 ? 0 : Configs[line_cursor+4];	//限幅
//			LCD_ShowIntNum(COLUMN1,(line_cursor)*16,Configs[line_cursor+4],4,GREEN,BLACK,16);			//显示数据
//			}
//		}
//		
//	}
//	//LCD_ShowFloatNum1(95,(6+1)*16,ec11_count,4,1,GREEN,BLACK,16);			//显示数据
//	last_ec11_count = ec11_count;
//	
//}

//void menu4(void)
//{
//	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11编码器数值
//	
////	static uint8_t selected_flag = 0 ;	//行选中
//	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//重新初始化SPI1
//	if(GUI.init_flag)
//	{
//		//发生界面切换
//		GUI.init_flag = 0;
//		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//初始颜色
//		ec11_count = EC11_getCount();	//获取EC11计数
//		last_ec11_count = ec11_count;	//
//		line_cursor = 0;				//初始光标归零
//	}
//	LCD_ShowString(10,0,"config_L_A",WHITE,BLACK,16,0);	//显示接收到的数据
//	LCD_ShowString(10,20,"config_B",WHITE,BLACK,16,0);	//显示接收到的数据
//	LCD_ShowString(10,40,"save",WHITE,BLACK,16,0);	//显示接收到的数据
//	LCD_ShowString(10,60,"bin",WHITE,BLACK,16,0);	//显示接收到的数据
//	ec11_count = EC11_getCount();	//获取EC11计数
//	//LCD_ShowFloatNum1(10,0,ec11_count,4,1,GREEN,BLACK,16);			//显示数据
//	if(ec11_count != last_ec11_count)
//	{
//			//未选中行，旋转编码器进行 行选择
//			line_cursor += ec11_count - last_ec11_count;
//			line_cursor = line_cursor > 4 ? 1 : line_cursor;		//最后一行跳转到第一行
//			line_cursor = line_cursor <=0 ? 4 : line_cursor;	//第一行跳转至最后一行
//			LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);							//清空标识
//			LCD_Fill(0,(line_cursor-1)*20,COLUMN0,line_cursor*20,RED);	//在指定行标识红色色块
//	}
//	last_ec11_count = ec11_count;
//	if(EC11_getButtonFlag())
//	{
//		if(line_cursor == 1)
//		{
//			screen_flag=1;
//		//	selected_flag=0;
//			menuSkip(&GUI,menu1);	//跳转至menu1
//		}
//		if(line_cursor == 2)
//		{
//			screen_flag=2;
//			//selected_flag=0;
//			menuSkip(&GUI,menu1);	//跳转至menu1
//		}
//		if(line_cursor == 3)
//		{
//			LCD_Fill(0,(line_cursor-1)*20,COLUMN0,(line_cursor)*20,GREEN);	//在指定行标识绿色色块
//			flash_erase_address(USER_FLASH_ADDRESS, 1);//擦除扇区

//			flash_write_single_address(USER_FLASH_ADDRESS, (uint32_t *)Configs, (FLASH_DATA_LENGHT + 3) / 4);//写flash
////		    	usart2_SendData(Configs,6);
////			   User_Delay(500);
//			//sprintf((char*)buff,"%3d",Configs[0]);
//			OpenMV_SendLabThr(Configs);
//		//	printf("%3d",Configs[0]);
//			 // send_data(Configs);
//	//	LCD_ShowIntNum(50,60,blob_y,4,GREEN,BLACK,16);			//显示数据
//			
//			//bt_SendData(Configs[0],Configs[1],Configs[2],Configs[3],Configs[4],Configs[5]);
//		}
//		if(line_cursor == 4)
//		{
//			static uint8_t openmv_disp_flag = 0;
//			//LCD_Fill(0,(line_cursor-1)*20,COLUMN0,(line_cursor)*20,GREEN);	//在指定行标识绿色色块
//			openmv_disp_flag = !openmv_disp_flag;
//			if(openmv_disp_flag)
//				OPENMV_SetDispMode(OPENMV_MODE_BIN);
//			else
//				OPENMV_SetDispMode(OPENMV_MODE_COL);
//			
//		}
//	}

//	
////	LCD_ShowFloatNum1(50,60,Angle_calc(),4,GREEN,BLACK,16);			//显示数据
//	//LCD_ShowFloatNum1(70,60,length_calc(bolb_x_low,bolb_y_low,bolb_x_high,bolb_y_high),4,GREEN,BLACK,16);			//显示数据
////	LCD_ShowFloatNum1(50,60,Lengh_calc(),4,GREEN,BLACK,16);			//显示数据
//	
////	printf("%2d\n",RXBUF[0]);
//}



