#include "materialcheck.h"
#include "stm32f4xx_hal.h"
#include "globalvariables.h"
#include "PrintControl.h"
#include "user_debug.h"
#include "sysconfig_data.h"
#include "threed_engine.h"
#include "Alter.h"
#include "config_model_tables.h"
#include "user_interface.h"
#include "planner.h"
/////////////////////////////////////////////////////////////////////////////////////
///////////////////////// MaterialCheck    start         ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

  static void Check_NotHaveMat(void)
  {
    static int8_t nomat_cnt = 0;
    static uint32_t NotHaveMatTimeOut = 0;
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
    {
      sys_os_delay(10);//防抖
      if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
      {
        if(NotHaveMatTimeOut<sys_task_get_tick_count())
        {
          ++nomat_cnt;
          NotHaveMatTimeOut=sys_task_get_tick_count()+3000;
        }
        if(nomat_cnt>3)
        {
          USER_EchoLogStr("IsNotHaveMatInPrint\r\n");
          t_gui_p.IsNotHaveMatInPrint=1; //在打印的时候没料了
          printControl.pause(false); //暂停打印
          nomat_cnt=0;
        }
      }
    }
    else
    {
      nomat_cnt=0;
    }
  }

#ifdef __cplusplus
} // extern "C" {
#endif

MaterialCheck::MaterialCheck()
{

}

void MaterialCheck::init(void)
{
  if(t_sys_data_current.enable_material_check) //有断料检测功能
  {
    if(t_sys_data_current.model_id == M41G)
    {
      Mat_Cut_Init();
    }
    else
    {
      // 默认PA5注册为断料检测ADC引脚，如果开启堵料检测，需要复位PA5
      gpio_material_check_init();
    }
  }
}

void MaterialCheck::process(void)
{
  if(t_sys_data_current.enable_material_check) //有断料检测功能
  {
    if(IsPrint() && (1U == t_gui_p.m109_heating_complete) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE)) //打印开始且加热完成归零后 才去检测是否有料
    {
      if(t_sys_data_current.model_id == M41G)
      {
        if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14))
        {
          sys_os_delay(10);//防抖
          if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14))
          {
            USER_EchoLogStr("IsNotHaveMatInPrint\r\n");
            t_gui_p.IsNotHaveMatInPrint=1; //在打印的时候没料了
            printControl.pause(false); //暂停打印
          }
        }
      }
      else if(t_sys_data_current.model_id == K5)
      {
        Check_NotHaveMat();
      }
      else
      {
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
        {
          sys_os_delay(10);//防抖
          if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
          {
            USER_EchoLogStr("IsNotHaveMatInPrint\r\n");
            t_gui_p.IsNotHaveMatInPrint=1; //在打印的时候没料了
            printControl.pause(false); //暂停打印
          }
        }
      }
    }
  }
}

MaterialCheck materialCheck;
