/**
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  * @file      	appMenu.c/h
  * @brief     	�༶�˵����û��㣩
  * @note      	ÿ���˵�������һ��������������ʽ�����ã��½��˵������½�һ��������
				ͨ������menuSkipʵ�ֲ˵���ת����غ�����menu.c/h
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     July-1-2021     doee            1. ���
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


menu_s_t GUI;	//����༶�˵�

uint8_t lcdBuf[64];
uint8_t txxbuf[6]={11,22,33,44,55,66};
extern uint16_t write_data[FLASH_DATA_LENGHT];
extern uint16_t after_write_data[FLASH_DATA_LENGHT];
extern uint8_t	 blob_y;//ɫ������y����
extern unsigned char RXBUF[6];
extern uint8_t y;//������y��ĳ���
extern uint16_t bolb_y_low;
// �л�ѡ����
// shift_flag  0������  1������
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

	"Lmin:",				//����2
	"Lmax:",
	"Amin:",	
	"Amax:",
	"Bmin:",
	"Bmax:",
};


uint8_t buff[64];
uint8_t Configs[6];	//��������
int8_t line_cursor;		//�й��
int8_t screen_flag;		//�˵����
int8_t back_flag;
extern uint8_t first_flag;
//uint8_t selected_flag=0;

extern uint8_t debug_mode_flag;	//��Ƶ���Ա�־λ
extern uint8_t spi2_rx_Flag;	//spi2������ɱ�־λ

uint8_t openmv_mode_flag = 0;		//openmv��ʾģʽ��־λ


//�����棨��ʼ���棩
void menu_main(void)
{
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//���³�ʼ��SPI1	
	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11��������ֵ
	if(GUI.init_flag)
	{
		//���������л�,����ҳ���ʼ��
		GUI.init_flag = 0;	//��־λ����
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ:��ɫ
		
		//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,0,"Start UDP",GREEN,BLACK,16,0);	//����UDP����
		LCD_ShowString(COLUMN0,16,"OPENMV Configure",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		if(openmv_mode_flag == 1)
			LCD_ShowString(COLUMN0,32,"OV:To Color Mode",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		else
			LCD_ShowString(COLUMN0,32,"OV:To Bin Mode  ",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		
		LCD_ShowString(COLUMN0,48,"Debug Video",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		
		ec11_count = EC11_getCount();	//��ȡEC11����
		last_ec11_count = ec11_count;	//��¼��ʼEC11����
		line_cursor = 0;				//��ʼ������
		LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
	}
	
	ec11_count = EC11_getCount();	//��ȡEC11����
	if(ec11_count != last_ec11_count)
	{
		if(ec11_count > last_ec11_count)
			line_cursor += 1;
		else
			line_cursor -= 1;
		line_cursor = line_cursor > 3 ? 3 : line_cursor;		//�ƶ���ͷ
		line_cursor = line_cursor < 0 ? 0 : line_cursor;		//	
		LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);							//��ձ�ʶ
		LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
	}
	last_ec11_count = ec11_count;
	if(EC11_getButtonFlag())	//����������
	{
		if(line_cursor == 0)
			menuSkip(&GUI,menu_StartUDP);	//��ת��menuStartUDP
		if(line_cursor == 1)
			menuSkip(&GUI,menu_OPENMV_Conig);	//��ת��menu_OPENMV_Conig
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
			menuSkip(&GUI,menu_DebugVideo);	//��ת��menu_DebugVideo
		}
	}
}

extern uint8_t img_Buf[26000];

// ����ڵ�鿴ץȡ��ͼ�񣨵����ã�
void menu_DebugVideo(void)
{
	if(GUI.init_flag)
	{
		GUI.init_flag=0;
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ:��ɫ
		LCD_ShowString(COLUMN0,16,"NO Video",GREEN,BLACK,16,0);
		LCD_ShowString(COLUMN0,32,"Please Reset",GREEN,BLACK,16,0);
	}
	if(spi2_rx_Flag)
	{
		spi2_rx_Flag = 0;
		//���ݽ������
		LCD_ShowPicture(0,0,128,80,img_Buf);	//��ʾ���յ�����Ƶ
		//HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED��˸
	}		
	if(EC11_getButtonFlag())
	{
		debug_mode_flag = 0;	//���°����˳�ͼ�����
		menuReturn(&GUI);		//������һҳ
		return;
	}
}

extern uint8_t udp_start_flag;	//ͼ���Ϳ�����־λ
//UDP����˵�
void menu_StartUDP(void)
{
	if(GUI.init_flag)
	{
		//���������л�,����ҳ���ʼ��
		GUI.init_flag = 0;	//��־λ����
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ:��ɫ
		
		//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,16,"UDP Transmitting",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,32,"Press to exit",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		udp_start_flag = 1;		//����UDO����
	}	
	if(EC11_getButtonFlag())
	{
		udp_start_flag = 0;		//���°����˳�ͼ����
		menuReturn(&GUI);		//������һҳ
		return;
	}
}


extern uint8_t measure_flag;
//��ʼ�����˵�
void menu_StartMeasure(void)
{
	if(GUI.init_flag)
	{
		//���������л�,����ҳ���ʼ��
		GUI.init_flag = 0;	//��־λ����
		LCD_Fill(0,0,LCD_W,LCD_H,GREEN);	//��ʼ��ɫ:��ɫ
		
		//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,16,"Measuring",BLACK,GREEN,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,32,"Press to exit",BLACK,GREEN,16,0);	//��ʾÿһ�еı���
		udp_start_flag = 1;		//����UDP����
		measure_flag = 1;		//��������
	}
	if(EC11_getButtonFlag())
	{
		//�ֶ��˳�����
		measure_flag = 0;		//��������
		menuReturn(&GUI);		//������һҳ
		return;
	}
}

//OPENMV�������ڲ˵�
void menu_OPENMV_Conig(void)
{
	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11��������ֵ
	static uint8_t selected_flag = 0 ;	//��ѡ��
	
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//���³�ʼ��SPI1	
	
	if(GUI.init_flag)
	{
		//���������л�,����ҳ���ʼ��
		GUI.init_flag = 0;	//��־λ����
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ:��ɫ
		line_cursor = 0;
		LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
		
		//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,0,"Lmin:",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN1,0,"Lmax:",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,16,"Amin:",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN1,16,"Amax:",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,32,"Bmin:",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN1,32,"Bmax:",GREEN,BLACK,16,0);	//��ʾÿһ�еı���
		LCD_ShowString(COLUMN0,48,"Save",GREEN,BLACK,16,0);		//��ʾÿһ�еı���
		LCD_ShowString(COLUMN1,48,"Back",GREEN,BLACK,16,0);		//��ʾÿһ�еı���
		//��ʾÿ������
		LCD_ShowIntNum(COLUMN0+DATA_BIAS,0,Configs[0],3,GREEN,BLACK,16);	//��ʾÿ������
		LCD_ShowIntNum(COLUMN1+DATA_BIAS,0,Configs[1],3,GREEN,BLACK,16);	//��ʾÿ������
		LCD_ShowIntNum(COLUMN0+DATA_BIAS,16,Configs[2],3,GREEN,BLACK,16);	//��ʾÿ������
		LCD_ShowIntNum(COLUMN1+DATA_BIAS,16,Configs[3],3,GREEN,BLACK,16);	//��ʾÿ������	
		LCD_ShowIntNum(COLUMN0+DATA_BIAS,32,Configs[4],3,GREEN,BLACK,16);	//��ʾÿ������
		LCD_ShowIntNum(COLUMN1+DATA_BIAS,32,Configs[5],3,GREEN,BLACK,16);	//��ʾÿ������
		
	}
	ec11_count = EC11_getCount();	//��ȡEC11����
	
	if(ec11_count != last_ec11_count)	//��������ת
	{
		if(selected_flag == 0) //δѡ����
		{
			if(ec11_count > last_ec11_count)
				line_cursor += 1;
			else
				line_cursor -= 1;
			//line_cursor += ec11_count - last_ec11_count;	//����ƶ�
			line_cursor = line_cursor > 7 ? 7 : line_cursor;	//�ƶ���ͷ
			line_cursor = line_cursor < 0 ? 0: line_cursor;		//�ƶ���ͷ
			
			//���ɫ��
			LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);
			LCD_Fill(80,0,COLUMN1,LCD_H,BLACK);
			
			//���ɫ��
			if(line_cursor % 2 == 0)
				LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
			else
				LCD_Fill(80,(line_cursor/2)*16,COLUMN1,((line_cursor/2)+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
		}
		if(selected_flag == 1)		// ��ѡ����
		{
			if(line_cursor < 6)
			{
				//��ѡ���еĲ������е���
				switch (line_cursor)//���ȵ���
				{
					case 0:  Configs[0] += (ec11_count - last_ec11_count)*1;break;
					case 1:  Configs[1] += (ec11_count - last_ec11_count)*1;break;
					case 2:  Configs[2] += (ec11_count - last_ec11_count)*1;break;
					case 3:  Configs[3] += (ec11_count - last_ec11_count)*1;break;
					case 4:  Configs[4] += (ec11_count - last_ec11_count)*1;break;
					case 5:  Configs[5] += (ec11_count - last_ec11_count)*1;break;
					default :break;
				}
				Configs[line_cursor] = Configs[line_cursor] >= 255 ? 0 : Configs[line_cursor];	//�޷�
				
				if(line_cursor % 2 == 0)
					LCD_ShowIntNum(COLUMN0+DATA_BIAS,(line_cursor/2)*16,Configs[line_cursor],3,GREEN,BLACK,16);			//��ʾ����
				else
					LCD_ShowIntNum(COLUMN1+DATA_BIAS,(line_cursor/2)*16,Configs[line_cursor],3,GREEN,BLACK,16);			//��ʾ
			}
		}
	}	
	last_ec11_count = ec11_count;
	
	if(EC11_getButtonFlag())	//����������
	{
		if(line_cursor == 6)
		{
			//����
			flash_erase_address(USER_FLASH_ADDRESS, 1);//��������
			flash_write_single_address(USER_FLASH_ADDRESS, (uint32_t *)Configs, (FLASH_DATA_LENGHT + 3) / 4);//дflash
			OpenMV_SendLabThr(Configs);	//��OPENMV���Ͳ���
			return;
		}
		if(line_cursor == 7)
		{
			//����
			menuReturn(&GUI);
			return;
		}
		selected_flag=!selected_flag;	//��ѡ�� or δѡ��
		if(selected_flag)
		{
			if(line_cursor % 2 == 0)
				LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,WHITE);	//��ָ���б�ʶ��ɫɫ��
			else
				LCD_Fill(80,(line_cursor/2)*16,COLUMN1,((line_cursor/2)+1)*16,WHITE);	//��ָ���б�ʶ��ɫɫ��
		}
		else
		{
			if(line_cursor % 2 == 0)
				LCD_Fill(0,(line_cursor/2)*16,COLUMN0,((line_cursor/2)+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
			else
				LCD_Fill(80,(line_cursor/2)*16,COLUMN1,((line_cursor/2)+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��	
		}				
	}	
}




//void menu1(void)
//{
//	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11��������ֵ
//	static uint8_t selected_flag = 0 ;	//��ѡ��
//	uint8_t i = 0;
//	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//���³�ʼ��SPI1	
//	if(GUI.init_flag)
//	{
//		//���������л�
//		GUI.init_flag = 0;	//��־λ����
//		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ:��ɫ
//	//	LCD_ShowString(COLUMN0,0,"GUI DEMO - DOEE",GREEN,BLACK,16,0);	//����ҳ�����
//      if(screen_flag==1)
//			{
//				 for(i = 0; i < 4; i ++)
//		   {
//			  LCD_ShowString(COLUMN0,16*(i),LineTitle[i],GREEN,BLACK,16,0);	//��ʾÿһ�еı���
//			  LCD_ShowIntNum(COLUMN1,16*(i),Configs[i],4,GREEN,BLACK,16);	//��ʾÿ������
//			 }
//			 LCD_ShowString(COLUMN0,4*16,"BACK",GREEN,BLACK,16,0);	//ĩ�У���ת��menu2(ʾ��������)
//	  	}
//			if(screen_flag==2)
//			{
//				 for(i = 4; i < LINE_NUM; i ++)
//		   {
//			  LCD_ShowString(COLUMN0,16*(i-4),LineTitle[i],GREEN,BLACK,16,0);	//��ʾÿһ�еı���
//			  LCD_ShowIntNum(COLUMN1,16*(i-4),Configs[i],4,GREEN,BLACK,16);	//��ʾÿ������
//		 	 }
//			 LCD_ShowString(COLUMN0,(LINE_NUM-4)*16,"BACK",GREEN,BLACK,16,0);	//ĩ�У���ת��menu2(ʾ��������)
//	  	}
//		
//		ec11_count = EC11_getCount();	//��ȡEC11����
//		last_ec11_count = ec11_count;	//
//		line_cursor = 0;				//��ʼ������
//	}
//	if(screen_flag==1)//����1
//	{
//		if(line_cursor==4)//���й���ڷ�����
//			 back_flag=1;//���ر�־λ��1
//		else
//			back_flag=0;
//	}
//	if(screen_flag==2)//����2
//	{
//		if(line_cursor==2)//���й���ڷ�����
//		 {
//			 back_flag=1;
//		 }
//		 else
//			 back_flag=0;
//	}
//	if(EC11_getButtonFlag())	//����������
//	{
//		if(back_flag==1)//���ر�־λ��1
//		{
//			menuSkip(&GUI,menu4);	//��ת��menu4
//			
//		}
//		if(line_cursor==LINE_NUM)//���ر�־λ��1
//		{	
//			menuSkip(&GUI,menu4);	//��ת��menu4
//		}
//		else
//		{
//			selected_flag=!selected_flag;	//��ѡ�� or δѡ��  or  ������ �л�
//			//if(selected_flag>2)selected_flag=0;
//			if(selected_flag==1) 
//			{
//				LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,WHITE);	//ѡ��Ϊ��ɫ
//			}
//			if(selected_flag==0)
//				LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//δѡ��Ϊ��ɫ
//		}
//	}
//	ec11_count = EC11_getCount();	//��ȡEC11����
//	
//	if(ec11_count != last_ec11_count)
//	{
//		if(selected_flag == 0) //δѡ����
//		{
//			line_cursor += ec11_count - last_ec11_count;
//			
//			if(screen_flag == 1)//����1
//			{
//			//δѡ���У���ת���������� ��ѡ��
//			line_cursor = line_cursor > 4 ? 0 : line_cursor;		//���һ����ת����һ��
//			line_cursor = line_cursor < 0 ? 4 : line_cursor;	//��һ����ת�����һ��
//			}
//			if(screen_flag == 2)//����2
//			{
//			//δѡ���У���ת���������� ��ѡ��
//			line_cursor = line_cursor > 2 ? 0 : line_cursor;		//���һ����ת����һ��
//			line_cursor = line_cursor < 0 ? 2 : line_cursor;	//��һ����ת�����һ��
//			}
//			LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);							//��ձ�ʶ
//			LCD_Fill(0,(line_cursor)*16,COLUMN0,(line_cursor+1)*16,RED);	//��ָ���б�ʶ��ɫɫ��
//			if(screen_flag == 3)
//			{
//			line_cursor = line_cursor > 2 ? 0 : line_cursor;		//���һ����ת����һ��
//			line_cursor = line_cursor < 0 ? 2 : line_cursor;	//��һ����ת�����һ��
//			}
//		}
//		else if(selected_flag == 1)		// ��ѡ����
//		{
//	
//				if(screen_flag==1)//����1
//			{
//			//��ѡ���еĲ������е���
//				switch (line_cursor)//���ȵ���
//				{
//					case 0:  Configs[0] += (ec11_count - last_ec11_count)*1;break;
//					case 1:  Configs[1] += (ec11_count - last_ec11_count)*1;break;
//					case 2:  Configs[2] += (ec11_count - last_ec11_count)*1;break;
//					case 3:  Configs[3] += (ec11_count - last_ec11_count)*1;break;
//					default :break;
//				}
//			    Configs[line_cursor] = Configs[line_cursor] >= 255 ? 0 : Configs[line_cursor];	//�޷�
//			    LCD_ShowIntNum(COLUMN1,(line_cursor)*16,Configs[line_cursor],4,GREEN,BLACK,16);			//��ʾ����
//			}
//			  if(screen_flag==2)//����2
//			{
//			//��ѡ���еĲ������е���
//				switch (line_cursor+4)
//				{
//					case 4:  Configs[4] += (ec11_count - last_ec11_count)*1;break;
//					case 5:  Configs[5] += (ec11_count - last_ec11_count)*1;break;
//					default :break;
//				}
//			Configs[line_cursor+4] = Configs[line_cursor+4] >= 255 ? 0 : Configs[line_cursor+4];	//�޷�
//			LCD_ShowIntNum(COLUMN1,(line_cursor)*16,Configs[line_cursor+4],4,GREEN,BLACK,16);			//��ʾ����
//			}
//		}
//		
//	}
//	//LCD_ShowFloatNum1(95,(6+1)*16,ec11_count,4,1,GREEN,BLACK,16);			//��ʾ����
//	last_ec11_count = ec11_count;
//	
//}

//void menu4(void)
//{
//	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11��������ֵ
//	
////	static uint8_t selected_flag = 0 ;	//��ѡ��
//	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//���³�ʼ��SPI1
//	if(GUI.init_flag)
//	{
//		//���������л�
//		GUI.init_flag = 0;
//		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ
//		ec11_count = EC11_getCount();	//��ȡEC11����
//		last_ec11_count = ec11_count;	//
//		line_cursor = 0;				//��ʼ������
//	}
//	LCD_ShowString(10,0,"config_L_A",WHITE,BLACK,16,0);	//��ʾ���յ�������
//	LCD_ShowString(10,20,"config_B",WHITE,BLACK,16,0);	//��ʾ���յ�������
//	LCD_ShowString(10,40,"save",WHITE,BLACK,16,0);	//��ʾ���յ�������
//	LCD_ShowString(10,60,"bin",WHITE,BLACK,16,0);	//��ʾ���յ�������
//	ec11_count = EC11_getCount();	//��ȡEC11����
//	//LCD_ShowFloatNum1(10,0,ec11_count,4,1,GREEN,BLACK,16);			//��ʾ����
//	if(ec11_count != last_ec11_count)
//	{
//			//δѡ���У���ת���������� ��ѡ��
//			line_cursor += ec11_count - last_ec11_count;
//			line_cursor = line_cursor > 4 ? 1 : line_cursor;		//���һ����ת����һ��
//			line_cursor = line_cursor <=0 ? 4 : line_cursor;	//��һ����ת�����һ��
//			LCD_Fill(0,0,COLUMN0,LCD_H,BLACK);							//��ձ�ʶ
//			LCD_Fill(0,(line_cursor-1)*20,COLUMN0,line_cursor*20,RED);	//��ָ���б�ʶ��ɫɫ��
//	}
//	last_ec11_count = ec11_count;
//	if(EC11_getButtonFlag())
//	{
//		if(line_cursor == 1)
//		{
//			screen_flag=1;
//		//	selected_flag=0;
//			menuSkip(&GUI,menu1);	//��ת��menu1
//		}
//		if(line_cursor == 2)
//		{
//			screen_flag=2;
//			//selected_flag=0;
//			menuSkip(&GUI,menu1);	//��ת��menu1
//		}
//		if(line_cursor == 3)
//		{
//			LCD_Fill(0,(line_cursor-1)*20,COLUMN0,(line_cursor)*20,GREEN);	//��ָ���б�ʶ��ɫɫ��
//			flash_erase_address(USER_FLASH_ADDRESS, 1);//��������

//			flash_write_single_address(USER_FLASH_ADDRESS, (uint32_t *)Configs, (FLASH_DATA_LENGHT + 3) / 4);//дflash
////		    	usart2_SendData(Configs,6);
////			   User_Delay(500);
//			//sprintf((char*)buff,"%3d",Configs[0]);
//			OpenMV_SendLabThr(Configs);
//		//	printf("%3d",Configs[0]);
//			 // send_data(Configs);
//	//	LCD_ShowIntNum(50,60,blob_y,4,GREEN,BLACK,16);			//��ʾ����
//			
//			//bt_SendData(Configs[0],Configs[1],Configs[2],Configs[3],Configs[4],Configs[5]);
//		}
//		if(line_cursor == 4)
//		{
//			static uint8_t openmv_disp_flag = 0;
//			//LCD_Fill(0,(line_cursor-1)*20,COLUMN0,(line_cursor)*20,GREEN);	//��ָ���б�ʶ��ɫɫ��
//			openmv_disp_flag = !openmv_disp_flag;
//			if(openmv_disp_flag)
//				OPENMV_SetDispMode(OPENMV_MODE_BIN);
//			else
//				OPENMV_SetDispMode(OPENMV_MODE_COL);
//			
//		}
//	}

//	
////	LCD_ShowFloatNum1(50,60,Angle_calc(),4,GREEN,BLACK,16);			//��ʾ����
//	//LCD_ShowFloatNum1(70,60,length_calc(bolb_x_low,bolb_y_low,bolb_x_high,bolb_y_high),4,GREEN,BLACK,16);			//��ʾ����
////	LCD_ShowFloatNum1(50,60,Lengh_calc(),4,GREEN,BLACK,16);			//��ʾ����
//	
////	printf("%2d\n",RXBUF[0]);
//}



