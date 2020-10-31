#include "view_common.h"
#include "view_commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "config_motion_3d.h"

namespace gui_view
{

#ifdef __cplusplus
  extern "C" {
#endif

  static void FinishedCalculateZMaxLimit(void)
  {
    if (gui_is_refresh())
    {
      display_picture(48);
      char TextBuffer[20];
      //显示Z轴最大位置
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (int)motion_3d_model.xyz_move_max_pos[2]);
      DisplayTextDefault((uint8_t *)TextBuffer, 245, 141);
    }

    if (touchxy(165, 210, 320, 290))
    {
      gui_set_curr_display(settingF);
    }
  }


  void CalculatingZMaxLimit(void)
  {
    if (gui_is_refresh())
    {
      display_picture(47);
    }

    if (t_gui_p.IsFinishedCalculateZPosLimit)
    {
      t_gui_p.IsFinishedCalculateZPosLimit = 0;
      gui_set_curr_display(FinishedCalculateZMaxLimit);
    }
  }

#ifdef __cplusplus
} // extern "C" {
#endif

}


