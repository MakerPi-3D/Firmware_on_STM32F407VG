#include "functioncustom.h"
#include "poweroffrecovery.h"
#include "globalvariables.h"
#include "boardtest.h"
#include "user_debug.h"
#include "guicontrol.h"
#include "filamentcontrol.h"
//#include "stepper_pin.h"
//#include "ConfigurationStore.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "user_interface.h"
#include "config_motion_3d.h"
//static PowerOffOperation powerOffOperation;
//static PowerOffUpDownMinMin powerOffUpDownMinMin;
#include "stm32f4xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "temperature.h"
#include "PrintControl.h"
#include "Alter.h"

  void poweroff_start_cal_z_max_pos(void)
  {
    if(!t_sys_data_current.enable_powerOff_recovery)
    {
      return;
    }
    powerOffOperation.startCalculateZMaxPos();
  }

  void poweroff_stop_cal_z_max_pos(void)
  {
    if(!t_sys_data_current.enable_powerOff_recovery)
    {
      return;
    }
    powerOffOperation.stopGetZMaxPos();
  }

  void poweroff_ready_to_recover_print(void)
  {
    if(!t_sys_data_current.enable_powerOff_recovery)
    {
      return;
    }
    powerOffOperation.readyToRecoveryPrint();
  }

/////////////////////////////////////PowerOff end/////////////////////
  void board_test_display_function(void)
  {
    if(!motion_3d.enable_board_test)
    {
      return;
    }
    if(boardTest.guiInterface())
    {
      return ;
    }
  }

  void board_test_model_select(void)
  {
    if(!motion_3d.enable_board_test)
    {
      return;
    }
    boardTest.modelSelect();
  }

  void board_test_cal_heat_time_gui(void)
  {
    if(!motion_3d.enable_board_test)
    {
      return;
    }
    if(boardTest.calHeatTimeGui())
    {
      return ;
    }
  }

  void board_test_cal_touch_count(void)
  {
//  if(!motion_3d.enable_board_test)
//    return;
    if(boardTest.ERRTouchCount())
    {
      return ;
    }
  }
  void board_test_pressure(void)
  {
//  if(!motion_3d.enable_board_test)
//    return;
    boardTest.PressureTest();
    return ;
  }

#ifdef __cplusplus
} //extern "C" {
#endif

