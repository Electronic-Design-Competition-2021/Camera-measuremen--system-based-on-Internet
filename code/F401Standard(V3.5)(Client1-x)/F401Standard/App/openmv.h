#ifndef __OPENMV_H__
#define __OPENMV_H__

#include "main.h"

#include "struct_typedef.h"
#include "fifo.h"

#define OpenMV_TX_LENGTH 27
#define OpenMV_RX_LENGTH 8

#define OPENMV_MODE_COL 0X02	//彩色图像模式
#define OPENMV_MODE_BIN 0X03	//二值化图像模式

typedef struct
{
	uint8_t dataFlag;	//数据更新标志位
	uint8_t buttonRes;	//button结果
	float 	float1;		//浮点数1
} OpenMVDataType;

typedef struct
{
	float 	float1;		//浮点数1
	float 	float2;		//浮点数2
	float 	float3;		//浮点数3
} OpenMVDataType_tx;

void usart2_SendData(uint8_t *data, uint32_t length);
void openmv_DataUnPack(uint8_t *data);//用于和OpenMV通信
void OpenMV_SendLabThr(uint8_t dat[6]);//给openmv,发送6字节数据，
extern void openmv_DataUnPack(uint8_t *OpenMV_RX_Buf);//用于和OpenMV通信
void OpenMV_DataUnPack(uint8_t *data);
void OpenMV_DataProcess(fifo_s_t * p_fifo);    
void OPENMV_SetDispMode(uint8_t send_type);//调节OpenMV显示模式



#endif

