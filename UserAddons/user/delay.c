#include "delay.h"
//#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include "threed_engine.h"
#include "user_interface.h"

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
static UINT8  fac_us=0;//us延时倍乘数
static UINT16 fac_ms=0;//ms延时倍乘数,在ucos下,代表每个节拍的ms数

void HAL_Delay(__IO UINT32 Delay)
{
  (void)sys_os_delay(Delay);
}
//初始化延迟函数
//当使用ucos的时候,此函数会初始化ucos的时钟节拍
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init(UINT8 SYSCLK)
{
  fac_us=SYSCLK/8;
  fac_ms=1;
}

//延时nus
//nus:要延时的us数.
void delay_us(UINT32 nus)
{
  UINT32 ticks;
  UINT32 told,tnow,tcnt=0;
  UINT32 reload=SysTick->LOAD;	//LOAD的值
  ticks=nus*fac_us; 			//需要的节拍数
  vTaskSuspendAll();
  told=SysTick->VAL;        	//刚进入时的计数器值
  for(;;)
  {
    tnow=SysTick->VAL;
    if(tnow!=told)
    {
      if(tnow<told)
      {
        tcnt=tcnt+(told-tnow);//这里注意一下SYSTICK是一个递减的计数器就可以了.
      }
      else
      {
        tcnt=tcnt+((reload-tnow)+told);
      }
      told=tnow;
      if(tcnt>=ticks)
      {
        break;//时间超过/等于要延迟的时间,则退出.
      }
    }
  };
  (void)xTaskResumeAll();
}
//延时nms
//nms:要延时的ms数
void delay_ms(UINT16 nms)
{
  if(nms>=fac_ms)//延时的时间大于ucos的最少时间周期
  {
    (void)sys_os_delay(nms/fac_ms);
  }
  nms%=fac_ms;				//ucos已经无法提供这么小的延时了,采用普通方式延时
  delay_us((UINT32)(nms*1000));		//普通方式延时
}




































