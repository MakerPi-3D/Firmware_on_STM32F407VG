#include  "view_common.h"
#include  "view_commonf.h"
#include "globalvariables.h"
#include "user_debug.h"
#include "sysconfig_data.h"

#ifdef __cplusplus
extern "C" {
#endif

//extern PCHAR call_sysconfig_get_model_info(void);
//extern PCHAR call_sysconfig_get_function_info(void);
//extern PCHAR call_sysconfig_get_version_info(void);
static void Hide_SettingInterface(UINT8 isclear)
{
  static UINT8 TouchCount1 = 0;
  static UINT8 TouchCount2 = 0;

  if (isclear)
  {
    if (t_sys.lcd_ssd1963_43_480_272)
    {
      //      static uint8_t test[4]= {12,2,13,3};
      for (INT i = 0; i < 2; ++i)
      {
      }
    }

    TouchCount1 = 0;
    TouchCount2 = 0;
  }

  /*******************************************************************************/
  if (TouchXY_NoBeep(0, 250, 80, 320)) //点击3次
  {
    ++TouchCount1;

    if (TouchCount1 >= 3)
    {
      TouchCount1 = 3;
    }
  }

  if (TouchXY_NoBeep(400, 250, 480, 320)) //点击3次
  {
    ++TouchCount2;

    if (TouchCount2 >= 3)
    {
      TouchCount2 = 3;
    }
  }

  if ((TouchCount1 >= 3) && (TouchCount2 >= 3)) //右上角和右下角各点击5次，进入机器配置界面
  {
    TouchCount1 = 0;
    TouchCount2 = 0;
    gui_set_curr_display(MachineSetting);
  }

  /*******************************************************************************/
}

void statusF(void)
{
  CHAR buffer[20];
  INT second;

  if (gui_is_refresh())
  {
    display_picture(14);
    DisplayTextDefault((UINT8 *)t_sys.model_str, 148, 103); //机型
    DisplayTextDefault((UINT8 *)t_sys.function_str, 148, 156); //功能配置
    //DisplayTextDefault ((UINT8*)"Base",148,156); //功能配置
    DisplayTextDefault((UINT8 *)t_sys.version_str, 148, 213); //版本
    //串口上传信息到上位机2017.7.6
    USER_EchoLogStr("M:%s\r\n", (UINT8 *)t_sys.model_str);
    USER_EchoLogStr("F:%s\r\n", (UINT8 *)t_sys.function_str);
    USER_EchoLogStr("V:%s\r\n", (UINT8 *)t_sys.version_str);
    SetTextDisplayRange(148, 268, 12 * 11, 24, &RunTimeTextRange);
    ReadTextDisplayRangeInfo(RunTimeTextRange, TimeTextRangeBuf);
    snprintf(buffer, sizeof(buffer), "%5ld:%2ld:%2ld", t_gui.machine_run_time / 3600, (t_gui.machine_run_time % 3600) / 60, (t_gui.machine_run_time % 3600) % 60);
    CopyTextDisplayRangeInfo(RunTimeTextRange, TimeTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((UINT8 *)buffer, RunTimeTextRange, TextRangeBuf);
    Hide_SettingInterface(1);
  }

  if (touchxy(0, 0, 160, 70))
  {
    gui_set_curr_display(maindisplayF);
    return ;
  }

  /*******************************************************************************/
  Hide_SettingInterface(0);//隐藏的设置界面

  /*******************************************************************************/
  if (second < t_gui.machine_run_time)
  {
    second = t_gui.machine_run_time + 60;
    //显示运行时间
    snprintf(buffer, sizeof(buffer), "%5ld:%2ld:%2ld", t_gui.machine_run_time / 3600, (t_gui.machine_run_time % 3600) / 60, (t_gui.machine_run_time % 3600) % 60);
    CopyTextDisplayRangeInfo(RunTimeTextRange, TimeTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((UINT8 *)buffer, RunTimeTextRange, TextRangeBuf);
    //串口上传信息到上位机2017.7.6
    //开发商要求不上传2017.9.6
    //    USER_EchoLogStr("T:%s\r\n",(UINT8*)buffer);
  }

  //  if(gui_is_rtc())
  //  {
  //    second=t_gui.machine_run_time;
  //    hour=second/3600;
  //    minute = (second-hour*3600)/60;
  //    second = (second-hour*3600)%60;
  //    //显示运行时间
  //    snprintf(buffer, sizeof(buffer), "%3d:%2d:%2d",hour,minute,second);
  //    CopyTextDisplayRangeInfo(RunTimeTextRange,TimeTextRangeBuf, TextRangeBuf);
  //    DisplayTextInRangeDefault((UINT8*)buffer, RunTimeTextRange,TextRangeBuf);
  //  }
}
#ifdef __cplusplus
}//extern "C" {
#endif

