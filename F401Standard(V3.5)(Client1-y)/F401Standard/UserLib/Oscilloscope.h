/**
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  * @file       Oscilloscope.c/h
  * @brief      示波器实现（滚动波形显示）
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     July-1-2021     doee            1. 完成
  * @blog		https://blog.csdn.net/weixin_44578655?spm=1001.2101.3001.5343
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  */

#ifndef __OSCILL_H__
#define __OSCILL_H__
#include "struct_typedef.h"

  
  

/*******************************************************************************
	mode0:幅值自适应模式，此模式下示波器根据上一页的数据的最大、最小值确定当前页的缩放比例和y轴偏移量
	mode1:幅值手动模式，用户自行确定缩放比例和y轴偏移量
*******************************************************************************/
typedef struct
{
	uint16_t loca_x;	//窗口起点x坐标
	uint16_t loca_y;	//窗口起点y坐标
	uint16_t length;	//窗口长度
	uint16_t width;		//窗口宽度
	uint16_t cursor_x;		//当前刷新位置x坐标
	uint16_t last_cursor_x;	//上次刷新位置x坐标
	uint16_t cursor_y;		//当前刷新位置y坐标
	uint16_t last_cursor_y;	//上次刷新位置y坐标
	int32_t y_max;		//y值历史最大值
	int32_t y_min;		//y值历史最小值
	float y_ratio;		//y轴缩放比例
	int32_t y_max_last;		//上一页y值历史最大值
	int32_t y_min_last;		//上一页y值历史最大值
	uint8_t mode;			//模式
	int32_t y_bias;		//y轴偏移
}osc_window;

void osc_init(osc_window* win1,uint16_t loca_x_set,uint16_t loca_y_set,uint16_t length_set,uint16_t width_set,uint8_t mode);
void osc_refresh(osc_window* win1, uint32_t x_raw,int32_t y_raw);
void osc_y_boundary_set(osc_window* osc1,float ymin,float ymax);
void osc_mode_set(osc_window* win1,uint8_t mode);
#endif

