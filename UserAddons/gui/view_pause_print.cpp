#include "view_common.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsPrintSDFile(void);
void PausePrintF(void)
{
  UINT length;

  if (gui_is_refresh())
  {
    display_picture(76);
    change_print_file_name();
    length = strlen(printnameb);
    DisplayTextDefault((UINT8 *)printnameb, (INT)((t_sys.lcd_ssd1963_43_480_272 ? 212 : 240) - ((length / 2) * 12)), 105); //240-(length/2)*12   是为了让文字显示在中间
  }

  if (touchxy(60, 200, 240, 285))
  {
    gui_set_curr_display(maindisplayF);  //设置要显示的界面函数
    pauseprint = 1;              //暂停打印标志
    respond_gui_send_sem(PausePrintValue);
    return ;
  }

  if (touchxy(240, 200, 425, 285))
  {
    gui_set_curr_display(maindisplayF);
    return ;
  }

  if (t_gui_p.IsNotHaveMatInPrint) //打印中检测到没有材料
  {
    gui_set_curr_display(NoHaveMatWaringInterface);
    return ;
  }

  if (t_gui_p.ChangeFilamentHeatStatus == 0) //加热没完成
  {
    if ((0 == t_gui_p.SDIsInsert) && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      respond_gui_send_sem(StopPrintValue);
      print_flage = 0;
      pauseprint = 0;
      gui_set_curr_display(maindisplayF);
      return ;
    }
  }

  if ((t_gui_p.IsDoorOpen) && (pauseprint == 0) && print_flage) //打印且没有暂停
  {
    gui_set_curr_display(DoorOpenWarningInfo_Printing);
    return ;
  }
}


void ResumePrintF(void)
{
  UINT length;

  if (gui_is_refresh())
  {
    display_picture(77);
    change_print_file_name();
    length = strlen(printnameb);
    DisplayTextDefault((UINT8 *)printnameb, (INT)(240 - ((length / 2) * 12)), 105); //240-(length/2)*12    是为了让文字显示在中间
  }

  if (touchxy(60, 200, 240, 285))
  {
    gui_set_curr_display(maindisplayF);  //设置要显示的界面函数
    pauseprint = 0;              //暂停打印标志
    respond_gui_send_sem(ResumePrintValue);
    return ;
  }

  if (touchxy(240, 200, 425, 285))
  {
    gui_set_curr_display(maindisplayF);
    return ;
  }

  if (t_gui_p.IsNotHaveMatInPrint) //打印中检测到没有材料
  {
    gui_set_curr_display(NoHaveMatWaringInterface);
    return ;
  }

  if ((0 == t_gui_p.SDIsInsert) && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
  {
    respond_gui_send_sem(StopPrintValue);
    print_flage = 0;
    pauseprint = 0;
    gui_set_curr_display(maindisplayF);
    return ;
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

