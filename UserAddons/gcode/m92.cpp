#include "globalvariables_ccmram.h"
#include "gcodebufferhandle.h"
#include "ConfigurationStore.h"
#include "gcode_global_params.h"

#include <stdint.h>

namespace gcode
{
  void m92_process(void)
  {
    for(int8_t i=0; i < motion_3d.axis_num; ++i)
    {
      if(parseGcodeBufHandle.codeSeen(axis_codes[i]))
      {
        if(i == E_AXIS || i == B_AXIS)
        {
          // E B
          const float value = parseGcodeBufHandle.codeValue();
          if(value < 20.0F)
          {
            const float factor = sg_grbl::axis_steps_per_unit[i] / value; // increase e constants if M92 E14 is given for netfab.
            sg_grbl::max_e_jerk *= factor;
            sg_grbl::max_feedrate[i] *= factor; // float
            sg_grbl::axis_steps_per_sqr_second[i] *= (uint32_t)factor;
          }
          sg_grbl::axis_steps_per_unit[i] = value;
        }
        else
        {
          sg_grbl::axis_steps_per_unit[i] = parseGcodeBufHandle.codeValue();
        }
      }
    }
  }
}








