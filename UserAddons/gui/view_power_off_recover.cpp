#include "view_common.h"
#include "machinecustom.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
//#include "ConfigurationStore.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "user_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PowerOffRecoverWarningTime 5000
static ULONG PowerOffRecoverWarningTimeout;

extern void MainInterfaceDisplayInit(void);
extern void MainInterfaceDisplayText(void);
extern CHAR poweroffrunonce;

void PowerOffRecoverReady(void)
{
  if (gui_is_refresh())
  {
    if (t_custom_services.disable_hot_bed)
    {
      display_picture(53);
    }
    else
    {
      display_picture(49);
    }

    MainInterfaceDisplayInit();
    MainInterfaceDisplayText();

    if (poweroffrunonce)
    {
      respond_gui_send_sem(CloseBeep);
      respond_gui_send_sem(ConfirmPowerOffRecover);
      poweroffrunonce = 0;
    }

    pauseprint = 0;
    print_flage = 1;//2017511在按下确定断电续打时标志，防止加热失败仍继续打印
  }

  if (touchxy(0, 0, 240, 65))
  {
    gui_set_curr_display(SetPowerOffRecoverNozzleTemp);
    return;
  }

  if (!t_custom_services.disable_hot_bed) //NotM14_Right
  {
    if (touchxy(240, 0, 480, 65))
    {
      gui_set_curr_display(SetPowerOffRecoverHotBedTemp);
      return;
    }
  }

  if (t_gui_p.IsFinishedPowerOffRecoverReady)
  {
    t_gui_p.IsFinishedPowerOffRecoverReady = 0;
    //    pauseprint=0;
    //    print_flage = 1;//2017511在按下确定断电续打时标志，防止加热失败仍继续打印
    gui_set_curr_display(maindisplayF);
  }

  if (gui_is_rtc())
  {
    MainInterfaceDisplayText();
  }
}

// 设置断电续打超时时间为60s，超过60s直接恢复打印
static ULONG power_off_recover_tick_count = 0;
static CONST ULONG POWER_OFF_RECOVER_TIME_OUT = 1000 * 60;

void PowerOffRecover(void)
{
  UINT length;

  if (0 == power_off_recover_tick_count)
  {
    power_off_recover_tick_count = sys_task_get_tick_count();
  }

  if (gui_is_refresh())
  {
    display_picture(45);
    change_print_file_name();
    length = strlen(printnameb);
    DisplayTextDefault((UINT8 *)printnameb, (INT)(240 - ((length / 2) * 12)), 105); //240-(length/2)*12    是为了让文字显示在中间

    if (M3145T != t_sys_data_current.model_id)
    {
      respond_gui_send_sem(OpenBeep);
      PowerOffRecoverWarningTimeout = sys_task_get_tick_count() + PowerOffRecoverWarningTime;
    }
  }

  if (((M3145T != t_sys_data_current.model_id) && ((power_off_recover_tick_count + POWER_OFF_RECOVER_TIME_OUT) <= sys_task_get_tick_count())) || touchxy(60, 200, 240, 285)) //确认键
  {
    poweroffrunonce = 1;
    gui_set_curr_display(PowerOffRecoverReady);
    return ;
  }

  if (touchxy(240, 200, 425, 285)) //取消键
  {
    respond_gui_send_sem(CloseBeep);
    respond_gui_send_sem(CancelPowerOffRecover);
    gui_set_curr_display(maindisplayF);
    return ;
  }

  if (gui_is_rtc())
  {
    if (PowerOffRecoverWarningTimeout < sys_task_get_tick_count())
    {
      respond_gui_send_sem(CloseBeep);
    }
  }
}
#ifdef __cplusplus
} // extern "C" {
#endif

