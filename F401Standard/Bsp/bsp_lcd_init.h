#ifndef __BSP_LCD_H__
#define __BSP_LCD_H__

#include "main.h"
#include "bsp_gpio.h"
#include "bsp_delay.h"

//默认为0.96寸IPS彩屏（ST7735S），开启以下宏定义后，使用1.8寸彩屏
#define ST7735S_1_8

#define USE_HORIZONTAL 1  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#ifdef ST7735S_1_8		//1.8寸LCD
	#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
	#define LCD_W 128
	#define LCD_H 160

	#else
	#define LCD_W 160
	#define LCD_H 128
	#endif
#else			//0.96寸LCD
	#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
		#define LCD_W 80
		#define LCD_H 160

	#else

		#define LCD_W 160
		#define LCD_H 80

	#endif
#endif



//-----------------LCD端口定义---------------- 

#define LCD_RES_Clr()  User_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_RESET)	//RES
#define LCD_RES_Set()  User_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_SET)

#define LCD_DC_Clr()   User_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET)	//DC
#define LCD_DC_Set()   User_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   User_GPIO_WritePin(CS_port,CS_pin,GPIO_PIN_RESET)	//CS
#define LCD_CS_Set()   User_GPIO_WritePin(CS_port,CS_pin,GPIO_PIN_SET)

#define LCD_BLK_Clr()  User_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_RESET)	//BLK
#define LCD_BLK_Set()  User_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_SET)


extern GPIO_TypeDef* CS_port;
extern uint16_t CS_pin;

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);	//LCD初始化
void SPI1_LCD_ReInit(uint8_t SPI_BaudRatePrescaler);	//LCD SPI 配置参数初始化
#endif




