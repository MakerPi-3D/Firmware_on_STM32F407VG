#include "lightcontrol.h"
#include "stm32f4xx_hal.h"
#include "globalvariables.h"
#include "temperature.h"
#include "planner.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "config_model_tables.h"
#include "user_interface.h"
#ifdef __cplusplus
extern "C" {
#endif

  extern INT IsPrint(void);
  extern INT IsHeating(void);

#ifdef __cplusplus
} // extern "C" {
#endif

LightControl::LightControl()
{

}

/**
 * [LightControl::process 灯控制执行入口]
 * @Author   bingo
 * @DateTime 2017-11-11
 */
void LightControl::process(void)
{
  core_board_led(); // 核心板LED一直开启
  if(0U != t_custom_services.enable_led_light) // 有LED灯条功能，开启LED灯条照明
  {
    led_light();
  }
  if((0U != t_custom_services.enable_warning_light) &&
      (0U != motion_3d.enable_check_door_open)) // 有警示灯和门检测功能，开启警示灯
  {
    caution_light();
  }
}

/**
 * [LightControl::core_board_led 核心板LED灯控制]
 * @Author   bingo
 * @DateTime 2017-11-11
 */
void LightControl::core_board_led(void)
{
  static ULONG led_last_time = 0UL;

  if(led_last_time < sys_task_get_tick_count())
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
    led_last_time = sys_task_get_tick_count() + 500U; // 500ms闪烁一次
  }
}

/**
 * [LightControl::led_light LED灯条控制]
 * @Author   bingo
 * @DateTime 2017-11-11
 */
extern bool SetLedValue;
void LightControl::led_light(void)
{
  static UINT8 LEDLight_status = 0U;
  static ULONG LEDLight_timeoutToStatus = 0UL;
  if((0 != sg_grbl::planner_moves_planned()) || (0 != IsHeating())) // 移动或加热，灯条开启
  {
    if( LEDLight_status==0U )
    {
      SetLedValue=true;
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
      LEDLight_status = 1U;
    }
    LEDLight_timeoutToStatus = sys_task_get_tick_count() + (60UL*1000UL); //5 minute
  }
  else
  {
    if( (LEDLight_status==1U) && (LEDLight_timeoutToStatus<sys_task_get_tick_count()))
    {
      SetLedValue=false;
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
      LEDLight_status = 0U;
    }
  }
}

void LightControl::led_switch(void)
{
  if(SetLedValue)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
  }
}

/**
 * [LightControl::caution_light_twinkle M14R03警示灯闪烁]
 * @Author   bingo
 * @DateTime 2017-11-11
 * @param    DelayTime  [闪烁时间]
 */
void LightControl::caution_light_twinkle(UINT DelayTime)
{
  static ULONG CautionLightTimeControl=0UL;
  if(CautionLightTimeControl<sys_task_get_tick_count())
  {
    if(t_sys_data_current.model_id != M41G)
    {
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
    }
    CautionLightTimeControl=sys_task_get_tick_count()+DelayTime;
  }
}

/**
 * [LightControl::caution_light M14R03警示灯控制]
 * @Author   bingo
 * @DateTime 2017-11-11
 */
void LightControl::caution_light(void)
{
  static UINT8 LastStatus=0U;
  static bool doorOpenBeep = false;//标志门检打开了蜂鸣器
  if((0U != t_gui_p.doorStatus) && (0 != IsPrint())) //打印中门未关闭
  {
    caution_light_twinkle(100U);//0.1S闪烁
    LastStatus=1U;
    t_gui_p.IsDisplayDoorOpenInfo=0U;
  }
  else if((0 != IsPrint()) && (1U == t_gui_p.m109_heating_complete)) //加热完成并开始打印
  {
    if(t_sys_data_current.model_id != M41G)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);  //常亮
    }
    LastStatus=2U;
    t_gui_p.IsDisplayDoorOpenInfo=0U;
  }
  else if((sg_grbl::temperature_get_extruder_current(0)>60.0F) || (0 != IsPrint())) //温度大于60度 或 打印前的加热阶段
  {
    if(0U != t_gui_p.doorStatus)
    {
      t_gui_p.isBeepAlarm = 1;
      gpio_beep_control(false);
      doorOpenBeep = true;
      caution_light_twinkle(100U);//0.1S闪烁
    }
    else
    {
      if(doorOpenBeep)
      {
        t_gui_p.isBeepAlarm = 0;
        doorOpenBeep = false;
      }
      caution_light_twinkle(500U);//0.5S闪烁
    }

    if((sg_grbl::temperature_get_extruder_current(0)>60.0F) && (0==IsPrint()) && (0U!=t_gui_p.doorStatus)) //待机、预热、进丝、退丝 时 温度大于60度且门打开显示提示信息
    {
      t_gui_p.IsDisplayDoorOpenInfo=1U;
    }
    else
    {
      t_gui_p.IsDisplayDoorOpenInfo=0U;
    }

    LastStatus=3U;
  }
  else //待机且温度小于60度
  {
    if((LastStatus==3U) && (0U!=t_gui_p.doorStatus))
    {
      t_gui_p.isBeepAlarm = 0;
    }
    if(t_sys_data_current.model_id != M41G)
    {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);  //常灭
    }
    LastStatus=4U;
    t_gui_p.IsDisplayDoorOpenInfo=0U;
  }
}

LightControl lightControl;
