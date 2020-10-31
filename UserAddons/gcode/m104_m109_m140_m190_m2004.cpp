#include "stepper.h"
#include "gcodebufferhandle.h"
#include "temperature.h"
#include "globalvariables.h"
#include "sys_function.h"
#include "user_debug.h"
#include "config_model_tables.h"
#include "gcode.h"
#include "gcode_global_params.h"

#include <stdint.h>

namespace gcode
{
  static volatile bool is_heat_not_wait_sync = false;    /*!< 设置温度不等待队列完成标志 */
  static long respond_temp_status_time_count = 0L;
  static uint32_t respond_temp_status_peroid = 0UL;
  static bool is_cool_down_not_wait = true;              /*!< 冷却不等待 */
  static bool target_direction;                          /*!<  */
  static bool cancel_heatup = false;                     /*!< 是否取消加热 */

  extern "C" void manage_synchronize(void);

  void m104_process(void)
  {
    if(!t_set_targeted_hotend(104))
    {
      if(!is_heat_not_wait_sync)
      {
        sg_grbl::st_synchronize();
      }
      if (parseGcodeBufHandle.codeSeen('S'))
      {
        sg_grbl::temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), tmp_extruder);
      }
    }
  }

  void m109_process(void)
  {
    if(!t_set_targeted_hotend(109))
    {
      t_gui_p.m109_heating_complete = 0U;
      if (parseGcodeBufHandle.codeSeen('S'))
      {
        sg_grbl::temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), tmp_extruder);
        is_cool_down_not_wait = true;
      }
      else if (parseGcodeBufHandle.codeSeen('R'))
      {
        sg_grbl::temperature_set_extruder_target(parseGcodeBufHandle.codeValue(), tmp_extruder);
        is_cool_down_not_wait = false;
      }

      /* See if we are heating up or cooling down */
      target_direction = sg_grbl::temperature_is_extruder_heating(tmp_extruder); // true if heating, false if cooling

      cancel_heatup = false;

      while ( target_direction ? (sg_grbl::temperature_is_extruder_heating(tmp_extruder)) : (sg_grbl::temperature_is_extruder_cooling(tmp_extruder)&&(!is_cool_down_not_wait)) )
      {
        USER_EchoLogStr("T:%.1f E:%d\n",sg_grbl::temperature_get_extruder_current(0),tmp_extruder);
        // 秒数累加
        if ( respond_temp_status_peroid < sys_task_get_tick_count() )
        {
          respond_temp_status_peroid = sys_task_get_tick_count() + 1000U;
          ++respond_temp_status_time_count;
        }
        if(2 < respond_temp_status_time_count)
        {
          respond_temp_status_time_count = 0;
        }
        manage_synchronize();
        sys_os_delay(100U);
      }
      t_gui_p.m109_heating_complete = 1U;
      previous_xTaskGetTickCount_cmd = sys_task_get_tick_count();
    }
  }

  void m140_process(void)
  {
    if(!is_heat_not_wait_sync)
    {
      sg_grbl::st_synchronize();
    }
    if(t_sys_data_current.model_id != M14) //M14没有热床
    {
      if (parseGcodeBufHandle.codeSeen('S'))
      {
        sg_grbl::temperature_set_bed_target(parseGcodeBufHandle.codeValue());
      }
    }
  }

  void m190_process(void)
  {
    t_gui_p.m190_heating_complete = 0U;
    if(t_sys_data_current.model_id != M14) //M14没有热床
    {
      if (parseGcodeBufHandle.codeSeen('S'))
      {
        sg_grbl::temperature_set_bed_target(parseGcodeBufHandle.codeValue());
        is_cool_down_not_wait = true;
      }
      else if (parseGcodeBufHandle.codeSeen('R'))
      {
        sg_grbl::temperature_set_bed_target(parseGcodeBufHandle.codeValue());
        is_cool_down_not_wait = false;
      }

      cancel_heatup = false;
      target_direction = sg_grbl::temperature_is_bed_heating(); // true if heating, false if cooling

      while ( ((target_direction)&&(!cancel_heatup)) ? (sg_grbl::temperature_is_bed_heating()) : (sg_grbl::temperature_is_bed_cooling()&&(!is_cool_down_not_wait)) )
      {
        printf("T:%.1f E:%d B:%.1f\n",sg_grbl::temperature_get_extruder_current(0),tmp_extruder,sg_grbl::temperature_get_bed_current());
        // 秒数累加
        if ( respond_temp_status_peroid < sys_task_get_tick_count() )
        {
          respond_temp_status_peroid = sys_task_get_tick_count() + 1000U;
          ++respond_temp_status_time_count;
        }
        if(2 < respond_temp_status_time_count)
        {
          respond_temp_status_time_count = 0;
        }
        manage_synchronize();
        sys_os_delay(100U);
      }
      previous_xTaskGetTickCount_cmd = sys_task_get_tick_count();
    }
    t_gui_p.m190_heating_complete = 1U;
  }

  void m2004_process(void)
  {
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      if(parseGcodeBufHandle.codeValueLong() == 0)
      {
        is_heat_not_wait_sync = false;
      }
      else
      {
        is_heat_not_wait_sync = true;
      }
    }
  }

}










