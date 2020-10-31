#include "gcodebufferhandle.h"
#include "planner.h"
#include "sys_function.h"
#include "gcode_global_params.h"

namespace gcode
{
  extern "C" float Laser_Print_Position;

  void g92_process(void)
  {
    uint32_t isNotProcessAutoBedL = 0U;
    if(parseGcodeBufHandle.codeSeen('A'))
    {
      isNotProcessAutoBedL=(uint32_t)parseGcodeBufHandle.codeValueLong();
      if(isNotProcessAutoBedL==1U)
      {
        sg_grbl::plan_set_process_auto_bed_level_status(false);
      }
    }
    if((!parseGcodeBufHandle.codeSeen(axis_codes[E_AXIS])) || (!parseGcodeBufHandle.codeSeen(axis_codes[B_AXIS])))
    {
      while(sg_grbl::planner_moves_planned() > 0)
      {
        sys_os_delay(50);
      }
    }
    for(uint8_t i=0U; i < motion_3d.axis_num; ++i)
    {
      if(parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        if((i == E_AXIS) || (i == B_AXIS))
        {
          printing_material_length+=(uint32_t)current_position[i];
          current_position[i] = parseGcodeBufHandle.codeValue();
          sg_grbl::planner_set_axis_position(current_position[i], static_cast<int32_t>(i));
        }
        else
        {
          current_position[i] = parseGcodeBufHandle.codeValue()+add_homing[i]; // float
          if(t_sys_data_current.IsLaser)
            current_position[Z_AXIS] = Laser_Print_Position;
          if(current_position[Z_AXIS] > t_sys_data_current.poweroff_rec_z_max_value)
            current_position[Z_AXIS] = t_sys_data_current.poweroff_rec_z_max_value;
          sg_grbl::planner_set_position(current_position);
        }
      }
    }
    if(parseGcodeBufHandle.codeSeen('A'))
    {
      if(isNotProcessAutoBedL==1U)
      {
        sg_grbl::plan_set_process_auto_bed_level_status(true);
      }
    }
  }


}







