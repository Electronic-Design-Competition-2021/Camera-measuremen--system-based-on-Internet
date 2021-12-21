#ifndef __APP_BT_H__
#define __APP_BT_H__


#include "struct_typedef.h"
#include "fifo.h"

#define BT_TX_LENGTH 27
#define BT_RX_LENGTH 11

typedef struct
{
	uint8_t dataFlag;	//���ݸ��±�־λ
	uint8_t buttonRes;	//button���
	float 	float1;		//������1
} btDataType;

typedef struct
{
	float 	float1;		//������1
	float 	float2;		//������2
	float 	float3;		//������3
} btDataType_tx;

extern btDataType btData;
void bt_SendData(float data1 ,float data2,float data3,float data4,float data5,float data6);
void bt_DataProcess(fifo_s_t * p_fifo);

#endif

