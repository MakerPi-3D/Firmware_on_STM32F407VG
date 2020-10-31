#include "gcodebufferhandle.h"
#include "globalvariables.h"
#include "view_common.h"
#include "interface.h"
#include "PrintControl.h"
#include "Alter.h"

namespace gcode
{

  void m305_process(void)
  {
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      t_gui_p.m305_is_force_verify = parseGcodeBufHandle.codeValueLong();
      if(serial_print[0])
      {
        switch(static_cast<INT>(parseGcodeBufHandle.codeValue()))
        {
        case 0:
          pauseprint=0;
          print_flage = 0;
          gui_set_curr_display(maindisplayF);
          break;

        case 1:
          respond_gui_send_sem(FilePrintValue);
          SetPrintStatus(true);
          pauseprint=0;
          print_flage = 1;
          gui_set_curr_display(maindisplayF);
          break;

        default:
          break;
        }

      }
    }
  }

  void m2501_process(void)
  {
    gui_set_curr_display(maindisplayF);
    pauseprint=1;
    respond_gui_send_sem(PausePrintValue);
  }

  void m2502_process(void)
  {
    gui_set_curr_display(maindisplayF);
    pauseprint=0;
    respond_gui_send_sem(ResumePrintValue);
  }

  void m2503_process(void)
  {
    gui_set_curr_display(maindisplayF);
    print_flage = 0;
    respond_gui_send_sem(StopPrintValue);
  }
}








