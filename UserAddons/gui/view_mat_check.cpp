#include "user_interface.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "view_common.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include <stdbool.h>

/*20170803*/
extern FLOAT blockdetect_OldEposition;

void NotHaveMatControlInterface2(void) //断料操作界面2
{
  CHAR cmdResume_e_pos[40] = {0};

  if (gui_is_refresh())
  {
    display_picture(61);
  }

  if (touchxy(106, 148, 196, 218)) //继续打印
  {
    /*20170803修复堵料*/
    if (t_sys_data_current.enable_block_detect)
    {
      (void)snprintf(cmdResume_e_pos, sizeof(cmdResume_e_pos), "G92 E%0.4f isInternal", blockdetect_OldEposition);
      sys_send_gcode_cmd(cmdResume_e_pos);
    }

    t_gui_p.IsNotHaveMatInPrint = 0;
    gui_set_curr_display(maindisplayF);  //设置要显示的界面函数
    pauseprint = 0;              //暂停打印标志
    respond_gui_send_sem(ResumePrintValue);
    return ;
  }

  if (touchxy(270, 148, 361, 218)) //停止打印
  {
    t_gui_p.IsNotHaveMatInPrint = 0;
    print_flage = 0;
    pauseprint = 0;
    gui_set_curr_display(maindisplayF);
    respond_gui_send_sem(StopPrintValue);
    return ;
  }
}

extern INT get_pause_extruder_target_temp(void);
extern void temperature_set_extruder_target(CONST FLOAT celsius, INT extruder);
void HeatNozzleToChangeFilament(void)
{
  CHAR buffer[20];

  if (gui_is_refresh())
  {
    display_picture(64);
    SetTextDisplayRange(244, 49, 12 * 3, 24, &NozzleTempTextRange);
    ReadTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf);
    snprintf(buffer, sizeof(buffer), "%3d", (INT)t_gui.nozzle_temp);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((UINT8 *)buffer, NozzleTempTextRange, TextRangeBuf);
    snprintf(buffer, sizeof(buffer), "/%3d", (INT)get_pause_extruder_target_temp());
    DisplayTextDefault((UINT8 *)buffer, 244 + (12 * 3), 49);
    temperature_set_extruder_target(get_pause_extruder_target_temp(), 0);
  }

  if (gui_is_rtc())
  {
    snprintf(buffer, sizeof(buffer), "%3d", (INT)t_gui.nozzle_temp);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((UINT8 *)buffer, NozzleTempTextRange, TextRangeBuf);

    if (abs(get_pause_extruder_target_temp() - t_gui.nozzle_temp) <= 8) //最后5度需要等待较长时间，不等待
    {
      respond_gui_send_sem(ConfirmChangeFilamentValue);
      gui_set_curr_display(NotHaveMatToChangeFilament);
    }
  }
}

BOOL IsPauseToCoolDown(void);
void NotHaveMatControlInterface1(void) //断料操作界面1
{
  if (gui_is_refresh())
  {
    display_picture(60);
  }

  if (touchxy(106, 148, 196, 218)) //换料
  {
    if (IsPauseToCoolDown()) //暂停打印的时候降低了温度
    {
      respond_gui_send_sem(PauseToResumeNozzleTemp);
      gui_set_curr_display(HeatNozzleToChangeFilament);
    }
    else
    {
      respond_gui_send_sem(ConfirmChangeFilamentValue);
      gui_set_curr_display(NotHaveMatToChangeFilament);
    }

    return ;
  }

  if (touchxy(270, 148, 361, 218)) //停止打印
  {
    t_gui_p.IsNotHaveMatInPrint = 0;
    print_flage = 0;
    pauseprint = 0;
    gui_set_curr_display(maindisplayF);
    respond_gui_send_sem(StopPrintValue);
    return ;
  }
}

// 断料报警后，等待执行暂停打印操作，然后再进行进退丝操作
extern bool isProcessPausePrintDone(void);
void NoHaveMatWaringHome(void)
{
  if (gui_is_refresh())
  {
    display_picture(114);
  }

  if (isProcessPausePrintDone())
  {
    gui_set_curr_display(NotHaveMatControlInterface1);
  }
}

void NoHaveMatWaringInterface(void) //断料提示界面
{
  static ULONG BeepWaringTime = 0;

  if (gui_is_refresh())
  {
    display_picture(62);
    respond_gui_send_sem(OpenBeep);
    BeepWaringTime = sys_task_get_tick_count() + 5000; //鸣叫5秒
  }

  if (BeepWaringTime < sys_task_get_tick_count()) //时间到关闭鸣叫
  {
    respond_gui_send_sem(CloseBeep);
  }

  if (touchxy(155, 185, 335, 285)) //点击确定
  {
    gui_set_curr_display(NoHaveMatWaringHome);
    respond_gui_send_sem(CloseBeep);
  }
}

#ifdef __cplusplus
} // extern "C" {
#endif


