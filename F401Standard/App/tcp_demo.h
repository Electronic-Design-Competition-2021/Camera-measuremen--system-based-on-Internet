#ifndef __TCP_DEMO_H
#define __TCP_DEMO_H
#include "types.h"

extern uint16 W5500_tcp_server_port;

void Tx_udp(uint8 * buff,uint32 len);//���͵��ͻ���
void Rx_udp(uint8 * buff1,uint8 * buff2);	//�������Կͻ��˵�����

#endif 

