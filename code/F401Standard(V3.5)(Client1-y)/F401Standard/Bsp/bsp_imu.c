#include <string.h>
#include <stdio.h>
#include "bsp_imu.h"

// 超核电子9轴姿态传感器数据解析
// 使用方法：
// 波特率：921600
// 在初始化阶段调用 imu_data_decode_init()
// 在串口中断调用 packet_decode(ch);    ch:接收到的数据
// 接收数据读取示例：receive_imusol.eul[0];	读取当前pitch角度值

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#endif

#ifndef CH_OK
#define CH_OK   (0)
#endif

#ifndef CH_ERR
#define CH_ERR  (1)
#endif


static void crc16_update(uint16_t *currect_crc, const uint8_t *src, uint32_t len)
{
    uint32_t crc = *currect_crc;
    uint32_t j;
    for (j=0; j < len; ++j)
    {
        uint32_t i;
        uint32_t byte = src[j];
        crc ^= byte << 8;
        for (i = 0; i < 8; ++i)
        {
            uint32_t temp = crc << 1;
            if (crc & 0x8000)
            {
                temp ^= 0x1021;
            }
            crc = temp;
        }
    } 
    *currect_crc = crc;
}


enum status
{
    kStatus_Idle,
    kStatus_Cmd,
    kStatus_LenLow,
    kStatus_LenHigh,
    kStatus_CRCLow,
    kStatus_CRCHigh,
    kStatus_Data,
};

/* function pointer */
static on_data_received_event event_handler;
static packet_t *RxPkt;

 /**
 * @brief  初始化姿态解码模块
 * @note   完成初始化一个引脚配置
 * @param  pkt 接收包指针
 * @param  接收成功回调函数
 * @code

 *      void OnDataReceived(Packet_t *pkt)
 *      {
 *          pkt->buf 为数据 pkt->payload_len 为接收到的字节长度 
 *      }
 *
 *      Packet_t pkt;
 *      Packet_DecodeInit(&pkt, OnDataReceived);
 * @endcode
 * @retval None
 */
void packet_decode_init(packet_t *pkt, on_data_received_event func)
{
    event_handler = func;
    memset(pkt, 0, sizeof(packet_t));
    RxPkt = pkt;
}

 /**
 * @brief  接收IMU数据
 * @note   在串口接收中断中调用此函数
 * @param  c 串口数据
 * @retval CH_OK
 */


uint32_t packet_decode(uint8_t c)
{
    static uint16_t CRCReceived = 0;            /* CRC value received from a frame */
    static uint16_t CRCCalculated = 0;          /* CRC value caluated from a frame */
    static uint8_t status = kStatus_Idle;       /* state machine */
    static uint8_t crc_header[4] = {0x5A, 0xA5, 0x00, 0x00};
  
    switch(status)
    {
        case kStatus_Idle:
            if(c == 0x5A)
                status = kStatus_Cmd;
			break;
        case kStatus_Cmd:
            RxPkt->type = c;
			if(RxPkt->type == 0xA5)
				status = kStatus_LenLow;
            break;
        case kStatus_LenLow:
            RxPkt->payload_len = c;
            crc_header[2] = c;
            status = kStatus_LenHigh;
            break;
        case kStatus_LenHigh:
            RxPkt->payload_len |= (c<<8);
            crc_header[3] = c;
            status = kStatus_CRCLow;
            break;
        case kStatus_CRCLow:
            CRCReceived = c;
            status = kStatus_CRCHigh;
            break;
        case kStatus_CRCHigh:
            CRCReceived |= (c<<8);
            RxPkt->ofs = 0;
            CRCCalculated = 0;
            status = kStatus_Data;
            break;
        case kStatus_Data:
	
            RxPkt->buf[RxPkt->ofs++] = c;

            if(RxPkt->ofs >= MAX_PACKET_LEN)
            {
                status = kStatus_Idle;
                return CH_ERR;   
            }

            if(RxPkt->ofs >= RxPkt->payload_len && RxPkt->type == 0xA5)
            {
                /* calculate CRC */
                crc16_update(&CRCCalculated, crc_header, 4);
                crc16_update(&CRCCalculated, RxPkt->buf, RxPkt->ofs);
                
                /* CRC match */
                if(CRCCalculated == CRCReceived)
                {
                    event_handler(RxPkt);
                }
                status = kStatus_Idle;
            }
            break;
        default:
            status = kStatus_Idle;
            break;
    }
    return CH_OK;
}




static packet_t RxPkt_s; /* used for data receive */

uint32_t frame_count;
uint8_t bitmap;

__align(4)  receive_imusol_packet_t receive_imusol;
__align(4)  receive_gwsol_packet_t receive_gwsol;

static int stream2int16(int *dest,uint8_t *src)
{
	dest[0] = (int16_t)(src[0] | src[1] << 8);
	dest[1] = (int16_t)(src[2] | src[3] << 8);
	dest[2] = (int16_t)(src[4] | src[5] << 8);	
	return 0;
}   


/*  callback function of  when recv a data frame successfully */
static void on_data_received(packet_t *pkt)
{
	int temp[3] = {0};
    int i = 0;
    int offset = 0;
    uint8_t *p = pkt->buf;

	if(pkt->type != 0xA5)
    {
        return;
    }
	while(offset < pkt->payload_len)
	{
        if(offset == 0)
        {
            frame_count++;
            bitmap = 0;
        }
		switch(p[offset])
		{
            case kItemID:
                bitmap |= BIT_VALID_ID;
                receive_imusol.id = p[1];
                offset += 2;
                break;
            case kItemAccRaw:
                bitmap |= BIT_VALID_ACC;
                stream2int16(temp, p + offset + 1);
                receive_imusol.acc[0] = (float)temp[0] / 1000;
                receive_imusol.acc[1] = (float)temp[1] / 1000;
                receive_imusol.acc[2] = (float)temp[2] / 1000;
                offset += 7;
                break;
            case kItemGyrRaw:
                bitmap |= BIT_VALID_GYR;
                stream2int16(temp, p + offset + 1);
                receive_imusol.gyr[0] = (float)temp[0] / 10;
                receive_imusol.gyr[1] = (float)temp[1] / 10;
                receive_imusol.gyr[2] = (float)temp[2] / 10;
                offset += 7;
                break;
            case kItemMagRaw:
                bitmap |= BIT_VALID_MAG;
                stream2int16(temp, p + offset + 1);
                receive_imusol.mag[0] = (float)temp[0] / 10;
                receive_imusol.mag[1] = (float)temp[1] / 10;
                receive_imusol.mag[2] = (float)temp[2] / 10;
                offset += 7;
                break;
            case kItemRotationEul:
                bitmap |= BIT_VALID_EUL;
                stream2int16(temp, p + offset + 1);
                receive_imusol.eul[1] = (float)temp[0] / 100;
                receive_imusol.eul[0] = (float)temp[1] / 100;
                receive_imusol.eul[2] = (float)temp[2] / 10;
                offset += 7;
                break;
            case kItemRotationQuat:
                bitmap |= BIT_VALID_QUAT;
                memcpy((void *)receive_imusol.quat, p + offset + 1, sizeof( receive_imusol.quat));
                offset += 17;
                break;
            case kItemPressure:
                offset += 5;
                break;

            case KItemIMUSOL:
                bitmap = BIT_VALID_ALL;

                receive_imusol.id =p[offset + 1];
        
                memcpy((void *) receive_imusol.acc, p + 12 , sizeof(float) * 16);

                offset += 76;
                break;
            case KItemGWSOL:

                receive_gwsol.tag = p[offset];
                receive_gwsol.gw_id = p[offset + 1]; 
                receive_gwsol.n = p[offset + 2];
                offset += 8;
                for (i = 0; i < receive_gwsol.n; i++)
                {
                    bitmap = BIT_VALID_ALL;
                    receive_gwsol.receive_imusol[i].tag = p[offset];
                    receive_gwsol.receive_imusol[i].id = p[offset + 1];
                    memcpy(&receive_gwsol.receive_imusol[i].acc, p + offset + 12 , sizeof(float) * 16);

                    offset += 76;
                }
                break;
            default:
                offset++;
		}
    }
}


int imu_data_decode_init(void)
{
    packet_decode_init(&RxPkt_s, on_data_received);
    return 0;
}




