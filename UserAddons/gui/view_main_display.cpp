
#include "machinecustom.h"
#include "view_commonf.h"              //包含界面函数
#include "view_common.h"
#include "globalvariables.h"
#include "interface.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "Alter.h"
#include "sys_function.h"
#include "user_fatfs.h"
#ifdef __cplusplus
extern "C" {
#endif

extern bool IsFinishedPrint(void);
extern bool IsPrintSDFile(void);

//跳转归零页面
void goto_page_homing(void)
{
  if (PICTURE_IS_JAPANESE != t_sys_data_current.pic_id) //日文没归零页面，不做下面操作
  {
    if (0U == t_gui_p.G28_ENDSTOPS_COMPLETE)
    {
      predisplay = currentdisplay;
      gui_set_curr_display(page_homing);
    }
  }
}

//归零页面
void page_homing(void)
{
  if (gui_is_refresh())
  {
    display_picture(83);
  }

  if (1U == t_gui_p.G28_ENDSTOPS_COMPLETE)
  {
    gui_set_curr_display(predisplay);
  }
}

INT CheckIsPrintFinish(void)
{
  //以下根据实时信号跳转界面
  if (IsFinishedPrint()) //打印完成标志
  {
    gui_common::is_finish_print_beep_once = false;
    print_flage = 0;
    gui_set_curr_display(printfinishF);
    return 1;
  }

  return 0;
}

INT CheckIsHaveUdisk(void)
{
  //拔U盘返回主界面
  if ((0 == t_gui_p.SDIsInsert) && (!IsPrintSDFile()) && (!t_power_off.is_file_from_sd) && !serial_print[0])
  {
    //U盘拔出标志，文件上传完成后打印的标志
    respond_gui_send_sem(StopPrintValue);
    pauseprint = 0;
    print_flage = 0;
    gui_set_curr_display(maindisplayF);  //切换界面
    return 1;
  }

  return 0;
}

void Custom_PrintInterfaceDisplayInit(void)
{
  CHAR print_percent[5];
  SetTextDisplayRange(132, 36, 12 * 3, 24, &NozzleTempTextRange); //设置显示区域
  SetTextDisplayRange(324, 36, 12 * 3, 24, &HotBedTempTextRange);
  SetTextDisplayRange(230, 147, 12 * 3, 24, &PrintScheduleTextRange);
  SetTextDisplayRange(324, 84, 12 * 9, 24, &PrintTimeTextRange);
  SetTextDisplayRange(132, 84, 12 * 3, 24, &CavityTempTextRange); //设置显示区域
  SetTextDisplayRange((132 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &NozzleTargetTempTextRange);
  SetTextDisplayRange((324 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &HotBedTargetTempTextRange);
  SetTextDisplayRange((132 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 84, 12 * 3, 24, &CavityTargetTempTextRange);
  //更新打印进度
  snprintf(print_percent, sizeof(print_percent), "%2d%%", (INT)t_gui.print_percent);

  if (Draw_progressBar_new(t_gui.printfile_size, t_gui.file_size - (UINT32)((t_gui.printfile_size + 99) * 0.01), 92, 148, 344, 20))
  {
    ReadTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape);
    CopyTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)print_percent, PrintScheduleTextRange, TextRangeBuf);
  }

  ReadTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf); //从lcd读取像素，保存到数组
  ReadTextDisplayRangeInfo(HotBedTempTextRange, HotBedTempTextRangeBuf);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf);
  ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf);
  //  ReadTextDisplayRangeInfo(PrintScheduleTextRange,PrintScheduleShape);
  ReadTextDisplayRangeInfo(PrintTimeTextRange, TimeTextRangeBuf);
  ReadTextDisplayRangeInfo(CavityTempTextRange, CavityTempTextRangeBuf); //从lcd读取像素，保存到数组
  ReadTextDisplayRangeInfo(CavityTargetTempTextRange, CavityTargetTempTextRangeBuf);
}

void PrintInterfaceDisplayInit(void)
{
  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    Custom_PrintInterfaceDisplayInit();
  }
  else
  {
    CHAR printnameb[MK_MAX_LFN];

    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      SetTextDisplayRange(100, 22, 12 * 3, 24, &NozzleTempTextRange); //设置显示区域
      SetTextDisplayRange(100, 62, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRange((100 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 22, 12 * 3, 24, &NozzleTargetTempTextRange);
      SetTextDisplayRange((100 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 62, 12 * 3, 24, &HotBedTargetTempTextRange);
      SetTextDisplayRange(250, 286, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRange(100, 102, 12 * 9, 24, &PrintTimeTextRange);
      SetTextDisplayRange(100, 142, 12 * 9, 24, &SpeedRange);
      ReadTextDisplayRangeInfo(SpeedRange, SpeedRangeBuf);
      strcpy(printnameb, SettingInfoToSYS.PrintFileName);

      if (strlen(printnameb) > (MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4)) //现在显示方式，一个字节占12列
      {
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4] = 0;
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4 - 1] = '.';
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4 - 2] = '.';
        printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 4 - 3] = '.';
      }

      DisplayTextDefault((UINT8 *)printnameb, (INT)(375 - ((strlen(printnameb) / 2) * 12)), 260); //240-(length/2)*12是为了让文字显示在中间
      //更新打印进度
      snprintf(printnameb, sizeof(printnameb), "%2d%%", (INT)t_gui.print_percent);

      if (Draw_progressBar(t_gui.printfile_size, t_gui.file_size - (UINT32)((t_gui.printfile_size + 99) * 0.01)))
      {
        ReadTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape);
        CopyTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape, TextRangeBuf);
        DisplayTextInRangeDefault((PUCHAR)printnameb, PrintScheduleTextRange, TextRangeBuf);
      }
    }
    else
    {
      SetTextDisplayRange(132, 36, 12 * 3, 24, &NozzleTempTextRange); //设置显示区域
      SetTextDisplayRange(335, 36, 12 * 3, 24, &HotBedTempTextRange);
      SetTextDisplayRange((132 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &NozzleTargetTempTextRange);
      SetTextDisplayRange((335 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 36, 12 * 3, 24, &HotBedTargetTempTextRange);
      SetTextDisplayRange(132, 114, 12 * 3, 24, &PrintScheduleTextRange);
      SetTextDisplayRange(335, 114, 12 * 9, 24, &PrintTimeTextRange);
      ReadTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape);
    }

    ReadTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf); //从lcd读取像素，保存到数组
    ReadTextDisplayRangeInfo(HotBedTempTextRange, HotBedTempTextRangeBuf);
    ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf);
    ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf);
    //  ReadTextDisplayRangeInfo(PrintScheduleTextRange,PrintScheduleShape);
    ReadTextDisplayRangeInfo(PrintTimeTextRange, TimeTextRangeBuf);
  }
}

void Custom_PrintInterfaceDisplayText(void)
{
  CHAR TextBuffer[20];
  INT second, minute, hour;
  //显示喷嘴温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTempTextRange, TextRangeBuf);
  //显示斜杠
  DisplayTextDefault((PUCHAR)"/", (132 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36); //直接显示到lcd
  //显示喷嘴目标温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf);

  if (!t_custom_services.disable_hot_bed)
  {
    //显示热床温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTempTextRange, HotBedTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTempTextRange, TextRangeBuf);
    //显示斜杠
    DisplayTextDefault((PUCHAR)"/", (324 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36);
    //显示热床目标温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf);
  }

  //显示打印进度
  snprintf(TextBuffer, sizeof(TextBuffer), "%2d%%", (INT)t_gui.print_percent);

  if (Draw_progressBar_new(t_gui.printfile_size, t_gui.file_size, 92, 148, 344, 20))
  {
    CopyTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintScheduleTextRange, TextRangeBuf);
  }

  //显示喷嘴温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.cavity_temp);
  CopyTextDisplayRangeInfo(CavityTempTextRange, CavityTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, CavityTempTextRange, TextRangeBuf);
  //显示斜杠
  DisplayTextDefault((PUCHAR)"/", (132 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 84); //直接显示到lcd
  //显示喷嘴目标温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_cavity_temp);
  CopyTextDisplayRangeInfo(CavityTargetTempTextRange, CavityTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, CavityTargetTempTextRange, TextRangeBuf);
  //显示打印时间
  second = t_gui.printed_time_sec;
  hour = second / 3600;
  minute = (second - (hour * 3600)) / 60;
  second = (second - (hour * 3600)) % 60;
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d:%2d:%2d", hour, minute, second);
  CopyTextDisplayRangeInfo(PrintTimeTextRange, TimeTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintTimeTextRange, TextRangeBuf);
}


void PrintInterfaceDisplayText(void)
{
  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    Custom_PrintInterfaceDisplayText();
  }
  else
  {
    CHAR TextBuffer[20];
    INT second, minute, hour;
    //显示喷嘴温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.nozzle_temp);
    CopyTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTempTextRange, TextRangeBuf);

    //显示斜杠
    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      DisplayTextDefault((PUCHAR)"/", (100 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 22); //直接显示到lcd
    }
    else
    {
      DisplayTextDefault((PUCHAR)"/", (132 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36); //直接显示到lcd
    }

    //显示喷嘴目标温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_nozzle_temp);
    CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf);

    if (!t_custom_services.disable_hot_bed)
    {
      //显示热床温度
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.hot_bed_temp);
      CopyTextDisplayRangeInfo(HotBedTempTextRange, HotBedTempTextRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTempTextRange, TextRangeBuf);

      //显示斜杠
      if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
      {
        DisplayTextDefault((PUCHAR)"/", (100 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 62);
      }
      else
      {
        DisplayTextDefault((PUCHAR)"/", (335 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 36);
      }

      //显示热床目标温度
      snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_hot_bed_temp);
      CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf);
    }

    //显示打印进度
    snprintf(TextBuffer, sizeof(TextBuffer), "%2d%%", (INT)t_gui.print_percent);

    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      if (Draw_progressBar(t_gui.printfile_size, t_gui.file_size))
      {
        //ReadTextDisplayRangeInfo(PrintScheduleTextRange,PrintScheduleShape);
        CopyTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape, TextRangeBuf);
        DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintScheduleTextRange, TextRangeBuf);
      }
    }
    else
    {
      CopyTextDisplayRangeInfo(PrintScheduleTextRange, PrintScheduleShape, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintScheduleTextRange, TextRangeBuf);
    }

    //显示打印时间
    second = t_gui.printed_time_sec;
    hour = second / 3600;
    minute = (second - (hour * 3600)) / 60;
    second = (second - (hour * 3600)) % 60;
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d:%2d:%2d", hour, minute, second);
    CopyTextDisplayRangeInfo(PrintTimeTextRange, TimeTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintTimeTextRange, TextRangeBuf);

    if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      snprintf(TextBuffer, sizeof(TextBuffer), "%5.1fmm/s", (t_gui.print_speed_value * t_gui.cura_speed) / 100.0F);
      CopyTextDisplayRangeInfo(SpeedRange, SpeedRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextBuffer, SpeedRange, TextRangeBuf);
    }
  }
}

#define PauseInterface 1
#define ResumeInterface 2

typedef enum
{
  NO_KEY = 0,
  PrintSetLeft_KEY,
  PrintSetRight_KEY,
  PrintSetCavity_KEY,
  PauseOrResume_KEY,
  ChangeFilament_KEY,
  Stop_KEY,
} KEYValueTypeDef;

KEYValueTypeDef Custom_get_key_value(bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  if (touchxy(132, 36, 232, 66)) //喷嘴温度 || 打印速度
  {
    KEYValue = PrintSetLeft_KEY;
  }

  if (!t_custom_services.disable_hot_bed) //热床温度
  {
    if (touchxy(324, 36, 424, 66)) //热床温度 || 风扇速度
    {
      KEYValue = PrintSetRight_KEY;
    }
  }

  if (touchxy(132, 86, 232, 116))
  {
    KEYValue = PrintSetCavity_KEY;
  }

  if (touchxy(48, 212, 144, 280)) //暂停打印 或 继续打印
  {
    KEYValue = PauseOrResume_KEY;
  }

  if (have_chg_filament && touchxy(192, 212, 290, 282)) //中途换料
  {
    KEYValue = ChangeFilament_KEY;
  }

  if (touchxy(338, 212, 434, 280)) //停止打印按钮
  {
    KEYValue = Stop_KEY;
  }

  return KEYValue;
}

KEYValueTypeDef sgcode_get_key_value(bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  if (touchxy(0, 0, 255, 45) || touchxy(0, 130, 255, 170)) //喷嘴温度 || 打印速度
  {
    KEYValue = PrintSetLeft_KEY;
  }

  if (!t_custom_services.disable_hot_bed) //热床温度
  {
    if (touchxy(0, 46, 255, 90)) //热床温度 || 风扇速度
    {
      KEYValue = PrintSetRight_KEY;
    }
  }

  if (touchxy(0, 180, 89, 260)) //暂停打印 或 继续打印
  {
    KEYValue = PauseOrResume_KEY;
  }

  if (have_chg_filament && touchxy(90, 180, 179, 260)) //中途换料
  {
    KEYValue = ChangeFilament_KEY;
  }

  if (touchxy(180, 180, 265, 260)) //停止打印按钮
  {
    KEYValue = Stop_KEY;
  }

  return KEYValue;
}

KEYValueTypeDef normal_get_key_value(bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  if (touchxy(53, 0, 225, 67))
  {
    KEYValue = PrintSetLeft_KEY;
  }

  if (!t_custom_services.disable_hot_bed) //NotM14_Right
  {
    if (touchxy(225, 0, 480, 67))
    {
      KEYValue = PrintSetRight_KEY;
    }
  }

  if (have_chg_filament)
  {
    if (touchxy(50, 212, 144, 280)) //暂停打印 或 继续打印
    {
      KEYValue = PauseOrResume_KEY;
    }

    if (touchxy(194, 210, 290, 282)) //中途换料
    {
      KEYValue = ChangeFilament_KEY;
    }

    if (touchxy(338, 212, 434, 282)) //停止打印按钮
    {
      KEYValue = Stop_KEY;
    }
  }
  else
  {
    if (touchxy(94, 212, 190, 278)) //暂停打印 或 继续打印
    {
      KEYValue = PauseOrResume_KEY;
    }

    if (touchxy(288, 212, 384, 280)) //停止打印按钮
    {
      KEYValue = Stop_KEY;
    }
  }

  return KEYValue;
}

INT InterfaceTouchCheck(UINT8 SelectPauseOrResumeInterface, bool have_chg_filament)
{
  KEYValueTypeDef KEYValue = NO_KEY;

  /******************************按键扫描**************************************/
  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    KEYValue = Custom_get_key_value(have_chg_filament);
  }
  else if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
  {
    KEYValue = sgcode_get_key_value(have_chg_filament);
  }
  else
  {
    KEYValue = normal_get_key_value(have_chg_filament);
  }

  /****************************按键值命令执行*******************************************/
  switch (KEYValue)
  {
  case PrintSetLeft_KEY:
    if (t_custom_services.disable_hot_bed) //M14
    {
      gui_set_curr_display(PrintSet_M14);
    }
    else  //NotM14_Left
    {
      gui_set_curr_display(PrintSet_NotM14_Left);
    }

    return 1;

  case PrintSetRight_KEY:
    gui_set_curr_display(PrintSet_NotM14_Right);
    return 1;

  case PrintSetCavity_KEY:
    gui_set_curr_display(PrintSet_Cavity);
    return 1;

  case PauseOrResume_KEY:
    if (PauseInterface == SelectPauseOrResumeInterface)
    {
      gui_set_curr_display(PausePrintF);
    }
    else
    {
      gui_set_curr_display(ResumePrintF);
    }

    return 1;

  case ChangeFilament_KEY:
    if (have_chg_filament)
    {
      gui_set_curr_display(ChangeFilamentConfirm);
      return 1;
    }

  case Stop_KEY:
    gui_set_curr_display(stopprintF);
    return 1;

  default:
    break;
  }

  /****************************电脑命令执行*******************************************/
  if ((t_gui_p.IsComputerControlToPausePrint) || (t_gui_p.IsComputerControlToResumePrint)) //电脑控制了暂停、继续
  {
    t_gui_p.IsComputerControlToPausePrint = 0;
    t_gui_p.IsComputerControlToResumePrint = 0;
    gui_set_curr_display(maindisplayF);  //设置要显示的界面函数

    if (PauseInterface == SelectPauseOrResumeInterface)
    {
      respond_gui_send_sem(PausePrintValue);
      pauseprint = 1;              //暂停打印标志
    }
    else
    {
      respond_gui_send_sem(ResumePrintValue);
      pauseprint = 0;              //暂停打印标志
    }

    return 1;
  }

  if (t_gui_p.IsComputerControlToStopPrint) //电脑端控制停止打印
  {
    t_gui_p.IsComputerControlToStopPrint = 0;
    print_flage = 0;
    pauseprint = 0;
    gui_set_curr_display(maindisplayF);
    respond_gui_send_sem(StopPrintValue);
    return 1;
  }

  //打印中检测到没有材料
  if (t_gui_p.IsNotHaveMatInPrint)
  {
    gui_set_curr_display(NoHaveMatWaringInterface);
    return 1;
  }

  return 0;
}

INT pause_or_resume_interface(INT status, bool have_chg_filament)
{
  if (gui_is_refresh())            //检查是否需要更新，初始化
  {
    if (1 == t_sys_data_current.enable_cavity_temp)
    {
      if (PauseInterface == status)
      {
        display_picture(107);
      }
      else if (ResumeInterface == status)
      {
        display_picture(108);
      }

      if (!have_chg_filament)
      {
        LCD_Fill(192, 210, 290, 282, BACKBLUE); //遮住中途换料
      }
    }
    else if (strstr(SettingInfoToSYS.PrintFileName, ".sgcode"))
    {
      if (PauseInterface == status)
      {
        display_picture(96);
      }
      else if (ResumeInterface == status)
      {
        display_picture(97);
      }

      if (!have_chg_filament)
      {
        LCD_Fill(95, 185, (t_sys.lcd_ssd1963_43_480_272 ? 165 : 175), 255, BACKBLUE); //遮住中途换料
      }

      diplayBMP(t_sys.lcd_ssd1963_43_480_272 ? 4 : 24); //显示未压缩的bmp图片
    }
    else if (t_custom_services.disable_hot_bed)
    {
      if (PauseInterface == status)
      {
        display_picture(have_chg_filament ? 33 : 36);
      }
      else if (ResumeInterface == status)
      {
        display_picture(have_chg_filament ? 34 : 37);
      }
    }
    else
    {
      if (PauseInterface == status)
      {
        display_picture(have_chg_filament ? 2 : 28);
      }
      else if (ResumeInterface == status)
      {
        display_picture(have_chg_filament ? 3 : 29);
      }
    }

    PrintInterfaceDisplayInit();
    PrintInterfaceDisplayText();
  }

  if (InterfaceTouchCheck(status, have_chg_filament))
  {
    return 1;
  }

  if (gui_is_rtc())   //根据rtc信号更新数值显示
  {
    PrintInterfaceDisplayText();

    if (CheckIsPrintFinish())
    {
      return 1;
    }

    if (PauseInterface == status)
    {
      if ((!have_chg_filament) && CheckIsHaveUdisk())
      {
        return 1;
      }
    }
    else if (ResumeInterface == status)
    {
      if (CheckIsHaveUdisk())
      {
        return 1;
      }
    }
  }

  return 0;
}

//#define DEBUG_Tempertuer_PID//debug pid temperatuer

void MainInterfaceDisplayInit(void)
{
  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    SetTextDisplayRange(138, 32, 12 * 3, 24, &NozzleTempTextRange);
    SetTextDisplayRange(344, 32, 12 * 3, 24, &HotBedTempTextRange);
    SetTextDisplayRange(138, 86, 12 * 3, 24, &CavityTempTextRange);
    SetTextDisplayRange((138 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 32, 12 * 3, 24, &NozzleTargetTempTextRange);
    SetTextDisplayRange((344 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 32, 12 * 3, 24, &HotBedTargetTempTextRange);
    SetTextDisplayRange((138 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 86, 12 * 3, 24, &CavityTargetTempTextRange);
    ReadTextDisplayRangeInfo(CavityTempTextRange, CavityTempTextRangeBuf);
    ReadTextDisplayRangeInfo(CavityTargetTempTextRange, CavityTargetTempTextRangeBuf);
  }
  else if (t_sys_data_current.model_id == AMP410W)
  {
    SetTextDisplayRange(344, 20, 12 * 3, 24, &NozzleTempTextRange);
    SetTextDisplayRange(344, 61, 12 * 3, 24, &HotBedTempTextRange);
    SetTextDisplayRange((344 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 20, 12 * 3, 24, &NozzleTargetTempTextRange);
    SetTextDisplayRange((344 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 61, 12 * 3, 24, &HotBedTargetTempTextRange);
  }
  else
  {
    SetTextDisplayRange(137, 35, 12 * 3, 24, &NozzleTempTextRange);
    SetTextDisplayRange(345, 35, 12 * 3, 24, &HotBedTempTextRange);
    SetTextDisplayRange((137 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 35, 12 * 3, 24, &NozzleTargetTempTextRange);
    SetTextDisplayRange((345 + (12 * 4)) + (t_sys.lcd_ssd1963_43_480_272 ? 10 : 0), 35, 12 * 3, 24, &HotBedTargetTempTextRange);
  }

  ReadTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf);
  ReadTextDisplayRangeInfo(HotBedTempTextRange, HotBedTempTextRangeBuf);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf);
  ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf);
#ifdef DEBUG_Tempertuer_PID
  SetTextDisplayRange(335, 114, 12 * 9, 24, &PrintTimeTextRange);
  ReadTextDisplayRangeInfo(PrintTimeTextRange, TimeTextRangeBuf);
#endif
}

extern void protect_nozzle(FLOAT hour);

void MainInterfaceDisplayText(void)
{
  CHAR TextBuffer[20];
  protect_nozzle(1.5);
  //显示喷嘴温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTempTextRange, NozzleTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTempTextRange, TextRangeBuf);

  //显示斜杠
  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    DisplayTextDefault((PUCHAR)"/", (138 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 32);
  }
  else if (t_sys_data_current.model_id == AMP410W)
  {
    DisplayTextDefault((PUCHAR)"/", (344 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 20);
  }
  else
  {
    DisplayTextDefault((PUCHAR)"/", (137 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 35);
  }

  //显示喷嘴目标温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTargetTempTextRange, TextRangeBuf);

  if (!t_custom_services.disable_hot_bed)
  {
    //显示热床温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTempTextRange, HotBedTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTempTextRange, TextRangeBuf);

    //显示斜杠
    if (1 == t_sys_data_current.enable_cavity_temp)
    {
      DisplayTextDefault((PUCHAR)"/", (344 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 32);
    }
    else if (t_sys_data_current.model_id == AMP410W)
    {
      DisplayTextDefault((PUCHAR)"/", (344 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 61);
    }
    else
    {
      DisplayTextDefault((PUCHAR)"/", (345 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 35);
    }

    //显示热床目标温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTargetTempTextRange, TextRangeBuf);
  }

  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    //显示喷嘴温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.cavity_temp);
    CopyTextDisplayRangeInfo(CavityTempTextRange, CavityTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, CavityTempTextRange, TextRangeBuf);
    //显示斜杠
    DisplayTextDefault((PUCHAR)"/", (138 + (12 * 3)) + (t_sys.lcd_ssd1963_43_480_272 ? 5 : 0), 86); //直接显示到lcd
    //显示喷嘴目标温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d", (INT)t_gui.target_cavity_temp);
    CopyTextDisplayRangeInfo(CavityTargetTempTextRange, CavityTargetTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, CavityTargetTempTextRange, TextRangeBuf);
  }

#ifdef DEBUG_Tempertuer_PID
  snprintf(TextBuffer, sizeof(TextBuffer), "f%0.4f ", t_sys_data_current.pid_output_factor);
  CopyTextDisplayRangeInfo(PrintTimeTextRange, TimeTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintTimeTextRange, TextRangeBuf);
#endif
}

void lcd_debug_interface(void)
{
#ifdef LCD_DEBUG
  extern menufunc_t lcdtest_lastdisplay;
  extern void board_test_cal_touch_count(void);

  //TFTLCD屏误触发计数测试，卢工2017.4.20
  if (touchxy(192, 109, 300, 200)) //隐藏按键，在准备键和U盘键之间
  {
    //    t_sys_data_current.have_set_machine = 0;
    //    motion_3d.enable_board_test = 1;
    lcdtest_lastdisplay = currentdisplay;
    gui_set_curr_display(board_test_cal_touch_count);
  }

#endif
}

void debug_presure_sensor(void)
{
  //#define Debug_PresureSensor
#ifdef Debug_PresureSensor
#include "functioncustom.h"
  extern menufunc_t lcdtest_lastdisplay;

  if (touchxy(0, 109, 105, 200)) //隐藏按键，在准备键和U盘键之间
  {
    lcdtest_lastdisplay = currentdisplay;
    gui_set_curr_display(board_test_pressure);
    return 1;
  }

#endif
}

INT MainInterfaceTouchCheck(void)
{
  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    if (touchxy(138, 32, 238, 62))
    {
      gui_set_curr_display(PrintSet_NotM14_Left);
      return 1;
    }

    if (touchxy(344, 32, 444, 62))
    {
      gui_set_curr_display(PrintSet_NotM14_Right);
      return 1;
    }

    if (touchxy(138, 86, 238, 116))
    {
      gui_set_curr_display(PrintSet_Cavity);
      return 1;
    }
  }
  else if (t_sys_data_current.model_id == AMP410W)
  {
    if (touchxy(250, 0, 480, 50))
    {
      gui_set_curr_display(PrintSet_NotM14_Left);
      return 1;
    }

    if (touchxy(250, 50, 480, 100))
    {
      gui_set_curr_display(PrintSet_NotM14_Right);
      return 1;
    }
  }
  else
  {
    if (touchxy(53, 0, 240, 65))
    {
      if (t_custom_services.disable_hot_bed) //M14
      {
        gui_set_curr_display(PrintSet_M14);
      }
      else  //NotM14_Left
      {
        gui_set_curr_display(PrintSet_NotM14_Left);
      }

      return 1;
    }

    if (!t_custom_services.disable_hot_bed) //NotM14_Right
    {
      if (touchxy(240, 0, 480, 65))
      {
        gui_set_curr_display(PrintSet_NotM14_Right);
        return 1;
      }
    }
  }

  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    if (touchxy(121, 138, 198, 218))
    {
      gui_set_curr_display(prepareF);
      return 1;
    }

    debug_presure_sensor();
    lcd_debug_interface();

    if (touchxy(74, 228, 150, 306))
    {
      gui_set_curr_display(settingF);
      return 1;
    }

    if (touchxy(170, 228, 244, 306))
    {
      gui_set_curr_display(statusF);
      return 1;
    }

    if (touchxy(302, 142, 402, 306))
    {
      UpdateUdiskStatus();
      sys_os_delay(50);

      if (t_gui_p.SDIsInsert)
      {
        respond_gui_send_sem(OpenSDCardValue);
        gui_set_curr_display(filescanF);
        return 1;
      }
      else
      {
        gui_set_curr_display(gui_view::NoUdiskF);
        return 1;
      }
    }
  }
  else
  {
    if (touchxy(107, 109, 191, 193))
    {
      gui_set_curr_display(prepareF);
      return 1;
    }

    debug_presure_sensor();
    lcd_debug_interface();

    if (touchxy(55, 205, 140, 292))
    {
      gui_set_curr_display(settingF);
      return 1;
    }

    if (touchxy(163, 205, 243, 292))
    {
      gui_set_curr_display(statusF);
      return 1;
    }

    if (touchxy(308, 111, 418, 292))
    {
      UpdateUdiskStatus();
      sys_os_delay(50);

      if (t_gui_p.SDIsInsert)
      {
        respond_gui_send_sem(OpenSDCardValue);
        gui_set_curr_display(filescanF);
        return 1;
      }
      else
      {
        gui_set_curr_display(gui_view::NoUdiskF);
        return 1;
      }
    }
  }

  if (t_power_off.flag) //断电续打
  {
    t_power_off.flag = 0;
    strcpy(printname, t_power_off.file_name);
    gui_set_curr_display(PowerOffRecover);
    return 1;
  }

  return 0;
}

INT MainInterface(void)
{
  if (gui_is_refresh())
  {
    Gui_Mode = MODE_MIAN;

    if (t_custom_services.disable_hot_bed)
    {
      display_picture(32);
    }
    else
    {
      if (1 == t_sys_data_current.enable_cavity_temp)
      {
        display_picture(106);
      }
      else if (t_sys_data_current.model_id == AMP410W)
      {
        display_picture(65);
      }
      else
      {
        display_picture(1);
      }
    }

    MainInterfaceDisplayInit();
    MainInterfaceDisplayText();
  }

  if (MainInterfaceTouchCheck())
  {
    return 1;
  }

  if (gui_is_rtc())
  {
    MainInterfaceDisplayText();
  }

  return 1;
}

#define HeatFinish 1
#define CurrentInterfaceIsNotPrintInterface 0
#define CurrentInterfaceIsPauseAndNoChangeFilament  1
#define CurrentInterfaceIsPauseAndHaveChangeFilament  2
#define CurrentInterfaceIsResumeAndNoChangeFilament  3
#define CurrentInterfaceIsResumeAndHaveChangeFilament  4

void IsRefreshPrintInterface(UINT8 CurrentInterface)
{
  static UINT8 LastPrintInterfaceValue = CurrentInterfaceIsNotPrintInterface;

  if (CurrentInterface != CurrentInterfaceIsNotPrintInterface)
  {
    if (LastPrintInterfaceValue != CurrentInterface) //打印时，当前要显示的界面和上次显示的界面不一样则刷新界面图片
    {
      gui_set_curr_display(currentdisplay);
    }
  }

  LastPrintInterfaceValue = CurrentInterface;
}

void PrintInterface(void)
{
  if (HeatFinish == t_gui_p.ChangeFilamentHeatStatus) //完成了加热后显示含有中途换料的界面
  {
    IsRefreshPrintInterface(!pauseprint ? CurrentInterfaceIsPauseAndHaveChangeFilament : CurrentInterfaceIsResumeAndHaveChangeFilament);

    if (pause_or_resume_interface(!pauseprint ? PauseInterface : ResumeInterface, true))
    {
      return;
    }
  }
  else  //没有完成加热和归零后显示没有含有中途换料的界面
  {
    IsRefreshPrintInterface(!pauseprint ? CurrentInterfaceIsPauseAndNoChangeFilament : CurrentInterfaceIsResumeAndNoChangeFilament);

    if (pause_or_resume_interface(!pauseprint ? PauseInterface : ResumeInterface, false))
    {
      return;
    }
  }

  if ((!pauseprint) && t_gui_p.IsDoorOpen)
  {
    gui_set_curr_display(DoorOpenWarningInfo_Printing);
  }

  lcd_debug_interface();
}

void maindisplayF(void)
{
  if (print_flage == 1) //正在打印，显示打印界面
  {
    PrintInterface();
  }
  else if (print_flage == 0) //没有打印，显示主界面
  {
    IsRefreshPrintInterface(CurrentInterfaceIsNotPrintInterface);

    if (MainInterface())
    {
      return ;
    }
  }
}

#ifdef __cplusplus
} // extern "C" {
#endif

