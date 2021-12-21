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
#include "Oscilloscope.h"
#include "bsp_lcd.h"


/**
  * @brief          示波器窗口初始化
  * @param[in]      osc1: 示波器窗口指针
  * @param[in]      loca_x_set: 窗口x坐标
  * @param[in]      loca_y_set: 窗口y坐标
  * @param[in]      length_set: 窗口长度
  * @param[in]      width_set:  窗口宽度 
  * @param[in]      mode：		模式
  *					@arg 0: 自适应幅值模式
  *					@arg 1: 手动幅值模式
  */
void osc_init(osc_window* osc1,uint16_t loca_x_set,uint16_t loca_y_set,uint16_t length_set,uint16_t width_set,uint8_t mode)
{
	//设置示波器窗口的基本参数
	length_set -= 1;
	width_set -= 1;
	osc1->length = length_set - 1;
	osc1->width =  width_set - 1;
	osc1->loca_x = loca_x_set + 1;
	osc1->loca_y = loca_y_set + 1;
	osc1->y_min = 0;
	osc1->y_max = 0;
	osc1->y_min_last = 0;
	osc1->y_max_last = 0;
	osc1->y_ratio = 0;
	osc1->mode = mode;
	//绘制示波器边框
	LCD_DrawLine(loca_x_set,loca_y_set,loca_x_set + length_set,loca_y_set,WHITE);
	LCD_DrawLine(loca_x_set,loca_y_set + width_set,loca_x_set + length_set,loca_y_set + width_set,WHITE);
	LCD_DrawLine(loca_x_set,loca_y_set,loca_x_set,loca_y_set + width_set,WHITE);
	LCD_DrawLine(loca_x_set + length_set,loca_y_set,loca_x_set + length_set,loca_y_set + width_set,WHITE);	
}
/**
  * @brief          示波器模式设置
  * @param[in]      mode：		模式
  *					@arg 0: 自适应幅值模式
  *					@arg 1: 手动幅值模式
  */
void osc_mode_set(osc_window* osc1,uint8_t mode)
{
	osc1->mode = mode;
}
/**
  * @brief          示波器设置y轴数据范围
  * @param[in]      osc1: 示波器窗口指针
  * @param[in]      ymin: y轴最小值
  * @param[in]      ymax: y轴最大值
  */
void osc_y_boundary_set(osc_window* osc1,float ymin,float ymax)
{
	
	osc1->y_ratio = osc1->width/ (ymax - ymin);	//设置缩放
	osc1->y_ratio -= osc1->y_ratio*0.2f;		//界面上下留20%的空白
	osc1->y_bias = ymin;
}

//示波器刷新显示：按照心电波形刷新特点自己写的，仅供参考，不一定适用于真实的示波器显示
//特点：幅值自适应，会对波形进行幅值缩放，始终让波形保持在窗口中。
/**
  * @brief          示波器刷新显示
  * @param[in]      osc1: 示波器窗口指针
  * @param[in]      x_raw: x轴数据
  * @param[in]      y_raw: y轴数据
  */
void osc_refresh(osc_window* osc1, uint32_t x_raw,int32_t y_raw)
{
	osc1->cursor_x = x_raw % osc1->length + osc1->loca_x;	//计算当前刷新点的横坐标：传入值%总长度 + 窗口起点坐标
	
	if(osc1->cursor_x < osc1->last_cursor_x)	//如果开始新的一页
	{
		osc1->y_max_last = osc1->y_max;
		osc1->y_min_last = osc1->y_min;
		osc1->y_min = y_raw;	//重置y轴最小值
		osc1->y_max = y_raw;	//重置y轴最大值
		if(osc1->mode == 0)		//自适应幅值模式
		{
			osc1->y_ratio = (float)osc1->width/(float)(osc1->y_max_last-osc1->y_min_last);	//更新y轴缩放，由上一页的最大值和最小值算出
			osc1->y_ratio -= osc1->y_ratio*0.2f;	//界面上下留20%的空白
		}
	}
	
	if(y_raw < osc1->y_min)
	{
		osc1->y_min = y_raw;	//更新y轴历史最小值
	}
	if(y_raw > osc1->y_max)
	{
		osc1->y_max = y_raw;	//更新y轴历史最大值
	}
	if(osc1->last_cursor_x != 0)//第一个点刷新时osc1->last_cursor_x = 0，导致边框被覆盖，这里跳过第一个点
	{
		if(osc1->y_ratio > 0)	//ratio值有效才进行刷新，幅值自适应模式下，第一页的刷新会被跳过（用以计算合适的缩放比例）
		{
			if(osc1->mode == 0)		//幅值自适应模式
				osc1->y_bias = osc1->y_min_last;	//y轴偏移量 = 上一页最小值
			
			// 下式第一项：下边界留10%空白；第二项：(y-y_bias)*y_ratio
			osc1->cursor_y = (osc1->loca_y + (float)osc1->width*0.9f) - (float)(y_raw-osc1->y_bias)*osc1->y_ratio;
			if(osc1->cursor_y < osc1->loca_y)
			{
				osc1->cursor_y = osc1->loca_y;
			}
			if(osc1->cursor_y > osc1->loca_y+osc1->width)
			{
				osc1->cursor_y = osc1->loca_y+osc1->width;
			}
	//		LCD_DrawLine(osc1->cursor_x,osc1->loca_y,osc1->cursor_x,osc1->loca_y+osc1->width,BLACK);//清空上次波形
			LCD_Fill(osc1->cursor_x,osc1->loca_y,osc1->cursor_x+1,osc1->loca_y+osc1->width,BLACK);	//清空上次波形(效率比上面的高)

			if(osc1->cursor_x > osc1->last_cursor_x)
			{
				if(osc1->last_cursor_y <= osc1->cursor_y)	//画线函数有缺陷，需要处理成终点>=起点的形式
				{
					LCD_DrawLine(osc1->last_cursor_x,osc1->last_cursor_y,osc1->cursor_x,osc1->cursor_y,GREEN);				
				}
				else
				{
					LCD_DrawLine(osc1->last_cursor_x,osc1->cursor_y,osc1->cursor_x,osc1->last_cursor_y,GREEN);				
				}
			}
		}
	}
	osc1->last_cursor_x = osc1->cursor_x;	//更新上次的刷新x坐标
	osc1->last_cursor_y = osc1->cursor_y;	//更新上次的刷新y坐标
}
