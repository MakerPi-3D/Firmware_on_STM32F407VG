#include "gcodebufferhandle.h"
#include "sg_util.h"
#include "gcode_global_params.h"

namespace gcode
{
  void m106_process(void)
  {
    int tmp_fan_speed = 0;
    if (parseGcodeBufHandle.codeSeen('S'))
    {
      const int32_t fanspeed_codevalue = static_cast<int32_t>(parseGcodeBufHandle.codeValueLong());
      tmp_fan_speed = sg_util::constrain_p(fanspeed_codevalue,0,255);
    }
    else
    {
      tmp_fan_speed=255;
    }
    set_fan_speed(tmp_fan_speed);
  }

  void m107_process(void)
  {
    set_fan_speed(0);
  }

}








