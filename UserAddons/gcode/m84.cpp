#include "stepper.h"
#include "gcodebufferhandle.h"
#include "globalvariables_ccmram.h"
#include "gcode_global_params.h"

namespace gcode
{
  /**
   * [process_m84 解锁电机]
   * M84 X Y Z E B S1\r\n
   * 参数X、Y、Z、E、B 代表对应的电机
   * 参数S1 代表1S后解锁电机，不带S参数代表立即解锁电机
   */
  void m84_process(void)
  {
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      stepper_inactive_time = (uint32_t)parseGcodeBufHandle.codeValue() * 1000; // 电机解锁延时
    }
    else
    {
      bool codeSeenAxis[MAX_NUM_AXIS] = {false, false, false, false, false}; // 电机参数查询
      bool codeSeenAllAxis = false; // 是否查询到所有电机参数
      for(int32_t i = 0; i < motion_3d.axis_num; ++i)
      {
        codeSeenAxis[i] = parseGcodeBufHandle.codeSeen(axis_codes[i]);
        if(0 == i)
        {
          codeSeenAllAxis = !codeSeenAxis[i];
        }
        else
        {
          codeSeenAllAxis = (codeSeenAllAxis) && (!codeSeenAxis[i]);
        }
      }

      sg_grbl::st_synchronize(); // 等待运动队列清空
      if(codeSeenAllAxis)
      {
        for(int32_t i = 0; i < motion_3d.axis_num; ++i)
        {
          sg_grbl::st_enable_axis(i, false); // 解锁电机
        }
      }
      else
      {
        for(int32_t i = 0; i < motion_3d.axis_num; ++i)
        {
          if(codeSeenAxis[i])
          {
            sg_grbl::st_enable_axis(i, false); // 解锁电机
          }
        }
      }
    }
  }


}









