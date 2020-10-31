#include "machinecustom.h"
#include "user_fatfs.h"
#include "guicontrol.h"
#include "filamentcontrol.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "user_debug.h"
#include "midwaychangematerial.h"
#include "view_common.h"


#include "controlfunction.h"
#include "boardtest.h"
#include "PrintControl.h"
#include "midwaychangematerial.h"
#include "autobedlevelinterface.h"
#include "temperature.h"
#include "config_model_tables.h"
#ifdef CAL_Z_ZERO_OFFSET
#include "view_common.h"
#include "infrared_z_zero_adjust.h"
#include "infrared_bed_level_adjust.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

#include "interface.h"
#include "UdiskControl.h"
#include "PrintControl.h"
#include "RespondGUI.h"
#include "USBFileTransfer.h"

#include "lcd.h"

#define Pop 0
#define NotPop 1

  UINT8 MinTempWarningPopSet=Pop;
  UINT8 MaxTempWarningPopSet=Pop;
  UINT8 MaxTempBedWarningPopSet=Pop;
  UINT8 HeatFailWarningPopSet=Pop;
  UINT8 MachineSizeLimitWarningPopSet=Pop;
  UINT8 IWDGResetWarningPopSet=Pop;
  UINT8 FatfsFailWarningPopSet=Pop;
  UINT8 ThermistorFallsWarnPopSet=Pop;

  void PopWarningInfo(UINT8 WarningSelectValue)//弹出警告信息
  {
    UINT8 IsPop=NotPop;
    t_gui_p.WarningInfoSelect=WarningSelectValue;
    // 串口上传一次错误信息到jb-app，避免重复上传导致app崩溃
    static UINT8 WarningSelectValueTemp = 0;
    if(WarningSelectValueTemp != WarningSelectValue)
    {
      USER_EchoLogStr("WarningSelectValue:%d\r\n",WarningSelectValue);//串口上传信息到上位机2017.7.6
      WarningSelectValueTemp = WarningSelectValue;
    }
    switch(WarningSelectValue)
    {
    case MinTempWarning:
      if(Pop == MinTempWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case MaxTempWarning:
      if(Pop == MaxTempWarningPopSet)
      {
        IsPop=Pop;
        sg_grbl::temperature_disable_heater();
      }
      break;
    case MaxTempBedWarning:
      if(Pop == MaxTempBedWarningPopSet)
      {
        IsPop=Pop;
        sg_grbl::temperature_disable_heater();
      }
      break;
    case HeatFailWarning:
      if(Pop == HeatFailWarningPopSet)
      {
        IsPop=Pop;
        if(IsPrint()) // 加热失败，处于打印状态时，停止打印
        {
          respond_gui_send_sem(StopPrintValue);
        }
        else
        {
          sg_grbl::temperature_disable_heater();
        }
      }
      break;
    case XMinLimitWarning:
      if(Pop == MachineSizeLimitWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case YMinLimitWarning:
      if(Pop == MachineSizeLimitWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case ZMinLimitWarning:
      if(Pop == MachineSizeLimitWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case XMaxLimitWarning:
      if(Pop == MachineSizeLimitWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case YMaxLimitWarning:
      if(Pop == MachineSizeLimitWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case ZMaxLimitWarning:
      if(Pop == MachineSizeLimitWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case IWDGResetWarning:
      if(Pop == IWDGResetWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case FatfsWarning:
      if(Pop == FatfsFailWarningPopSet)
      {
        IsPop=Pop;
      }
      break;
    case ThermistorFallsWarning:
      if(Pop == ThermistorFallsWarnPopSet)
      {
        IsPop=Pop;
        if(IsPrint()) // 加热失败，处于打印状态时，停止打印
        {
          respond_gui_send_sem(StopPrintValue);
        }
        else
        {
          sg_grbl::temperature_disable_heater();
        }
      }
      break;
    default:
      break;
    }
    if(Pop == IsPop)
    {
      t_gui_p.IsWarning=1;
    }
  }

  void ManagWarningInfo(void)
  {
    switch(t_gui_p.WarningInfoSelect)
    {
    case MinTempWarning:
      MinTempWarningPopSet=NotPop;
      break;
    case MaxTempWarning:
      MaxTempWarningPopSet=Pop;
      break;
    case MaxTempBedWarning:
      MaxTempBedWarningPopSet=Pop;
      break;
    case HeatFailWarning:
      HeatFailWarningPopSet=Pop;
      break;
    case XMinLimitWarning:
      MachineSizeLimitWarningPopSet=Pop;
      break;
    case YMinLimitWarning:
      MachineSizeLimitWarningPopSet=Pop;
      break;
    case ZMinLimitWarning:
      MachineSizeLimitWarningPopSet=Pop;
      break;
    case XMaxLimitWarning:
      MachineSizeLimitWarningPopSet=Pop;
      break;
    case YMaxLimitWarning:
      MachineSizeLimitWarningPopSet=Pop;
      break;
    case ZMaxLimitWarning:
      MachineSizeLimitWarningPopSet=Pop;
      break;
    case IWDGResetWarning:
      IWDGResetWarningPopSet=NotPop;
      break;
    case FatfsWarning:
      FatfsFailWarningPopSet=Pop;
      break;
    case ThermistorFallsWarning:
      ThermistorFallsWarnPopSet=Pop;
      break;
    default:
      break;
    }
    t_gui_p.IsWarning=0;
  }


#ifdef __cplusplus
} //extern "C" {
#endif

