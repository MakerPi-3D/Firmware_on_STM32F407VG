#include "view_common.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"

#ifdef __cplusplus
extern "C" {
#endif

static UINT8 IsNotHaveMatToChangeFilament = 0;

void ChangeFilamentStatus(void)
{
  static UINT8 RefreshOnce = 1;
  static UINT8 LastStatus;

  if (t_gui_p.M600FilamentChangeStatus < 3)
  {
    if ((1 == RefreshOnce) || (gui_is_refresh()))
    {
      display_picture(21);
      RefreshOnce = 3;
      LastStatus = 0;
    }
  }
  else if (t_gui_p.M600FilamentChangeStatus == 3)
  {
    if ((3 == RefreshOnce) || (gui_is_refresh()))
    {
      display_picture(22);
      RefreshOnce = 4;
    }

    if (touchxy(150, 200, 330, 300))
    {
      //      gui_send_sem_confirm_load_filament();
      respond_gui_send_sem(ConfirmLoadFilamentValue);
    }
  }
  else if (t_gui_p.M600FilamentChangeStatus == 4)
  {
    if ((4 == RefreshOnce) || (gui_is_refresh()))
    {
      display_picture(20);
      RefreshOnce = 5;
    }
  }
  else if (t_gui_p.M600FilamentChangeStatus == 5)
  {
    if (IsNotHaveMatToChangeFilament)
    {
      gui_set_curr_display(NotHaveMatControlInterface2);
    }
    else
    {
      gui_set_curr_display(maindisplayF);
    }

    RefreshOnce = 1;
    //    gui_send_sem_close_beep();
    respond_gui_send_sem(CloseBeep);
  }

  if ((t_gui_p.IsDoorOpen) && (LastStatus == 0))
  {
    //    gui_send_sem_open_beep();
    respond_gui_send_sem(OpenBeep);
    LastStatus = 1;
  }
  else if ((!t_gui_p.IsDoorOpen) && (LastStatus == 1))
  {
    //    gui_send_sem_close_beep();
    respond_gui_send_sem(CloseBeep);
    LastStatus = 0;
  }
}
BOOL IsPrintSDFile(void);
void ChangeFilamentConfirm(void)
{
  UINT length;

  if (gui_is_refresh())
  {
    display_picture(19);
    change_print_file_name();
    length = strlen(printnameb);
    DisplayTextDefault((UINT8 *)printnameb, (INT)((t_sys.lcd_ssd1963_43_480_272 ? 212 : 240) - ((length / 2) * 12)), 105); //240-(length/2)*12   是为了让文字显示在中间
  }

  if (touchxy(60, 200, 240, 285))
  {
    //    gui_send_sem_confirm_chg_filament();
    respond_gui_send_sem(ConfirmChangeFilamentValue);
    gui_set_curr_display(ChangeFilamentStatus);
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

  if (pauseprint && print_flage) //打印且暂停了，拔出U盘则停止打印且返回主界面
  {
    if ((0 == t_gui_p.SDIsInsert) && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd))
    {
      //      gui_send_sem_stop_print();
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

void NotHaveMatToChangeFilament(void)
{
  IsNotHaveMatToChangeFilament = 1;
  ChangeFilamentStatus();
  IsNotHaveMatToChangeFilament = 0;
}


#ifdef __cplusplus
} //extern "C" {
#endif



