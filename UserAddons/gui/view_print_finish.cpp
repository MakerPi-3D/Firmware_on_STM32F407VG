#include "view_common.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include "sysconfig_data.h"
#include "user_interface.h"
#include "functioncustom.h"
#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRINT_FINISH_ALARM_TIMEOUT  10000

static void pf_display_filename(void)
{
  int length;
  change_print_file_name();
  length = (INT)strlen(printnameb);
  DisplayTextDefault((UINT8 *)printnameb, 240 - ((length / 2) * 12), 95); //240-(length/2)*12    是为了让文字显示在中间
}

static void pf_display_time(void)
{
  char buffer[20];
  int hour, minute, second;
  second = t_gui.printed_time_sec;
  hour = second / 3600;
  minute = (second - (hour * 3600)) / 60;
  second = (second - (hour * 3600)) % 60;
  snprintf(buffer, sizeof(buffer), "%3d:%2d:%2d", hour, minute, second);
  DisplayTextDefault((UINT8 *)buffer, 195, 155);
}

static void pf_display_material_length(void)
{
  char buffer[20];

  if (t_gui.used_total_material >= 100000)
  {
    snprintf(buffer, sizeof(buffer), "%9d m", t_gui.used_total_material / 1000);
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "%9d mm", t_gui.used_total_material);
  }

  DisplayTextDefault((UINT8 *)buffer, 195, 200);
}

void printfinishF(void)
{
  static uint32_t mstime;

  if (gui_is_refresh())
  {
    display_picture(27);
    pf_display_filename();
    pf_display_time();
    pf_display_material_length();
    mstime = sys_task_get_tick_count();

    if (t_sys.alarm_sound && !gui_common::is_finish_print_beep_once)
    {
      respond_gui_send_sem(OpenBeep);
    }
  }

  if (touchxy(160, 230, 325, 320))
  {
    if (t_sys.alarm_sound)
    {
      respond_gui_send_sem(CloseBeep);
    }

    gui_set_curr_display(maindisplayF);
    return;
  }

  if (gui_is_rtc())
  {
    if ((sys_task_get_tick_count() - mstime) > PRINT_FINISH_ALARM_TIMEOUT)
    {
      if (t_sys.alarm_sound)
      {
        respond_gui_send_sem(CloseBeep);
        gui_common::is_finish_print_beep_once = true;
      }
    }
  }
}
#ifdef __cplusplus
}//extern "C" {
#endif
