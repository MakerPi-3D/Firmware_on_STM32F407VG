#include "view_common.h"
#include "view_commonf.h"

namespace gui_view
{

#ifdef __cplusplus
  extern "C" {
#endif

  void NoUdiskF(void)
  {
    if (gui_is_refresh())
    {
      display_picture(43);
    }

    if (touchxy(160, 190, 330, 285))
    {
      gui_set_curr_display(maindisplayF);
    }
  }

#ifdef __cplusplus
} // extern "C" {
#endif

}



