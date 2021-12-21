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
//#include "bluetooth.h"
//#include "bsp_flash.h"
//#include "bsp_24l01.h"
#include "menu.h"
#include "Oscilloscope.h"
#include "bsp_ec11.h"
#include "math.h"
#include "udp_demo.h"
#include "string.h"

menu_s_t GUI;	//����༶�˵�

uint8_t lcdBuf[64];
//extern uint16_t write_data[FLASH_DATA_LENGHT];
//extern uint16_t after_write_data[FLASH_DATA_LENGHT];

float Configs[LINE_NUM];	//��������
int8_t line_cursor;		//�й��
int8_t screen_flag;		//�˵����
int8_t back_flag;


uint8_t start_buf[3] = {0x11,0x11,0x11};
uint8_t done_buf[3]  = {0x12,0x12,0x12};
uint8_t Bin_buf[3] 	 = {0x13,0x13,0x13};
uint8_t Osc_on_buf[3] 	 = {0x14,0x14,0x14};
uint8_t Osc_off_buf[3] 	 = {0x15,0x15,0x15,};

uint8_t start_measure_flag = 0;
uint32_t measure_tick = 0;//������־λ
uint8_t beep_flag = 0;//��������־λ
uint32_t beep_tick = 0;


extern volatile int ec11_num;

extern uint8_t ang_rx_count;	//�Ƕ����ݽ��ռ���
extern uint8_t c1_len_rx_count,c2_len_rx_count;	//�������ݽ��ռ���


#define LEN_MEASURE_NUM  10
#define ANGLE_MEASURE_NUM  5

extern float angle_buf[ANGLE_MEASURE_NUM];	//ԭʼ�Ƕ����ݣ�5�Σ�
extern float c1_length_raw_buff[LEN_MEASURE_NUM],c2_length_raw_buff[LEN_MEASURE_NUM];	//ԭʼ�������ݣ�10�Σ�
extern uint8_t ang_measure_done_flag;		//angle������ɱ�־λ
extern uint8_t c1_len_measure_done_flag;	//c1_len������ɱ�־
extern uint8_t c2_len_measure_done_flag;	//c2_len������ɱ�־

//�������
extern float angle;
extern float c1_length,c2_length;


//�����棨��ʼ���棩
void menu_main(void)
{
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//���³�ʼ��SPI1	
	static int32_t ec11_count = 1,last_ec11_count = 0;//ec11��������ֵ
	static uint8_t selected_flag = 0 ;	//��ѡ��
	if(GUI.init_flag)
	{
		//���������л�,����ҳ���ʼ��
		GUI.init_flag = 0;	//��־λ����
		LCD_Fill(0,112,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ:��ɫ
		LCD_ShowString(6,100,"A:",GREEN,BLACK,12,0);		//�Ƕ�
		LCD_ShowString(70,100,"L:",GREEN,BLACK,12,0);		//����
		LCD_ShowString(6,112,"k_lmin",GREEN,BLACK,12,0);		//��һ��
		LCD_ShowString(70,112,"k_lmax",GREEN,BLACK,12,0);		//�ڶ���
		LCD_ShowString(6,124,"k_a0",GREEN,BLACK,12,0);			//������
		LCD_ShowString(70,124,"k_a9",GREEN,BLACK,12,0);			//������
		LCD_ShowString(6,136,"k_a1",GREEN,BLACK,12,0);			//������
		LCD_ShowString(70,136,"k_a4",GREEN,BLACK,12,0);			//������
		LCD_ShowString(6,148,"OSC",GREEN,BLACK,12,0);			//ʾ����
		LCD_ShowString(70,148,"BIN:",GREEN,BLACK,12,0);			//��ֵ��

		ec11_count = EC11_getCount();	//��ȡEC11����
		last_ec11_count = ec11_count;	//��¼��ʼEC11����
		line_cursor = 0;				//��ʼ������
//		LCD_Fill(0,112+(line_cursor)*12,COLUMN0,112+(line_cursor+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��
	}
	
	ec11_count = EC11_getCount();	//��ȡEC11����
	
	if(selected_flag == 0)
	{
		if(ec11_count != last_ec11_count)
		{
			if(ec11_count > last_ec11_count)
				line_cursor -= 1;
			else
				line_cursor += 1;
			
			line_cursor = line_cursor > 7 ? 7 : line_cursor;		//�ƶ���ͷ
			line_cursor = line_cursor < 0 ? 0 : line_cursor;		//	
			LCD_Fill(0,112,COLUMN0,LCD_H,BLACK);							//��ձ�ʶ
			LCD_Fill(64,112,COLUMN1,LCD_H,BLACK);							//��ձ�ʶ
			
			//���ɫ��
			if(line_cursor % 2 == 0)
				LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��
			else
				LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��		
		}
		
		last_ec11_count = ec11_count;
		
		if(EC11_getButtonFlag())	//����������
		{
			if(line_cursor == 6)
			{
				Tx_udp(Osc_on_buf,3);		//����ʾ����ָ��
				HAL_Delay(12);
				Tx_udp(Osc_on_buf,3);		//����ʾ����ָ��
				
				menuSkip(&GUI,menu_osc);
				return;
			}
			else if(line_cursor == 7)
			{
				Tx_udp(Bin_buf,3);		//���Ͷ�ֵ��ָ��
				return;
			}
			else
			{
				start_measure_flag = 1;				//��ʼ����
				
				measure_tick = HAL_GetTick();		//��¼��ʼʱ��
								//��־λ����
				ang_measure_done_flag = 0;
				c1_len_measure_done_flag = 0;
				c2_len_measure_done_flag = 0;

				//��������
				ang_rx_count = 0;		//�ǶȽ���5��
				c1_len_rx_count = 0;	//���Ƚ���10��
				c2_len_rx_count = 0;
				angle = 0;
				c1_length = 0;
				c2_length = 0;
				memset(angle_buf,0,ANGLE_MEASURE_NUM);
				memset(c1_length_raw_buff,0,LEN_MEASURE_NUM);
				memset(c2_length_raw_buff,0,LEN_MEASURE_NUM);
				
				//���Ϳ�ʼָ��
				Tx_udp(start_buf,3);				
				HAL_Delay(12);		//delay
				Tx_udp(start_buf,3);				//���Ϳ�ʼָ��(�������Σ���ֹ���ղ���)
				
			}
			
			selected_flag=1;	//��ѡ�� or δѡ��
			
			if(selected_flag)
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,WHITE);	//��ָ���б�ʶ��ɫɫ��
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,WHITE);	//��ָ���б�ʶ��ɫɫ��
			}
			else
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��	
			}				
		}	
	}
	else							//�쳣ʱ��ϲ���
	{
		if(EC11_getButtonFlag())	//����������
		{
			
				start_measure_flag = 0;				//��������
				
				measure_tick = HAL_GetTick();		//��¼��ʼʱ��
								//��־λ����
				ang_measure_done_flag = 0;
				c1_len_measure_done_flag = 0;
				c2_len_measure_done_flag = 0;

				//��������
				ang_rx_count = 0;		//�ǶȽ���5��
				c1_len_rx_count = 0;	//���Ƚ���10��
				c2_len_rx_count = 0;
				angle = 0;
				c1_length = 0;
				c2_length = 0;
				memset(angle_buf,0,ANGLE_MEASURE_NUM);
				memset(c1_length_raw_buff,0,LEN_MEASURE_NUM);
				memset(c2_length_raw_buff,0,LEN_MEASURE_NUM);
				LCD_ShowIntNum(COLUMN0+36,112+(line_cursor/2)*12,(HAL_GetTick() - measure_tick)/1000,2,BLACK,BLACK,12);//�����ʱ
				//���Ϳ�ʼָ��
				Tx_udp(done_buf,3);				
				HAL_Delay(12);		//delay
				Tx_udp(done_buf,3);				//���Ϳ�ʼָ��(�������Σ���ֹ���ղ���)
				
			
			
			selected_flag=0;	//��ѡ�� or δѡ��
			
			if(selected_flag)
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,WHITE);	//��ָ���б�ʶ��ɫɫ��
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,WHITE);	//��ָ���б�ʶ��ɫɫ��
			}
			else
			{
				if(line_cursor % 2 == 0)
					LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��
				else
					LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��	
			}				
		}	
	}
	if(start_measure_flag == 1)
	{
		//�����������ݶ�������ɣ���ǰ����
		if(ang_measure_done_flag && c1_len_measure_done_flag && c2_len_measure_done_flag)
		{
			//��־λ����
			ang_measure_done_flag = 0;
			c1_len_measure_done_flag = 0;
			c2_len_measure_done_flag = 0;
			
			//���ͽ���ָ��
			Tx_udp(done_buf,3);				
			HAL_Delay(12);
			Tx_udp(done_buf,3);				//���ͽ���ָ��(��������)
			
			//��ʾ�Ƕ�
			LCD_ShowFloatNum1(18,100, angle,4,1,WHITE,BLACK,12);	
			
			//���ݽǶ��ж�ѡ����һ��ĳ�������
			if(angle > 45)
				LCD_ShowFloatNum1(82,100, c1_length,4,1,WHITE,BLACK,12);		//��ʾ����
			else
				LCD_ShowFloatNum1(82,100, c2_length,4,1,WHITE,BLACK,12);		//��ʾ����
			
			beep_flag = 1;		/*��ʼ��������*/
			start_measure_flag = 0;
		}
		//�ﵽ���ʱ�䣬��������
		else if(HAL_GetTick() - measure_tick >= 28000)		//28s
		{
			Tx_udp(done_buf,3);				//���ͽ���ָ��
			HAL_Delay(12);
			Tx_udp(done_buf,3);				//���ͽ���ָ��(��������)
			
			//90�Ȼ�0�ȵ���������Ȳ�����ɣ����Ƕ��޷���ȷ����
			if(ang_measure_done_flag == 0 && (c1_len_measure_done_flag == 1 || c2_len_measure_done_flag == 1))
			{
				
				int8_t delta_cnt = c1_len_rx_count - c2_len_rx_count;		//�������
				if(delta_cnt >= 2)
				{
					angle = 85;		//У׼ΪΪ5�ȣ�����0-10�ȣ�
				}
				else if(delta_cnt <= -2)
				{
					angle = 5;		//У׼Ϊ85�ȣ�����80-90�ȣ�
				}
				
				//��ʾ�Ƕ�
				LCD_ShowFloatNum1(18,100, angle,4,1,WHITE,BLACK,12);	
				
				//���ݽǶ��ж�ѡ����һ��ĳ�������
				if(angle > 45)
					LCD_ShowFloatNum1(82,100, c1_length,4,1,WHITE,BLACK,12);		//��ʾ����
				else
					LCD_ShowFloatNum1(82,100, c2_length,4,1,WHITE,BLACK,12);		//��ʾ����		
				

			}
			//�����쳣���
			else if(ang_measure_done_flag == 1 && (c1_len_measure_done_flag == 0 || c2_len_measure_done_flag == 0))
			{
				LCD_ShowFloatNum1(82,100, c1_length,4,1,WHITE,BLACK,12);		//��ʾ����
				LCD_ShowFloatNum1(82,100, c2_length,4,1,WHITE,BLACK,12);		//��ʾ����
			}
//			else
//			{
//				//��ʾerr
//				LCD_ShowString(18,100, "err",WHITE,BLACK,12,0);
//				LCD_ShowString(82,100, "err",WHITE,BLACK,12,0);		
//			}
			beep_flag = 1;		/*��ʼ��������*/
			start_measure_flag = 0;
		}
		else
		{
			//��ʾ��ʱ
			if(line_cursor % 2 == 0)
				LCD_ShowIntNum(COLUMN0+36,112+(line_cursor/2)*12,(HAL_GetTick() - measure_tick)/1000,2,YELLOW,BLACK,12);
			else
				LCD_ShowIntNum(COLUMN1+36,112+(line_cursor/2)*12,(HAL_GetTick() - measure_tick)/1000,2,YELLOW,BLACK,12);
		}
	}
	
	if(beep_flag == 1)		/*��������*/
	{
		static uint8_t n = 0;
		if(HAL_GetTick() - beep_tick >= 200)	//���200ms
		{
			beep_tick = HAL_GetTick();
			HAL_GPIO_TogglePin(LED_BEEP_GPIO_Port,LED_BEEP_Pin);
			n++;
		}
		if(n == 6)		//��3��
		{
			n = 0;
			HAL_GPIO_WritePin(LED_BEEP_GPIO_Port,LED_BEEP_Pin,GPIO_PIN_SET);
			//�رռ�ʱ
//			if(line_cursor % 2 == 0)
//				LCD_Fill(COLUMN0+36,112+(line_cursor/2)*12,COLUMN0+36+18,112+((line_cursor/2)+1)*12,BLACK);	//��ָ���б�ʶ��ɫɫ��
//			else
//				LCD_Fill(COLUMN1+36,112+(line_cursor/2)*12,COLUMN1+36+18,112+((line_cursor/2)+1)*12,BLACK);	//��ָ���б�ʶ��ɫɫ��	
			beep_flag = 0;		//������������
			selected_flag = 0;
			
			if(line_cursor % 2 == 0)
				LCD_Fill(0,112+(line_cursor/2)*12,COLUMN0,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��
			else
				LCD_Fill(64,112+(line_cursor/2)*12,COLUMN1,112+((line_cursor/2)+1)*12,RED);	//��ָ���б�ʶ��ɫɫ��
		}
	}
}

//����ʾ����ģʽʱ��x1_maxΪ�ڵ�1ʵʱ���͵�x����
extern uint16_t x1_max;
extern uint16_t x2_max;
uint8_t anolog_mode = 0;	//ģ��ģʽ��־

//����ʾ��������
osc_window osc_win;
uint32_t osc_x = 0;

//0.707f
void menu_osc(void)
{
	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//���³�ʼ��SPI1
	CS_pin=LCD2_CS_Pin;	//ƬѡLCD2
	if(GUI.init_flag)
	{
		//���������л�
		LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//����
		
		GUI.init_flag = 0;
		osc_init(&osc_win,0,112,128,48,1);		//ʾ������ʼ��,ģʽ1(�ֶ���ֵģʽ)
		osc_y_boundary_set(&osc_win,50,150);		//���÷�ֵ��Χ:0-120
		anolog_mode = 1;	//����ģ��ģʽ
		
		

	}

	osc_refresh(&osc_win,osc_x++,x1_max);	//ˢ��ʾ�������ڣ�����Ļ���ص�Ϊ��λ��
	
	//��Ŀ�꣬������ʵ�����ģ�100��160��
	
	int16_t x_aim = x2_max-160;
	int16_t y_aim = x1_max-100;
	int16_t z_aim = -20;		//��ֵ
	
	static uint8_t screen_x = 50;
	static uint8_t screen_y = 50;
	
	//��������ϵ��ԭ��(50,50)  x���յ�(15,75) y���յ�(110,50) z���յ�(50,5)
	LCD_DrawLine(50,50,25,75,WHITE);	//x��
	LCD_DrawLine(50,50,110,50,WHITE);	//y��
	LCD_DrawLine(50,50,50,5,WHITE);		//z��
	LCD_DrawLine(25,75,30,65,WHITE);	//x���ͷ
	LCD_DrawLine(25,75,35,75,WHITE);
	LCD_DrawLine(110,50,100,44,WHITE);	//y���ͷ
	LCD_DrawLine(110,50,100,56,WHITE);
	LCD_DrawLine(50,5,44,15,WHITE);		//z���ͷ
	LCD_DrawLine(50,5,56,15,WHITE);
	//����ϴ�Ŀ��
	LCD_Fill(screen_x-2,screen_y-2,screen_x+2,screen_y+2,BLACK);
	//����ϴΰ���
	LCD_DrawLine(50,5,screen_x,screen_y,BLACK);
	//������Ļ����
	screen_x = (y_aim-0.707f*x_aim)*0.6f + 50;
	screen_y = (z_aim-0.707f*x_aim)*0.6f + 50;
//	LCD_ShowFloatNum1(x_aim,y_aim,);
	//ͶӰ��Ļ���꣨xδ�䣩
	uint8_t screen_y_shadow = (-0.707f*x_aim)*0.6f + 50;
	
	LCD_Fill(screen_x-2,screen_y-2,screen_x+2,screen_y+2,RED);
	
	//���ư���
	LCD_DrawLine(50,5,screen_x,screen_y,YELLOW);
	
	//����ͶӰ
	LCD_Fill(screen_x-1,screen_y_shadow-1,screen_x+1,screen_y_shadow+1,BRRED);	
	
	if(EC11_getButtonFlag())
	{
		Tx_udp(Osc_off_buf,3);	//���͹ر�ʾ����ָ��
		HAL_Delay(12);
		Tx_udp(Osc_off_buf,3);	//���͹ر�ʾ����ָ��
		
		anolog_mode = 0;		//�˳�ģ��ģʽ
		menuReturn(&GUI);	//����
	}
}

