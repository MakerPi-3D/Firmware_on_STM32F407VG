#include "gcodebufferhandle.h"
#include "gcode.h"
#include "gcode_global_params.h"

#include <stdint.h>

namespace gcode
{

  void m220_process(void)
  {
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      feed_multiply = static_cast<int>(parseGcodeBufHandle.codeValueLong()) ;
    }
  }

  void m221_process(void)
  {
    if(!t_set_targeted_hotend(221))
    {
      if(parseGcodeBufHandle.codeSeen('S'))
      {
        if (parseGcodeBufHandle.codeSeen('T'))
        {
          //extruder_multiply[tmp_extruder] = static_cast<INT>(parseGcodeBufHandle.codeValueLong());
        }
        else
        {
          extruder_multiply = static_cast<int>(parseGcodeBufHandle.codeValueLong());
        }
      }
    }
  }
}









