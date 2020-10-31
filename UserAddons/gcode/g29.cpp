#include "gcodebufferhandle.h"
#include "globalvariables_ccmram.h"
#include "interface.h"
#include "user_debug.h"
#include "autobedlevelinterface.h"
#include "gcode_global_params.h"

namespace gcode
{
  extern "C" void infrared_adjust_check(void);
  extern "C" void infrared_adjust_finish_serial(void);
  extern "C" void infrared_next_adjust_check(void);
  /**
    * @brief  auto-bed leveling of Marling，自动校准平台，按照Marling的方式
    * @note   目前使用红外模块，只能半自动调平
    * @param  None
    * @retval None
    * @date 2017/10/20
    */
  void g29_process(void)
  {
    if(2U == t_sys_data_current.enable_bed_level)
    {
      UINT SParam = 0U;
      if(parseGcodeBufHandle.codeSeen('S'))  //下一条Gcode的文件位置
      {
        SParam=(UINT)parseGcodeBufHandle.codeValueLong();
        if(SParam==1U)
        {
          infrared_adjust_check();
        }
        else if(SParam==2U)
        {
          infrared_adjust_finish_serial();
        }
        else if(SParam==3U)
        {
          infrared_next_adjust_check();
        }
        else
        {
          // return
        }
      }
      else
      {
        respond_gui_send_sem(StartCalBedLevel);
        USER_EchoLogStr("G29 Done");
      }
    }
    else
    {
      gcodeCMD_g29_interface(is_serial_cmd);
    }
  }


}








