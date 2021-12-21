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
extern int8_t line_cursor;		//�й��
extern float Configs[LINE_NUM];	//��������

uint8_t txbuf[5]={0,1,2,3,4};

extern uint32_t spi2_rx_Count;
extern uint8_t spi2_rx_Flag;
void spi1DMASendByte(uint8_t *cmar,uint16_t cndtr);

void appInit(void)
{
	usart2_init();	//��ʼ������2
	spi1_Init();	//spi1��ʼ��
	spi2_Init();	//spi2��ʼ��

	ec11_Init();//EC11��ʼ��

	SPI1_LCD_ReInit(SPI_BAUDRATEPRESCALER_2);	//��ʼ��SPI
	LCD_Init();			//LCD��ʼ��
	User_Delay(100);
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);	//��ʼ��ɫ

	menuInit(&GUI,menu_main);	//��ʼ�����˵�

	//��flash�еĲ���
	flash_read(USER_FLASH_ADDRESS, (uint32_t *)Configs, (FLASH_DATA_LENGHT + 3)/ 4);

	/***************************W5500 Init*************************************/
	User_GPIO_WritePin(W5500_CS_GPIO_Port,W5500_CS_Pin,GPIO_PIN_SET);
	User_GPIO_WritePin(W5500_RST_GPIO_Port,W5500_RST_Pin,GPIO_PIN_SET);
	
	HAL_Delay(100);
	reset_w5500();											/*Ӳ��λW5500*/

	set_w5500_mac();										/*����MAC��ַ*/
	set_w5500_ip();											/*����IP��ַ*/
	socket_buf_init(txsize, rxsize);		/*��ʼ��8��Socket�ķ��ͽ��ջ����С*/
	/************************************************************************/
	
}

uint32_t ledTick;
uint32_t lcdTick;
uint32_t ec11Tick;
uint32_t netSendTick = 0;
uint32_t UDP_rx_Tick = 0;

uint8_t udp_measure_flag = 0;	
uint8_t udp_start_flag = 0;	//ͼ���Ϳ�����־λ
uint8_t debug_mode_flag = 0;

extern uint8_t img_Buf[26000];
uint8_t img_Data[24000];


uint8_t measure_flag = 0;//������־λ
uint32_t get_pos_tick = 0;
extern uint16_t x_max;

uint8_t UDP_tx_buf[12];		//UDP���ͻ���
uint8_t tx_buf[7];
extern float length;	//����l
extern uint32_t x_max_tick ;	//��׽���ֵ��ʱ��
extern uint8_t osc_flag;
extern uint16_t blob_x_pos;
void appMain(void)
{
	vari_init();	//������ʼ��
//	menuSkip(&GUI,menu_StartMeasure);	//��ת��menu_StartMeasure,��ʼ�����������ã�
	while(1)
	{
		menuRefresh(&GUI);			//ˢ��GUI:�˵���appMenu.c
		OpenMV_DataProcess(&u2_Fifo);
		
		//��ʼ���������ն˿���
		if(measure_flag == 1)
		{
			if(User_GetTick() - get_pos_tick >= 100)
			{
				get_pos_tick = User_GetTick();
				
				if(Measure_Xmax_Length() == 2)		/*�������ֵ*/
				{
					UDP_tx_buf[0] = 0xB5;
					UDP_tx_buf[1] = 0xB5;
					
//					UDP_tx_buf[2] = length;			//�����õ���L
					memcpy(&UDP_tx_buf[2],&length,4);	//�����õ���L
					memcpy(&UDP_tx_buf[6],&x_max,2);	//�����õ���x_max
					
					
					//У���
					UDP_tx_buf[8] = UDP_tx_buf[2]+UDP_tx_buf[3]+UDP_tx_buf[4]+
									UDP_tx_buf[5]+UDP_tx_buf[6]+UDP_tx_buf[7];
					
					Tx_udp(UDP_tx_buf,9);//���͵������
					HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);	//LED��˸
				}
			}
		}
		
		//LCD_ShowIntNum(80,130,x_max_tick,6,WHITE,BLACK,12);//�ڵ���ʾ��ȡ�����ֵ����ʱ��
		/*******************************�������Է����UDP����******************************/
		if(User_GetTick() - UDP_rx_Tick >= 100)
		{
			UDP_rx_Tick = User_GetTick();
			Rx_udp();	//�������Է����UDP����
		}
		
		if(User_GetTick() - lcdTick >= 	50)
		{
			lcdTick = User_GetTick();
			menuRefreshFlagSet(&GUI);	//menuˢ�±�־λ��λ
		}
		
		if(User_GetTick() - ec11Tick >= 10)
		{
			ec11Tick = User_GetTick();
			EC11_ScanButton();
		}
		/******************************����ͼ���ʾ����ʵʱ����**************************************/
		if(HAL_GetTick() - netSendTick >= 100)
		{
			netSendTick = HAL_GetTick();
			
			//����ʾ����ʵʱ����
			if(osc_flag == 1)
			{
				UDP_tx_buf[0] = 0xC5;
				UDP_tx_buf[1] = 0xC5;
				memcpy(&UDP_tx_buf[2],&blob_x_pos,2);	//ʵʱ����
				//У���
				UDP_tx_buf[4] = UDP_tx_buf[2]+UDP_tx_buf[3];
				UDP_tx_buf[5] = 0;		//�����ֽڣ�����ż������ֹͼ�����ش�λ
				Tx_udp(UDP_tx_buf,6);//���͵������
			}
			
			/*****************һ֡ͼ������η��������****************************/
			if(udp_start_flag ==  1)		//��GUI���棨appmenu.c������λ
			{
				static uint8_t n = 0;
				
				if(n == 0)	//����֡ͷ�����ڽ��ն˶�λ
				{		
					memcpy(img_Data,img_Buf,24000);	//��ס��һ֡����ֹ���ն˳��ֶϲ�
					img_Data[0] = 0xA5;
					img_Data[1] = 0xA5;
				}
				
				Tx_udp(img_Data + (n++)*8000,8000);		//���Ͳ�������ͼ��(�����Σ�ÿ��8000�ֽ�)
				if (n > 2)//������һ֡ͼ��֮�󣬷���һ��ʱ��
				{
					n = 0;
//					tx_buf[0] = 0xC5;
//					tx_buf[1] = 0xC5;
//					memcpy(&tx_buf[2],&x_max_tick,4);//��ȡ���������ֵ��ʱ��
//					tx_buf[6]=tx_buf[2]+tx_buf[3]+tx_buf[4]+tx_buf[5];
//					Tx_udp(tx_buf,7);//���͵������
				}
			}
	
		}

	}
	
}
