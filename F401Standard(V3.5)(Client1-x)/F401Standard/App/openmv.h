#ifndef __OPENMV_H__
#define __OPENMV_H__

#include "main.h"

#include "struct_typedef.h"
#include "fifo.h"

#define OpenMV_TX_LENGTH 27
#define OpenMV_RX_LENGTH 8

#define OPENMV_MODE_COL 0X02	//��ɫͼ��ģʽ
#define OPENMV_MODE_BIN 0X03	//��ֵ��ͼ��ģʽ

typedef struct
{
	uint8_t dataFlag;	//���ݸ��±�־λ
	uint8_t buttonRes;	//button���
	float 	float1;		//������1
} OpenMVDataType;

typedef struct
{
	float 	float1;		//������1
	float 	float2;		//������2
	float 	float3;		//������3
} OpenMVDataType_tx;

void usart2_SendData(uint8_t *data, uint32_t length);
void openmv_DataUnPack(uint8_t *data);//���ں�OpenMVͨ��
void OpenMV_SendLabThr(uint8_t dat[6]);//��openmv,����6�ֽ����ݣ�
extern void openmv_DataUnPack(uint8_t *OpenMV_RX_Buf);//���ں�OpenMVͨ��
void OpenMV_DataUnPack(uint8_t *data);
void OpenMV_DataProcess(fifo_s_t * p_fifo);    
void OPENMV_SetDispMode(uint8_t send_type);//����OpenMV��ʾģʽ



#endif

