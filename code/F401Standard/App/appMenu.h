/**
  ****************************(C) COPYRIGHT 2019 DOEE****************************
  * @file      	appMenu.c/h
  * @brief     	多级菜单（用户层）
  * @note      	每个菜单界面以一个独立函数的形式被调用，新建菜单仅需新建一个函数，
				通过调用menuSkip实现菜单跳转，相关函数见menu.c/h
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
 
#ifndef __APP_MENU_H__
#define __APP_MENU_H__
#include "struct_typedef.h"
#include "menu.h"


#define LINE_NUM 9

extern menu_s_t GUI;	//定义菜单
void menu_main(void);
void menu_osc(void);
void menu1(void);
void menu2(void);
void menu3(void);
void menu4(void);
#endif
