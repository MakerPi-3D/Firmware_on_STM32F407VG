#include "gcodebufferhandle.h"
#include "Configuration.h"
#include "ConfigurationStore.h"
#include "globalvariables_ccmram.h"
#include "gcode_global_params.h"

#include <stdint.h>

namespace gcode
{
  void m200_process(void)
  {
#if 0
    FLOAT area = 0.0F;
#endif
    if(parseGcodeBufHandle.codeSeen('D'))
    {
      const float radius = parseGcodeBufHandle.codeValue() * 0.5F; // float
      if(radius == 0)
      {
#if 0
        area = 1;
#endif
      }
      else
      {
#if 0
        area = PI * pow(radius, 2);
#endif
      }
      tmp_extruder = active_extruder;
      if(parseGcodeBufHandle.codeSeen('T'))
      {
        tmp_extruder = static_cast<int16_t>(parseGcodeBufHandle.codeValueLong());
        if(tmp_extruder >= EXTRUDERS)
        {
          // return
        }
      }
      //volumetric_multiplier[tmp_extruder] = 1 / area;
    }
    else
    {
      /*reserved for setting filament diameter via UFID or filament measuring device*/
    }

  }

  void m201_process(void)
  {
    for(int16_t i=0; i < motion_3d.axis_num; ++i)
    {
      if(parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        sg_grbl::max_acceleration_units_per_sq_second[i] = static_cast<uint32_t>(parseGcodeBufHandle.codeValueLong());
      }
    }
    // steps per sq second need to be updated to agree with the units per sq second (as they are what is used in the planner)
    sg_grbl::reset_acceleration_rates();
  }

  void m203_process(void)
  {
    for(int16_t i=0; i < motion_3d.axis_num; ++i)
    {
      if(parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        sg_grbl::max_feedrate[i] = parseGcodeBufHandle.codeValue();
      }
    }
  }

  void m204_process(void)
  {
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      sg_grbl::acceleration = parseGcodeBufHandle.codeValue() ;
    }
    if(parseGcodeBufHandle.codeSeen('T'))
    {
      sg_grbl::retract_acceleration = parseGcodeBufHandle.codeValue() ;
    }
  }

  void m205_process(void)
  {
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      sg_grbl::minimumfeedrate = parseGcodeBufHandle.codeValue();
    }
    if(parseGcodeBufHandle.codeSeen('T'))
    {
      sg_grbl::mintravelfeedrate = parseGcodeBufHandle.codeValue();
    }
    if(parseGcodeBufHandle.codeSeen('B'))
    {
      sg_grbl::minsegmenttime = static_cast<uint32_t>(parseGcodeBufHandle.codeValueLong()) ;
    }
    if(parseGcodeBufHandle.codeSeen('X'))
    {
      sg_grbl::max_xy_jerk = parseGcodeBufHandle.codeValue() ;
    }
    if(parseGcodeBufHandle.codeSeen('Z'))
    {
      sg_grbl::max_z_jerk = parseGcodeBufHandle.codeValue() ;
    }
    if(parseGcodeBufHandle.codeSeen('E'))
    {
      sg_grbl::max_e_jerk = parseGcodeBufHandle.codeValue() ;
    }
  }
}









