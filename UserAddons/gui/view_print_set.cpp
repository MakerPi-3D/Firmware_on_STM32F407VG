#include "user_interface.h"
#include "Alter.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "view_common.h"
#include "view_commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include  "interface.h"
#include "config_model_tables.h"
//#include "ConfigurationStore.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "threed_engine.h"

#define NoKey 0
#define NozzleTempDown 1
#define NozzleTempUp 2
#define BedTempDown 3
#define BedTempUp 4
#define PrintSpeedDown 5
#define PrintSpeedUp 6
#define FanSpeedDown 7
#define FanSpeedUp 8
#define CavityTempDown 9
#define CavityTempUp 10
#define CavityTempOnDown 11
#define CavityTempOnUp 12
#define LedOn   13
#define LedOff   14
#define Timeoutms 200
#define FirstTimeoutms 600

extern bool is_auto_bed_level_zero(void);
extern INT temperature_get_heater_maxtemp(INT axis);

static UINT8 KeyValue = NoKey;

static ULONG Timeout = 0;
static ULONG FirstTimeout = 0;

static INT SetNozzleTempValue = 0;
static INT SetBedTempValue = 0;
static UINT32 SetPrintSpeedValue = 100;
static INT SetFanSpeedValue = 0;
static INT SetCavityTempValue = 100;
static INT SetCavityTempOnValue = 0;
bool SetLedValue = false;

static UINT8 SlowUpdateData = 0;
static UINT8 FastUpdateData = 0;

static UINT8 IsLoadFilamentInterface = 0; //是否是进料的调节温度界面
static UINT8 IsUnloadFilamentInterface = 0; //是否是退料的调节温度界面

static UINT8 IsPowerOffRecoverReady = 0; //是否是断电续打恢复准备界面

//#ifdef ENABLE_AUTO_BED_LEVELING
extern void gui_bed_level_nozzle_heat(void);

//#endif //#ifdef ENABLE_AUTO_BED_LEVELING

UINT8 ScanKey_M14(void)
{
  if (IstouchxyDown(364, 0, 480, 82))
  {
    return NozzleTempDown;
  }
  else if (IstouchxyDown(250, 0, 364, 82))
  {
    return NozzleTempUp;
  }
  else if (IstouchxyDown(364, 82, 480, 179))
  {
    return FanSpeedDown;
  }
  else if (IstouchxyDown(250, 82, 364, 179))
  {
    return FanSpeedUp;
  }
  else if (IstouchxyDown(364, 179, 480, 260))
  {
    return PrintSpeedDown;
  }
  else if (IstouchxyDown(250, 179, 364, 260))
  {
    return PrintSpeedUp;
  }
  else
  {
    return NoKey;
  }
}

UINT8 ScanKey_NotM14_Left(void)
{
  if (IstouchxyDown((t_sys.lcd_ssd1963_43_480_272 ? 344 : 364), 40, 480, 129))
  {
    return NozzleTempDown;
  }
  else if (IstouchxyDown(255, 40, (t_sys.lcd_ssd1963_43_480_272 ? 344 : 364), 129))
  {
    return NozzleTempUp;
  }
  else if (IstouchxyDown(364, 129, 480, 220))
  {
    return PrintSpeedDown;
  }
  else if (IstouchxyDown(255, 129, 364, 220))
  {
    return PrintSpeedUp;
  }
  else
  {
    return NoKey;
  }
}

UINT8 ScanKey_NotM14_Right(void)
{
  if (IstouchxyDown((t_sys.lcd_ssd1963_43_480_272 ? 364 : 344), 40, 480, 129))
  {
    return BedTempDown;
  }
  else if (IstouchxyDown(255, 40, (t_sys.lcd_ssd1963_43_480_272 ? 364 : 344), 129))
  {
    return BedTempUp;
  }
  else if (IstouchxyDown(364, 129, 480, 220))
  {
    return FanSpeedDown;
  }
  else if (IstouchxyDown(255, 129, 364, 220))
  {
    return FanSpeedUp;
  }
  else
  {
    return NoKey;
  }
}

UINT8 ScanKey_NotM14_Right_withLed(void)
{
  if (IstouchxyDown((t_sys.lcd_ssd1963_43_480_272 ? 364 : 344), 10, 480, 83))
  {
    return BedTempDown;
  }
  else if (IstouchxyDown(255, 10, (t_sys.lcd_ssd1963_43_480_272 ? 364 : 344), 83))
  {
    return BedTempUp;
  }
  else if (IstouchxyDown(364, 95, 480, 177))
  {
    return FanSpeedDown;
  }
  else if (IstouchxyDown(255, 95, 364, 177))
  {
    return FanSpeedUp;
  }
  else if (IstouchxyDown(364, 189, 480, 242))
  {
    return LedOff;
  }
  else if (IstouchxyDown(255, 189, 364, 242))
  {
    return LedOn;
  }
  else
  {
    return NoKey;
  }
}

UINT8 ScanKey_Cavity(void)
{
  if (IstouchxyDown(364, 40, 480, 129))
  {
    return CavityTempDown;
  }
  else if (IstouchxyDown(255, 40, 364, 129))
  {
    return CavityTempUp;
  }
  else if (IstouchxyDown(364, 129, 480, 220))
  {
    return CavityTempOnDown;
  }
  else if (IstouchxyDown(255, 129, 364, 220))
  {
    return CavityTempOnUp;
  }
  else
  {
    return NoKey;
  }
}


void LongPress(void)
{
  if (IstouchxyUp())
  {
    KeyValue = 0;
    (void)sys_os_delay(200);
  }
  else if (Timeout < sys_task_get_tick_count())
  {
    Timeout = Timeoutms + sys_task_get_tick_count();
    SlowUpdateData = 0;
    FastUpdateData = 1;
  }
}

void ShortPress(void)
{
  SlowUpdateData = 1;
  FastUpdateData = 0;
}

void UpdateData(void)
{
  switch (KeyValue)
  {
  case NozzleTempDown:
    if (1 == FastUpdateData)
    {
      SetNozzleTempValue = SetNozzleTempValue - 10;
    }
    else
    {
      SetNozzleTempValue = SetNozzleTempValue - 1;
      KeyValue = NoKey;
    }

    break;

  case NozzleTempUp:
    if (1 == FastUpdateData)
    {
      SetNozzleTempValue = SetNozzleTempValue + 10;
    }
    else
    {
      SetNozzleTempValue = SetNozzleTempValue + 1;
      KeyValue = NoKey;
    }

    break;

  case BedTempDown:
    if (1 == FastUpdateData)
    {
      SetBedTempValue = SetBedTempValue - 10;
    }
    else
    {
      SetBedTempValue = SetBedTempValue - 1;
      KeyValue = NoKey;
    }

    break;

  case BedTempUp:
    if (1 == FastUpdateData)
    {
      SetBedTempValue = SetBedTempValue + 10;
    }
    else
    {
      SetBedTempValue = SetBedTempValue + 1;
      KeyValue = NoKey;
    }

    break;

  case PrintSpeedDown:
    if (1 == FastUpdateData)
    {
      SetPrintSpeedValue = SetPrintSpeedValue - 40; //0.4F;
    }
    else
    {
      SetPrintSpeedValue = SetPrintSpeedValue - 10; //0.1F;
      KeyValue = NoKey;
    }

    break;

  case PrintSpeedUp:
    if (1 == FastUpdateData)
    {
      SetPrintSpeedValue = SetPrintSpeedValue + 40; //0.4F;
    }
    else
    {
      SetPrintSpeedValue = SetPrintSpeedValue + 10; //0.1F;
      KeyValue = NoKey;
    }

    break;

  case FanSpeedDown:
    //      if(1==FastUpdateData)
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue-10;
    //      }
    //      else
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue-1;
    //        KeyValue=NoKey;
    //      }
    SetFanSpeedValue = 0;
    break;

  case FanSpeedUp:
    //      if(1==FastUpdateData)
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue+10;
    //      }
    //      else
    //      {
    //        SetFanSpeedValue=SetFanSpeedValue+1;
    //        KeyValue=NoKey;
    //      }
    SetFanSpeedValue = 255;
    break;

  case CavityTempDown:
    if (1 == FastUpdateData)
    {
      SetCavityTempValue = SetCavityTempValue - 10; //0.4F;
    }
    else
    {
      SetCavityTempValue = SetCavityTempValue - 1; //0.1F;
      KeyValue = NoKey;
    }

    break;

  case CavityTempUp:
    if (1 == FastUpdateData)
    {
      SetCavityTempValue = SetCavityTempValue + 10; //0.4F;
    }
    else
    {
      SetCavityTempValue = SetCavityTempValue + 1; //0.1F;
      KeyValue = NoKey;
    }

    break;

  case CavityTempOnDown:
    SetCavityTempOnValue = 0;
    break;

  case CavityTempOnUp:
    SetCavityTempOnValue = 1;
    break;

  case LedOn:
    SetLedValue = true;
    break;

  case LedOff:
    SetLedValue = false;
    break;

  default:
    break;
  }
}

void DataRangeLimit(void)
{
  if (SetNozzleTempValue > (temperature_get_heater_maxtemp(0) - 25))
  {
    SetNozzleTempValue = temperature_get_heater_maxtemp(0) - 25;
  }

  if (SetNozzleTempValue < 0)
  {
    SetNozzleTempValue = 0;
  }

  if (t_custom_services.disable_abs) //不能打印ABS,热床的可调最大温度限制
  {
    if (SetBedTempValue > 70)
    {
      SetBedTempValue = 70;
    }
  }
  else
  {
    if (SetBedTempValue > 120)
    {
      SetBedTempValue = 120;
    }

    if ((PICTURE_IS_JAPANESE == t_sys_data_current.pic_id) && (M4040 == t_sys_data_current.model_id) && (SetBedTempValue > 100))
    {
      SetBedTempValue = 100;
    }
  }

  if (SetBedTempValue < 0)
  {
    SetBedTempValue = 0;
  }

  if (SetFanSpeedValue > 255)
  {
    SetFanSpeedValue = 255;
  }

  if (SetFanSpeedValue < 0)
  {
    SetFanSpeedValue = 0;
  }

  if (SetPrintSpeedValue > 200) //2.0F)
  {
    SetPrintSpeedValue = 200; //2.0F;
  }

  if (SetPrintSpeedValue < 10) //0.1F)
  {
    SetPrintSpeedValue = 10; //0.1F;
  }

  if (1 == t_sys_data_current.enable_cavity_temp)
  {
    if (SetCavityTempValue > t_gui_p.cavity_temp_max_value)
    {
      SetCavityTempValue = t_gui_p.cavity_temp_max_value;
    }
    else if (SetCavityTempValue < 0)
    {
      SetCavityTempValue = 0;
    }
  }

#if LASER_MODE

  if (t_sys_data_current.IsLaser)
  {
    if (SetNozzleTempValue > 60)
      SetNozzleTempValue = 60;
  }

#endif
}

void DisplayText_M14(void)
{
  CHAR Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetNozzleTempValue);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, NozzleTargetTempTextRange, TextRangeBuf);
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetFanSpeedValue);
  CopyTextDisplayRangeInfo(FanSpeedTextSharp, FanSpeedRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, FanSpeedTextSharp, TextRangeBuf);
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", SetPrintSpeedValue / 100.0F); // float
  CopyTextDisplayRangeInfo(PrintSpeedTextSharp, PrintSpeedRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, PrintSpeedTextSharp, TextRangeBuf);
}

void DisplayText_NotM14_Left(void)
{
  CHAR Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetNozzleTempValue);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, NozzleTargetTempTextRange, TextRangeBuf);
  snprintf(Textbuffer, sizeof(Textbuffer), "%.1f", SetPrintSpeedValue / 100.0F); // float
  CopyTextDisplayRangeInfo(PrintSpeedTextSharp, PrintSpeedRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, PrintSpeedTextSharp, TextRangeBuf);
}

void DisplayText_NotM14_Right(void)
{
  CHAR Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetBedTempValue);
  CopyTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, HotBedTargetTempTextRange, TextRangeBuf);
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetFanSpeedValue);
  CopyTextDisplayRangeInfo(FanSpeedTextSharp, FanSpeedRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, FanSpeedTextSharp, TextRangeBuf);

  if (!t_custom_services.enable_led_light)
    return;

  if (SetLedValue)
    snprintf(Textbuffer, sizeof(Textbuffer), "ON");
  else
    snprintf(Textbuffer, sizeof(Textbuffer), "OFF");

  CopyTextDisplayRangeInfo(LedSwitchTextSharp, LedSwitchRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, LedSwitchTextSharp, TextRangeBuf);
}

void DisplayText_Cavity(void)
{
  CHAR Textbuffer[20];
  snprintf(Textbuffer, sizeof(Textbuffer), "%3d", SetCavityTempValue);
  CopyTextDisplayRangeInfo(CavityTargetTempTextRange, CavityTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, CavityTargetTempTextRange, TextRangeBuf);
  snprintf(Textbuffer, sizeof(Textbuffer), "%s", (SetCavityTempOnValue ? "on" : "off"));
  CopyTextDisplayRangeInfo(CavityTempOnTextSharp, CavityTempOnRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)Textbuffer, CavityTempOnTextSharp, TextRangeBuf);
}

void SetValue_M14(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + sys_task_get_tick_count();
    FirstTimeout = FirstTimeoutms + sys_task_get_tick_count();
    KeyValue = ScanKey_M14();
  }
  else
  {
    if (FirstTimeout < sys_task_get_tick_count())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_M14();
  }
}

void SetValue_NotM14_Left(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + sys_task_get_tick_count();
    FirstTimeout = FirstTimeoutms + sys_task_get_tick_count();
    KeyValue = ScanKey_NotM14_Left();
  }
  else
  {
    if (FirstTimeout < sys_task_get_tick_count())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_NotM14_Left();
  }
}

void SetValue_NotM14_Right(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + sys_task_get_tick_count();
    FirstTimeout = FirstTimeoutms + sys_task_get_tick_count();

    if (t_custom_services.enable_led_light)
      KeyValue = ScanKey_NotM14_Right_withLed();
    else
      KeyValue = ScanKey_NotM14_Right();
  }
  else
  {
    if (FirstTimeout < sys_task_get_tick_count())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_NotM14_Right();
  }
}

void GetPrintSpeedValue(void)
{
  SetPrintSpeedValue = t_gui.print_speed_value; ///100.0F;   //转换成小数  0-200   0-2.0
}

void GetFanSpeedValue(void)
{
  SetFanSpeedValue = t_gui.fan_speed_value;
}

void GetNozzleTargetTempValue(void)
{
  SetNozzleTempValue = t_gui.target_nozzle_temp;
}

void GetBedTargetTempValue(void)
{
  SetBedTempValue = t_gui.target_hot_bed_temp;
}

void RefreshTheInterface_M14(void)
{
  display_picture(35);
  SetTextDisplayRange(TextDisplayX, 35, 12 * 3, 24, &NozzleTargetTempTextRange);
  SetTextDisplayRange(TextDisplayX, 126, 12 * 3, 24, &FanSpeedTextSharp);
  ReadTextDisplayRangeInfo(FanSpeedTextSharp, FanSpeedRangeBuf);
  SetTextDisplayRange(TextDisplayX, 211, 12 * 3, 24, &PrintSpeedTextSharp);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf);
  ReadTextDisplayRangeInfo(PrintSpeedTextSharp, PrintSpeedRangeBuf);
  GetNozzleTargetTempValue();
  GetFanSpeedValue();
  GetPrintSpeedValue();
  DisplayText_M14();
}

void RefreshTheInterface_NotM14_Left(void)
{
  display_picture(18);
  SetTextDisplayRange(TextDisplayX, 75, 12 * 3, 24, &NozzleTargetTempTextRange);
  SetTextDisplayRange(TextDisplayX, 168, 12 * 3, 24, &PrintSpeedTextSharp);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange, NozzleTargetTempTextRangeBuf);
  ReadTextDisplayRangeInfo(PrintSpeedTextSharp, PrintSpeedRangeBuf);
  GetNozzleTargetTempValue();
  GetPrintSpeedValue();
  DisplayText_NotM14_Left();
}

void RefreshTheInterface_NotM14_Right(void)
{
  if (t_custom_services.enable_led_light)
  {
    display_picture(103);
    SetTextDisplayRange(TextDisplayX, 36, 12 * 3, 24, &HotBedTargetTempTextRange);
    SetTextDisplayRange(TextDisplayX, 128, 12 * 3, 24, &FanSpeedTextSharp);
    SetTextDisplayRange(TextDisplayX, 213, 12 * 3, 24, &LedSwitchTextSharp);
    ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf);
    ReadTextDisplayRangeInfo(FanSpeedTextSharp, FanSpeedRangeBuf);
    ReadTextDisplayRangeInfo(LedSwitchTextSharp, LedSwitchRangeBuf);
  }
  else
  {
    display_picture(41);
    SetTextDisplayRange(TextDisplayX, 75, 12 * 3, 24, &HotBedTargetTempTextRange);
    SetTextDisplayRange(TextDisplayX, 168, 12 * 3, 24, &FanSpeedTextSharp);
    ReadTextDisplayRangeInfo(HotBedTargetTempTextRange, HotBedTargetTempTextRangeBuf);
    ReadTextDisplayRangeInfo(FanSpeedTextSharp, FanSpeedRangeBuf);
  }

  GetBedTargetTempValue();
  GetFanSpeedValue();
  DisplayText_NotM14_Right();
}


void ConfirmKey_M14(void)
{
  SettingInfoToSYS.TargetNozzleTemp = SetNozzleTempValue;
  SettingInfoToSYS.FanSpeed = SetFanSpeedValue;
  SettingInfoToSYS.PrintSpeed = SetPrintSpeedValue;
  t_gui.print_speed_value = (UINT16)SetPrintSpeedValue;
  respond_gui_send_sem(PrintSetValue_M14);

  if (IsLoadFilamentInterface)
  {
    gui_set_curr_display(loadfilament0F);
  }
  else if (IsUnloadFilamentInterface)
  {
    gui_set_curr_display(unloadfilament0F);
  }
  else if (IsPowerOffRecoverReady)
  {
    gui_set_curr_display(PowerOffRecoverReady);
  }
  else if (is_auto_bed_level_zero())
  {
    gui_set_curr_display(gui_bed_level_nozzle_heat);
  }
  else
  {
    gui_set_curr_display(maindisplayF);
  }
}

void ConfirmKey_NotM14_Left(void)
{
  SettingInfoToSYS.TargetNozzleTemp = SetNozzleTempValue;
  SettingInfoToSYS.PrintSpeed = SetPrintSpeedValue;
  t_gui.print_speed_value = (UINT16)(SetPrintSpeedValue);
  respond_gui_send_sem(PrintSetValue_NotM14_Left);

  if (IsLoadFilamentInterface)
  {
    gui_set_curr_display(loadfilament0F);
  }
  else if (IsUnloadFilamentInterface)
  {
    gui_set_curr_display(unloadfilament0F);
  }
  else if (IsPowerOffRecoverReady)
  {
    gui_set_curr_display(PowerOffRecoverReady);
  }
  else if (is_auto_bed_level_zero())
  {
    gui_set_curr_display(gui_bed_level_nozzle_heat);
  }
  else
  {
    gui_set_curr_display(maindisplayF);
  }
}

void ConfirmKey_NotM14_Right(void)
{
  SettingInfoToSYS.TargetHotbedTemp = SetBedTempValue;
  SettingInfoToSYS.FanSpeed = SetFanSpeedValue;
  respond_gui_send_sem(PrintSetValue_NotM14_Right);

  if (IsPowerOffRecoverReady)
  {
    gui_set_curr_display(PowerOffRecoverReady);
  }
  else
  {
    gui_set_curr_display(maindisplayF);
  }
}

void CancelKey(void)
{
  if (IsLoadFilamentInterface)
  {
    gui_set_curr_display(loadfilament0F);
  }
  else if (IsUnloadFilamentInterface)
  {
    gui_set_curr_display(unloadfilament0F);
  }
  else if (IsPowerOffRecoverReady)
  {
    gui_set_curr_display(PowerOffRecoverReady);
  }
  else if (is_auto_bed_level_zero())
  {
    gui_set_curr_display(gui_bed_level_nozzle_heat);
  }
  else
  {
    gui_set_curr_display(maindisplayF);
  }
}
extern bool IsPrintSDFile(void);
void PrintSet_M14(void)
{
  if (gui_is_refresh())
  {
    RefreshTheInterface_M14();
    return;
  }

  if (touchxy(110, 255, 240, 320)) //确认键
  {
    ConfirmKey_M14();
    return ;
  }

  if (touchxy(240, 265, 380, 320)) //取消键
  {
    CancelKey();
    return ;
  }

  SetValue_M14();

  if ((pauseprint && print_flage) || (print_flage && (t_gui_p.ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
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
}

void PrintSet_NotM14_Left(void)
{
  if (gui_is_refresh())
  {
    RefreshTheInterface_NotM14_Left();
    return;
  }

  if (touchxy(110, 230, 240, 320)) //确认键
  {
    ConfirmKey_NotM14_Left();
    return ;
  }

  if (touchxy(240, 235, 380, 320)) //取消键
  {
    CancelKey();
    return ;
  }

  SetValue_NotM14_Left();

  if ((pauseprint && print_flage) || (print_flage && (t_gui_p.ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
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
}

void PrintSet_NotM14_Right(void)
{
  if (gui_is_refresh())
  {
    RefreshTheInterface_NotM14_Right();
    return;
  }

  if (touchxy(110, t_custom_services.enable_led_light ? 252 : 230, 240, 320)) //确认键
  {
    ConfirmKey_NotM14_Right();
    return ;
  }

  if (touchxy(240, t_custom_services.enable_led_light ? 252 : 235, 380, 320)) //取消键
  {
    CancelKey();
    return ;
  }

  SetValue_NotM14_Right();

  if ((pauseprint && print_flage) || (print_flage && (t_gui_p.ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
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
}

void ConfirmKey_Cavity(void)
{
  SettingInfoToSYS.TargetCavityTemp = SetCavityTempValue;
  SettingInfoToSYS.TargetCavityOnTemp = SetCavityTempOnValue;
  respond_gui_send_sem(PrintSetValue_Cavity);

  if (IsPowerOffRecoverReady)
  {
    gui_set_curr_display(PowerOffRecoverReady);
  }
  else
  {
    gui_set_curr_display(maindisplayF);
  }
}

void SetValue_Cavity(void)
{
  if (NoKey == KeyValue)
  {
    Timeout = Timeoutms + sys_task_get_tick_count();
    FirstTimeout = FirstTimeoutms + sys_task_get_tick_count();
    KeyValue = ScanKey_Cavity();
  }
  else
  {
    if (FirstTimeout < sys_task_get_tick_count())
    {
      LongPress();
    }
    else if (IstouchxyUp())
    {
      ShortPress();
    }
  }

  if ((1 == FastUpdateData) || (1 == SlowUpdateData))
  {
    UpdateData();
    FastUpdateData = 0;
    SlowUpdateData = 0;
    DataRangeLimit();
    DisplayText_Cavity();
  }
}

void RefreshTheInterface_Cavity(void)
{
  display_picture(109);
  SetTextDisplayRange(235, 75, 12 * 3, 24, &CavityTargetTempTextRange);
  SetTextDisplayRange(235, 168, 12 * 3, 24, &CavityTempOnTextSharp);
  ReadTextDisplayRangeInfo(CavityTargetTempTextRange, CavityTargetTempTextRangeBuf);
  ReadTextDisplayRangeInfo(CavityTempOnTextSharp, CavityTempOnRangeBuf);
  SetCavityTempValue = t_gui.target_cavity_temp;
  SetCavityTempOnValue = t_gui.target_cavity_temp_on;
  DisplayText_Cavity();
}

void PrintSet_Cavity(void)
{
  if (gui_is_refresh())
  {
    RefreshTheInterface_Cavity();
    return;
  }

  if (touchxy(110, 230, 240, 320)) //确认键
  {
    ConfirmKey_Cavity();
    return ;
  }

  if (touchxy(240, 235, 380, 320)) //取消键
  {
    CancelKey();
    return ;
  }

  SetValue_Cavity();

  if ((pauseprint && print_flage) || (print_flage && (t_gui_p.ChangeFilamentHeatStatus == 0))) //打印且暂停了，或打印且加热还没完成，拔出U盘则停止打印且返回主界面
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
}

void SetLoadFilamentNozzleTemp(void)
{
  IsLoadFilamentInterface = 1;

  if (t_custom_services.disable_hot_bed) //M14
  {
    PrintSet_M14();
  }
  else
  {
    PrintSet_NotM14_Left();
  }

  IsLoadFilamentInterface = 0;
}

void SetUnLoadFilamentNozzleTemp(void)
{
  IsUnloadFilamentInterface = 1;

  if (t_custom_services.disable_hot_bed) //M14
  {
    PrintSet_M14();
  }
  else
  {
    PrintSet_NotM14_Left();
  }

  IsUnloadFilamentInterface = 0;
}

void SetPowerOffRecoverNozzleTemp(void)
{
  IsPowerOffRecoverReady = 1;

  if (t_custom_services.disable_hot_bed) //M14
  {
    PrintSet_M14();
  }
  else
  {
    PrintSet_NotM14_Left();
  }

  IsPowerOffRecoverReady = 0;
}

void SetPowerOffRecoverHotBedTemp(void)
{
  IsPowerOffRecoverReady = 1;
  PrintSet_NotM14_Right();
  IsPowerOffRecoverReady = 0;
}

#ifdef __cplusplus
} // extern "C" {
#endif


