#include "user_interface.h"
#include "Alter.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "view_common.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_motion_3d.h"
#define NoClick 0
#define XPosDown 1
#define XPosUp 2
#define YPosDown 3
#define YPosUp 4
#define ZPosDown 5
#define ZPosUp 6

#define ClickTimeoutms 200
#define ClickFirstTimeoutms 600

static UINT8 ClickKeyValue = NoClick;
static ULONG ClickTimeout = 0;
static ULONG ClickFirstTimeout = 0;
static UINT8 SlowUpdateXYZPos = 0;
static UINT8 FastUpdateXYZPos = 0;

void DisplayXYZPosText(void)
{
  char TextXYZPosbuffer[20];
  memset(TextXYZPosbuffer, 0, sizeof(char) * 20);
  snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3d", t_gui.move_xyz_pos[X_AXIS]);
  CopyTextDisplayRangeInfo(XPosTextShape, XPosRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextXYZPosbuffer, XPosTextShape, TextRangeBuf);
  snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3d", t_gui.move_xyz_pos[Y_AXIS]);
  CopyTextDisplayRangeInfo(YPosTextShape, YPosRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextXYZPosbuffer, YPosTextShape, TextRangeBuf);
  snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3d", t_gui.move_xyz_pos[Z_AXIS]);
  CopyTextDisplayRangeInfo(ZPosTextShape, ZPosRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextXYZPosbuffer, ZPosTextShape, TextRangeBuf);
}


void RefreshMoveXYZInterface(void)
{
  display_picture(26);
  SetTextDisplayRange(TextDisplayX, 35, 12 * 3, 24, &XPosTextShape);
  ReadTextDisplayRangeInfo(XPosTextShape, XPosRangeBuf);
  SetTextDisplayRange(TextDisplayX, 126, 12 * 3, 24, &YPosTextShape);
  ReadTextDisplayRangeInfo(YPosTextShape, YPosRangeBuf);
  SetTextDisplayRange(TextDisplayX, 211, 12 * 3, 24, &ZPosTextShape);
  ReadTextDisplayRangeInfo(ZPosTextShape, ZPosRangeBuf);
  DisplayXYZPosText();
}

void MoveXYZConfirmKey(void)
{
  respond_gui_send_sem(MoveXYZValue);
  gui_set_curr_display(prepareF);
  goto_page_homing();
}

UINT8 ScanClickKeyValue(void)
{
  if (IstouchxyDown(364, 0, 480, 82))
  {
    return XPosDown;
  }
  else if (IstouchxyDown(250, 0, 364, 82))
  {
    return XPosUp;
  }
  else if (IstouchxyDown(364, 82, 480, 179))
  {
    return YPosDown;
  }
  else if (IstouchxyDown(250, 82, 364, 179))
  {
    return YPosUp;
  }
  else if (IstouchxyDown(364, 179, 480, 260))
  {
    return ZPosDown;
  }
  else if (IstouchxyDown(250, 179, 364, 260))
  {
    return ZPosUp;
  }
  else
  {
    return NoClick;
  }
}

void LongClickPress(void)
{
  if (IstouchxyUp())
  {
    ClickKeyValue = 0;
    (void)sys_os_delay(200);
  }
  else if (ClickTimeout < sys_task_get_tick_count())
  {
    ClickTimeout = ClickTimeoutms + sys_task_get_tick_count();
    SlowUpdateXYZPos = 0;
    FastUpdateXYZPos = 1;
  }
}

void ShortClickPress(void)
{
  SlowUpdateXYZPos = 1;
  FastUpdateXYZPos = 0;
}

void UpdateXYZPos(void)
{
  switch (ClickKeyValue)
  {
  case XPosDown:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] - 10;
    }
    else
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] - 1;
      ClickKeyValue = NoClick;
    }

    break;

  case XPosUp:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] + 10;
    }
    else
    {
      t_gui.move_xyz_pos[X_AXIS] = t_gui.move_xyz_pos[X_AXIS] + 1;
      ClickKeyValue = NoClick;
    }

    break;

  case YPosDown:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] - 10;
    }
    else
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] - 1;
      ClickKeyValue = NoClick;
    }

    break;

  case YPosUp:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] + 10;
    }
    else
    {
      t_gui.move_xyz_pos[Y_AXIS] = t_gui.move_xyz_pos[Y_AXIS] + 1;
      ClickKeyValue = NoClick;
    }

    break;

  case ZPosDown:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] - 10;
    }
    else
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] - 1;
      ClickKeyValue = NoClick;
    }

    break;

  case ZPosUp:
    if (1 == FastUpdateXYZPos)
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] + 10;
    }
    else
    {
      t_gui.move_xyz_pos[Z_AXIS] = t_gui.move_xyz_pos[Z_AXIS] + 1;
      ClickKeyValue = NoClick;
    }

    break;

  default:
    break;
  }
}

void XYZPosRangeLimit(void)
{
  if (t_gui.move_xyz_pos[X_AXIS] > motion_3d_model.xyz_move_max_pos[0])
  {
    t_gui.move_xyz_pos[X_AXIS] = motion_3d_model.xyz_move_max_pos[0];
  }

  if (t_gui.move_xyz_pos[X_AXIS] < 0)
  {
    t_gui.move_xyz_pos[X_AXIS] = 0;
  }

  if (t_gui.move_xyz_pos[Y_AXIS] > motion_3d_model.xyz_move_max_pos[1])
  {
    t_gui.move_xyz_pos[Y_AXIS] = motion_3d_model.xyz_move_max_pos[1];
  }

  if (t_gui.move_xyz_pos[Y_AXIS] < 0)
  {
    t_gui.move_xyz_pos[Y_AXIS] = 0;
  }

  if (t_gui.move_xyz_pos[Z_AXIS] > motion_3d_model.xyz_move_max_pos[2])
  {
    t_gui.move_xyz_pos[Z_AXIS] = motion_3d_model.xyz_move_max_pos[2];
  }

  if (t_gui.move_xyz_pos[Z_AXIS] < 0)
  {
    t_gui.move_xyz_pos[Z_AXIS] = 0;
  }

#if LASER_MODE

  if (t_sys_data_current.IsLaser)
  {
    if (t_gui.move_xyz_pos[Z_AXIS] < 100)
      t_gui.move_xyz_pos[Z_AXIS] = 100;
  }

#endif
}

void SetXYZPosValue(void)
{
  if (NoClick == ClickKeyValue)
  {
    ClickTimeout = ClickTimeoutms + sys_task_get_tick_count();
    ClickFirstTimeout = ClickFirstTimeoutms + sys_task_get_tick_count();
    ClickKeyValue = ScanClickKeyValue();
  }
  else
  {
    if (ClickFirstTimeout < sys_task_get_tick_count())
    {
      LongClickPress();
    }
    else if (IstouchxyUp())
    {
      ShortClickPress();
    }
  }

  if ((1 == FastUpdateXYZPos) || (1 == SlowUpdateXYZPos))
  {
    UpdateXYZPos();
    FastUpdateXYZPos = 0;
    SlowUpdateXYZPos = 0;
    XYZPosRangeLimit();
    DisplayXYZPosText();
  }
}

void MoveXYZCancelKey(void)
{
  gui_set_curr_display(prepareF);
}


void MoveXYZ(void)
{
  if (gui_is_refresh())
  {
    RefreshMoveXYZInterface();
    return;
  }

  if (touchxy(110, 255, 240, 320))
  {
    MoveXYZConfirmKey();
    return ;
  }

  if (touchxy(240, 265, 380, 320))
  {
    MoveXYZCancelKey();
    return ;
  }

  SetXYZPosValue();
}

#ifdef __cplusplus
} // extern "C" {
#endif




