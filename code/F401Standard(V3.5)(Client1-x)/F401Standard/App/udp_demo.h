#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "types.h"

extern uint16 W5500_tcp_server_port;

void Tx_udp(uint8 * buff,uint32 len);
void Rx_udp(void);//接收来自客户端的数据
#endif 

