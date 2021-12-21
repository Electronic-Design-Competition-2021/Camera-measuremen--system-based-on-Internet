#ifndef __BSP_LCD_H__
#define __BSP_LCD_H__

#include "main.h"
#include "bsp_gpio.h"
#include "bsp_delay.h"

//Ĭ��Ϊ0.96��IPS������ST7735S�����������º궨���ʹ��1.8�����
//#define ST7735S_1_8

#define USE_HORIZONTAL 2  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����

#ifdef ST7735S_1_8		//1.8��LCD
	#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
	#define LCD_W 128
	#define LCD_H 160

	#else
	#define LCD_W 160
	#define LCD_H 128
	#endif
#else			//0.96��LCD
	#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
		#define LCD_W 80
		#define LCD_H 160

	#else

		#define LCD_W 160
		#define LCD_H 80

	#endif
#endif


//-----------------LCD�˿ڶ���---------------- 

#define LCD_RES_Clr()  User_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_RESET)	//RES
#define LCD_RES_Set()  User_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_SET)

#define LCD_DC_Clr()   User_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET)	//DC
#define LCD_DC_Set()   User_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   User_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET)	//CS
#define LCD_CS_Set()   User_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET)

#define LCD_BLK_Clr()  User_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_RESET)	//BLK
#define LCD_BLK_Set()  User_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_SET)




void LCD_GPIO_Init(void);//��ʼ��GPIO
void LCD_Writ_Bus(uint8_t dat);//ģ��SPIʱ��
void LCD_WR_DATA8(uint8_t dat);//д��һ���ֽ�
void LCD_WR_DATA(uint16_t dat);//д�������ֽ�
void LCD_WR_REG(uint8_t dat);//д��һ��ָ��
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//�������꺯��
void LCD_Init(void);	//LCD��ʼ��
void SPI1_LCD_ReInit(uint8_t SPI_BaudRatePrescaler);	//LCD SPI ���ò�����ʼ��
#endif



