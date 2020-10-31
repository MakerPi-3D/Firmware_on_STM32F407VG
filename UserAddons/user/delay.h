#ifndef M_DELAY_H
#define M_DELAY_H

#include "wbtypes.h"
//#include <sys.h>
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//使用SysTick的普通计数模式对延迟进行管理(支持ucosii)
//包括delay_us,delay_ms
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/5/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//********************************************************************************
//修改说明
//无
//////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

  void delay_init(UINT8 SYSCLK);
  void delay_ms(UINT16 nms);
  void delay_us(UINT32 nus);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif





























