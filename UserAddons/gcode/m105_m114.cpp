#include "globalvariables_ccmram.h"
#include "user_debug.h"
#include "temperature.h"
#include "user_interface.h"
#include "Configuration.h"
#include "stepper.h"
#include "gcode.h"
#include "gcode_global_params.h"

#include <stdio.h>

namespace gcode
{
  //TIM3、TIM4优先级为6；USART1优先级为11(最原始)
  //Q的U盘无界面，YQ在Q的基础上显示正常触摸屏界面，
  void m105_process(void)
  {
    if(!t_set_targeted_hotend(105))
    {
      if(gpio_is_temp0_pin_exit())
      {
        USER_EchoLogStr("ok T:%.1f /%.1f",sg_grbl::temperature_get_extruder_current(0), sg_grbl::temperature_get_extruder_target(0));
      }
      else if(gpio_is_temp1_pin_exit())
      {
        for (int8_t cur_extruder = 0; cur_extruder < EXTRUDERS; ++cur_extruder)
        {
          USER_EchoLogStr("ok T%d:%.1f /%.1f",cur_extruder, sg_grbl::temperature_get_extruder_current((uint8_t)cur_extruder),
                          sg_grbl::temperature_get_extruder_target((uint8_t)cur_extruder));
        }
      }
      else
      {
        USER_ErrLog("Error:No thermistors - no temperature\n");
        USER_EchoLogStr(" @:%d B@:%d\n",sg_grbl::temperature_get_extruder_heater_power(tmp_extruder), sg_grbl::temperature_get_bed_heater_power());
        return;
      }

      if(gpio_is_bedtemp_pin_exit())
      {
        USER_EchoLogStr(" B:%.1f /%.1f",sg_grbl::temperature_get_bed_current(), sg_grbl::temperature_get_bed_target());
      }

      //腔体温度，先用热床温度代替20170906
      if(1U == t_sys_data_current.enable_cavity_temp)
      {
        USER_EchoLogStr(" C:%.1f /%.1f",sg_grbl::temperature_get_cavity_current(), sg_grbl::temperature_get_cavity_target());
      }
      USER_EchoLogStr("\n");
    }
  }

  void m114_process(void)
  {
    USER_EchoLogStr("ok X:%.2f Y:%.2f Z:%.2f E:%.2f ",sg_grbl::st_get_position_length(X_AXIS),
                    sg_grbl::st_get_position_length(Y_AXIS), sg_grbl::st_get_position_length(Z_AXIS), sg_grbl::st_get_position_length(E_AXIS));
    if(0U != t_sys_data_current.enable_color_mixing)
    {
      USER_EchoLogStr("B:%.2f ", sg_grbl::st_get_position_length(B_AXIS));
    }
  }
}









