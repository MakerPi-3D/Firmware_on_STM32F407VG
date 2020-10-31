#include "midwaychangematerial.h"
#include "globalvariables.h"
#include "user_debug.h"
#include "gcodebufferhandle.h"
#include "controlfunction.h"
#include "autobedlevelinterface.h"
#include "config_motion_3d.h"
#include "user_interface.h"
#include "gcode_global_params.h"
#include "PrintControl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NotStartChangeFilament 0
#define StartChangeFilament 1
#define ChangeFilamentFinish 5
#define HeatNotComplete 0
#define HeatComplete 1


 static INT IsStartChangeFilament=NotStartChangeFilament;


  void RefM600FilamentChangeStatus(void)
  {
    t_gui_p.M600FilamentChangeStatus = gcode::m600_filament_change_status;
  }

  void IsCompleteHeat(void)
  {
    if((1U == t_gui_p.m109_heating_complete))
    {
      t_gui_p.ChangeFilamentHeatStatus=HeatComplete;
    }
    else
    {
      t_gui_p.ChangeFilamentHeatStatus=HeatNotComplete;
    }
  }

  void ChangeFilament(void)
  {
    UINT8 print_status = 0;
    if(IsPrint())
    {
      print_status = 1;
      SetPrintStatus(false);  //解决多次中途换料后，打印乱跑现象
    }
    else if(IsPausePrint())
    {
      print_status = 2;
      SetPausePrintingStatus(false); // 解决暂停打印后中途换料，中途换料时，暂停后续操作同时进行（目标温度下降为目标温度一半），导致进丝异常
    }

    t_gui_p.M600FilamentChangeStatus=gcode::m600_filament_change_status;
    IsStartChangeFilament=StartChangeFilament;
    // 打开中途换料标志位
    gcode::m600_is_midway_change_material = true;
    if(t_gui_p.IsNotHaveMatInPrint)  //断料续打中的换料发送M601
    {
      sys_send_gcode_cmd("M601"); //换料
    }
    else
    {
      if(0 == print_status)
      {
        sys_send_gcode_cmd("M600 S0"); //不是打印状态下换料，不会执行该逻辑
      }
      if(1 == print_status)
      {
        sys_send_gcode_cmd("M600 S1"); //正常打印下换料
      }
      if(2 == print_status)
      {
        sys_send_gcode_cmd("M600 S2");  //暂停打印下换料
      }
    }
  }

  void RefChangeFilamentStatus(void)
  {
    IsCompleteHeat();
    if(StartChangeFilament==IsStartChangeFilament)
    {
      t_gui_p.M600FilamentChangeStatus=gcode::m600_filament_change_status;
      if(ChangeFilamentFinish==t_gui_p.M600FilamentChangeStatus)
      {
        IsStartChangeFilament=NotStartChangeFilament;
        gcode::m600_filament_change_status = false;
        gcode::is_confirm_load_filament = false;
      }
    }
  }

  
#ifdef __cplusplus
} // extern "C" {
#endif




