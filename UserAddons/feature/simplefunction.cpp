#include "simplefunction.h"
#include "globalvariables.h"

#include "temperature.h"
#include "planner.h"
#include "user_interface.h"

#include "stm32f4xx_hal.h"
#include "config_motion_3d.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "PrintControl.h"


#ifdef __cplusplus
} //extern "C" {
#endif

SimpleFunction::SimpleFunction()
{
  t_gui_p.doorStatus = 0;
}

void SimpleFunction::process(void)
{
  control_motor_fan(); // 电机风扇控制
  if(motion_3d.enable_check_door_open) // 有门检测功能，开启门检测
  {
    control_check_door_open();
  }
}

// 门检测
void SimpleFunction::control_check_door_open(void)
{
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15))   //M14R03，M14S 门检测占用了ZMAX PIN接口
  {
    //door open
    t_gui_p.IsDoorOpen=1;
    t_gui_p.doorStatus=1;
  }
  else
  {
    //door close
    t_gui_p.IsDoorOpen=0;
    t_gui_p.doorStatus=0;
  }
}

// 电机风扇控制
void SimpleFunction::control_motor_fan(void)
{
  static bool motor_fan_status = false;
  static UINT8 Open_Fan_temp = 50;

  if(sg_grbl::planner_moves_planned() || sg_grbl::temperature_get_extruder_current(0)>=Open_Fan_temp)
  {
    if(motor_fan_status)
      return;

    if(2 == t_sys_data_current.enable_v5_extruder)
      gcode::set_fan_speed(255);

    gpio_e_motor_fan_control(true);
    gpio_board_fan_control(true);

    motor_fan_status = true;
  }
  else
  {
    if(!motor_fan_status)
      return;

    if(2 == t_sys_data_current.enable_v5_extruder)
      gcode::set_fan_speed(0);

    gpio_e_motor_fan_control(false);
    gpio_board_fan_control(false);

    motor_fan_status = false;
  }
}

///////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
SoundControl::SoundControl()
{

}

/**
 * [SoundControl::beep 提示音]
 * @Author   bingo
 * @DateTime 2017-11-11
 * @param    time       [description]
 */
void SoundControl::beep(uint16_t time)
{
  static ULONG timeout_beeper = 0;
  if ( 0 == time )
  {
    gpio_beep_control(false);
    return;
  }
  if ( timeout_beeper < sys_task_get_tick_count() )
  {
    timeout_beeper = sys_task_get_tick_count() + time;
    gpio_beep_control(!t_gui_p.isOpenBeep);
  }
}

/**
 * [SoundControl::buzz 蜂鸣器]
 * @Author   bingo
 * @DateTime 2017-11-11
 * @param    msticks    [description]
 */
void SoundControl::buzz(uint16_t msticks)
{
  gpio_beep_control(true);
  (void)sys_os_delay(msticks);
  gpio_beep_control(false);
  (void)sys_os_delay(msticks);
}

/**
 * [SoundControl::beepAlarm 报警声]
 * @Author   bingo
 * @DateTime 2017-11-11
 */
void SoundControl::beepAlarm(void)
{
  static bool IsCloseBeep = true;
  if (1 == t_gui_p.isBeepAlarm)
  {
    beep(500);
    IsCloseBeep = false;
  }
  else
  {
    if (!IsCloseBeep)
    {
      beep(0);
      IsCloseBeep = true;
    }
  }
}

SimpleFunction simpleFunction;
SoundControl soundControl;
