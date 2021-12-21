/**
  ****************************(C) COPYRIGHT 2021 DOEE****************************
  * @file      	menu.c/h
  * @brief     	多级菜单（底层）
  * @note      	每个菜单界面以一个独立函数的形式被调用，新建菜单仅需新建一个函数，
				菜单间切换通过切换函数指针实现，菜单经过初始化后，
				需要不断调用menuRefresh刷新
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

#ifndef __MENU_H__
#define __MENU_H__
#include "struct_typedef.h"

#define MAX_DEGREE 20   //最大菜单深度

typedef struct
{
    void (*menu_func_p)(void);          //菜单显示函数指针
    uint8_t refresh_flag;               //刷新标志位
    void (*menu_func_return_list[MAX_DEGREE])(void);    //返回菜单列表
    uint8_t list_head;      //列表头部
    uint8_t cursor;         //光标位置
	uint8_t init_flag;		//初始化标志:发生菜单切换时被置位（需手动清零）
    //uint8_t list_free_num;  //列表剩余空间

} menu_s_t;

void menuInit(menu_s_t* menu_s,void (*init_func)(void));// GUI初始化
void menuRefresh(menu_s_t* menu_s);	// GUI刷新函数
void menuRefreshFlagSet(menu_s_t* menu_s);// GUI刷新标志位置位
void menuSkip(menu_s_t* menu_s,void (*new_func)(void));	//界面跳转
void menuReturn(menu_s_t* menu_s);			//界面返回
#endif


