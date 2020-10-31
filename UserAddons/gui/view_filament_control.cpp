#include "view_commonf.h"
#include "view_common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "Alter.h"
#include "mechanical_bed_level_adjust.h"

#ifdef __cplusplus
extern "C" {
#endif

static char buffer[20];

static void display_filament_temp_init(void)
{
  SetTextDisplayRange((t_sys.lcd_ssd1963_43_480_272 ? 15 : 0) + 244, 49 - (t_sys.lcd_ssd1963_43_480_272 ? 2 : 0), 12 * 3, 24, &NozzleTempTextRange);
  SetTextDisplayRange(((t_sys.lcd_ssd1963_43_480_272 ? 15 : 0) + 244) + (12 * 6), 49 - (t_sys.lcd_ssd1963_43_480_272 ? 2 : 0), 12 * 3, 24, &NozzleTargetTempTextRange);
  ReadTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf);
}

static void display_filament_temp_update(void)
{
  //显示喷嘴温度
  snprintf(buffer, sizeof(buffer), "%3d", (INT)t_gui.nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((UINT8 *)buffer, NozzleTempTextRange, TextRangeBuf);
  //显示喷嘴目标温度
  snprintf(buffer, sizeof(buffer), " / ");
  DisplayTextDefault((UINT8 *)buffer, ((t_sys.lcd_ssd1963_43_480_272 ? 15 : 0) + 244) + (12 * 3), 49 - (t_sys.lcd_ssd1963_43_480_272 ? 2 : 0));
  //显示目标喷嘴温度
  snprintf(buffer, sizeof(buffer), "%3d", (INT)t_gui.target_nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((UINT8 *)buffer, NozzleTargetTempTextRange, TextRangeBuf);
}

void loadfilament0F(void)
{
  if (gui_is_refresh())
  {
    display_picture(8);
    display_filament_temp_init();
    display_filament_temp_update();
  }

  if ((t_sys_data_current.IsMechanismLevel ? t_gui_p.G28_ENDSTOPS_COMPLETE : 1) && touchxy(160, 210, 310, 300))
  {
    gui_set_curr_display(prepareF);
    respond_gui_send_sem(StopFeedFilamentValue);
    return ;
  }

  if (touchxy(165, 35, 310, 85))
  {
    gui_set_curr_display(SetLoadFilamentNozzleTemp);
    return ;
  }

  if (gui_is_rtc())
  {
    if (t_gui_p.IsFinishedFilamentHeat)
    {
      gui_set_curr_display(loadfilament1F);
      return;
    }

    display_filament_temp_update();
    return;
  }
}

void loadfilament1F(void)
{
  if (gui_is_refresh())
  {
    display_picture(9);
  }

  if ((t_sys_data_current.IsMechanismLevel ? t_gui_p.G28_ENDSTOPS_COMPLETE : 1) && touchxy(160, 210, 310, 300))
  {
    gui_set_curr_display(prepareF);
    respond_gui_send_sem(StopFeedFilamentValue);
    return ;
  }

  if (gui_is_rtc())
  {
    if (t_gui_p.IsSuccessFilament)
    {
      gui_set_curr_display(loadfilament2F);
      return;
    }

    return;
  }
}

void loadfilament2F(void)
{
  if (gui_is_refresh())
  {
    display_picture(10);
  }

  if (touchxy(160, 200, 320, 300))
  {
    gui_set_curr_display(prepareF);
    return ;
  }
}

void unloadfilament0F(void)
{
  if (gui_is_refresh())
  {
    display_picture(8);
    display_filament_temp_init();
    display_filament_temp_update();
  }

  if ((t_sys_data_current.IsMechanismLevel ? t_gui_p.G28_ENDSTOPS_COMPLETE : 1) && touchxy(160, 210, 310, 300))
  {
#if LASER_MODE

    if (Gui_Mode == MODE_FILUM  && t_sys_data_current.IsLaserMode)
      gui_set_curr_display(Laser_change_warn);
    else
#endif
      if (Gui_Mode == MODE_FILUM_LEVEL && t_sys_data_current.IsMechanismLevel)
        gui_set_curr_display(Level_Before_Warn);
      else
        gui_set_curr_display(prepareF);

    respond_gui_send_sem(StopBackFilamentValue);
    return ;
  }

  if (touchxy(165, 35, 310, 85))
  {
    gui_set_curr_display(SetUnLoadFilamentNozzleTemp);
    return ;
  }

  if (gui_is_rtc())
  {
    if (t_gui_p.IsFinishedFilamentHeat)
    {
      gui_set_curr_display(unloadfilament1F);
      return;
    }

    display_filament_temp_update();
    return;
  }
}

void unloadfilament1F(void)
{
  if (gui_is_refresh())
  {
    display_picture(11);
  }

  if ((t_sys_data_current.IsMechanismLevel ? t_gui_p.G28_ENDSTOPS_COMPLETE : 1) && touchxy(160, 210, 310, 300))
  {
#if LASER_MODE

    if (Gui_Mode == MODE_FILUM  && t_sys_data_current.IsLaserMode)
      gui_set_curr_display(Laser_change_warn);
    else
#endif
      if (Gui_Mode == MODE_FILUM_LEVEL && t_sys_data_current.IsMechanismLevel)
        gui_set_curr_display(Level_Before_Warn);
      else
        gui_set_curr_display(prepareF);

    respond_gui_send_sem(StopBackFilamentValue);
    return ;
  }

  if (gui_is_rtc())
  {
    if (t_gui_p.IsSuccessFilament)
    {
      gui_set_curr_display(unloadfilament2F);
      return;
    }

    return;
  }
}

void unloadfilament2F(void)
{
  if (gui_is_refresh())
  {
    display_picture(12);
  }

  if (touchxy(160, 200, 320, 300))
  {
#if LASER_MODE

    if (Gui_Mode == MODE_FILUM  && t_sys_data_current.IsLaserMode)
      gui_set_curr_display(Laser_change_warn);
    else
#endif
      if (Gui_Mode == MODE_FILUM_LEVEL && t_sys_data_current.IsMechanismLevel)
        gui_set_curr_display(Level_Before_Warn);
      else
        gui_set_curr_display(prepareF);

    return ;
  }
}


#ifdef __cplusplus
} // extern "C" {
#endif






