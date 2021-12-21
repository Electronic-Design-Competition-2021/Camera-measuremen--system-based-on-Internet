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

#include "menu.h"
#include "stdio.h"

/**
  * @brief          菜单初始化
  * @note
  * @param[in]      menu_s：	菜单指针
  * @param[in]      init_func：	初始界面函数指针
  */
void menuInit(menu_s_t* menu_s,void (*init_func)(void))
{
    if(init_func != 0)
    {
        menu_s->menu_func_p = init_func;
        menu_s->refresh_flag = 1;
		menu_s->init_flag = 1;
    }
}


/**
  * @brief          菜单跳转
  * @note			通过切换函数指针实现，对切换操作进行记录，用以返回时回溯
  * @param[in]      menu_s：	菜单指针
  * @param[in]      new_func：  目标界面函数指针
  */
void menuSkip(menu_s_t* menu_s,void (*new_func)(void))
{
	uint8_t i = 0;
    if(new_func != 0 && menu_s->list_head < MAX_DEGREE)
    {
		if(new_func == menu_s->menu_func_p)	//目标界面为当前界面
			return;	//不需要跳转
		
		//遍历查找返回列表中的历史界面是否含有当前的目标界面
		//如果包含，则回溯到历史界面（此操作可以防止操作不良导致的返回时在重复的界面来回跳的问题）
		for(i = 0;i < menu_s->list_head;i++)
		{
			if(new_func == menu_s->menu_func_return_list[i])	
			{
				menu_s->list_head = i;			//截断返回列表
				menu_s->menu_func_p = new_func;	//切换界面
				menu_s->refresh_flag = 1;		//刷新标志位置位
				menu_s->init_flag = 1;			//界面切换标志位置位		
				return;
			}
		}
        menu_s->menu_func_return_list[menu_s->list_head++] = menu_s->menu_func_p; //记录跳转操作
        menu_s->menu_func_p = new_func;	//跳转为新界面函数
        menu_s->refresh_flag = 1;
		menu_s->init_flag = 1;
    }
}

/**
  * @brief          菜单返回
  * @note			回溯跳转过程，按顺序返回（已忽略重复跳转的返回路径）
  * @param[in]      menu_s：	菜单指针
  */
void menuReturn(menu_s_t* menu_s)
{
    if(menu_s->list_head != 0)
    {
        menu_s->list_head--;
        menu_s->menu_func_p = menu_s->menu_func_return_list[menu_s->list_head]; //跳转
        menu_s->refresh_flag = 1;
		menu_s->init_flag = 1;
    }
}

/**
  * @brief          菜单刷新标志位置位
  * @note			仅在标志位被置位时刷新菜单，减少不必要的系统开销
  * @param[in]      menu_s：	菜单指针
  */
void menuRefreshFlagSet(menu_s_t* menu_s)
{
	menu_s->refresh_flag = 1;
}

/**
  * @brief          菜单刷新
  * @note			
  * @param[in]      menu_s：	菜单指针
  */
void menuRefresh(menu_s_t* menu_s)
{
    if(menu_s->refresh_flag == 1)
    {
        menu_s->refresh_flag = 0;
        menu_s->menu_func_p();  	//调用显示函数，显示页面
    }
}
