#include "filter.h"
#define len_size   5
/*************************************************
ȥ�����ֵ����Сֵ����ȡƽ��ֵ  ��ȡǰ5�γ���
**************************************************/
float length_average(uint8_t a[len_size])
{
	int i,max,min,sum;
	double average;
	sum=0;
	max=a[0];
	min=a[0];
	for(i=0;i<len_size;i++)
	{
	if(max<a[i])
	max=a[i];
	if(min>a[i])
	min=a[i];
	sum=sum+a[i];
	}
	average=(float)(sum-max-min)/(len_size-2);
	return average;
}


