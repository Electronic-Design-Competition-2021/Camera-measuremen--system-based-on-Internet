#ifndef __APP_BT_H__
#define __APP_BT_H__


#include "struct_typedef.h"
#include "fifo.h"

#define BT_TX_LENGTH 27
#define BT_RX_LENGTH 11

typedef struct
{
	uint8_t dataFlag;	//数据更新标志位
	uint8_t buttonRes;	//button结果
	float 	float1;		//浮点数1
} btDataType;

typedef struct
{
	float 	float1;		//浮点数1
	float 	float2;		//浮点数2
	float 	float3;		//浮点数3
} btDataType_tx;

extern btDataType btData;
void bt_SendData(float data1 ,float data2,float data3,float data4,float data5,float data6);
void bt_DataProcess(fifo_s_t * p_fifo);

#endif

