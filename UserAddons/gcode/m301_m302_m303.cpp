#include "Configuration.h"
#include "gcodebufferhandle.h"
#include "temperature_pid_temp.h"
#include "temperature.h"
#include "globalvariables_ccmram.h"

namespace gcode
{
  void m301_process(void)
  {
#ifdef PIDTEMP
    if(parseGcodeBufHandle.codeSeen('P'))
    {
      sg_grbl::temp_pid_extruder_set_kp(parseGcodeBufHandle.codeValue());
    }
    if(parseGcodeBufHandle.codeSeen('I'))
    {
      sg_grbl::temp_pid_extruder_set_ki(parseGcodeBufHandle.codeValue());
    }
    if(parseGcodeBufHandle.codeSeen('D'))
    {
      sg_grbl::temp_pid_extruder_set_kd(parseGcodeBufHandle.codeValue());
    }
#ifdef PID_ADD_EXTRUSION_RATE
    if(parseGcodeBufHandle.codeSeen('C'))
    {
      temp_pid_extruder_set_kc(parseGcodeBufHandle.codeValue());
    }
#endif
    sg_grbl::temp_pid_extruder_update();
#endif // #ifdef PIDTEMP
  }

  void m302_process(void)
  {
#ifdef PREVENT_DANGEROUS_EXTRUDE
    if (parseGcodeBufHandle.codeSeen('S'))
    {
      const float temp = parseGcodeBufHandle.codeValue();
#ifdef PREVENT_DANGEROUS_EXTRUDE
      motion_3d.extrude_min_temp=temp;
#endif
    }
//	else //输出当前最小温度
#endif
  }

  void m303_process(void)
  {
    float temp = 150.0F;
    int16_t e=0;
    int16_t c=5;
    if (parseGcodeBufHandle.codeSeen('E'))
    {
      e= static_cast<int16_t>(parseGcodeBufHandle.codeValueLong());
    }
    if (e<0)
    {
      temp=70.0F;
    }
    if (parseGcodeBufHandle.codeSeen('S'))
    {
      temp=parseGcodeBufHandle.codeValue();
    }
    if (parseGcodeBufHandle.codeSeen('C'))
    {
      c=static_cast<int16_t>(parseGcodeBufHandle.codeValueLong());
    }
    sg_grbl::temperature_PID_autotune(temp, e, c);
  }
}








