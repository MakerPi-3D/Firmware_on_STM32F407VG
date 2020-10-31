#include "guicontrol.h"
#include "globalvariables.h"
#include "user_debug.h"
#include <string.h>
#include "temperature.h"
#include "interface.h"
#include "machinecustom.h"
#include "functioncustom.h"
#include "controlxyz.h"
#include "user_interface.h"
#include "stm32f4xx_hal.h"
#include "ConfigurationStore.h"
#include "config_motion_3d.h"
#include "lightcontrol.h"
#include "gcode_global_params.h"

#ifdef __cplusplus
extern "C" {
#endif
  static INT TempHotendToResume = 0;
  static INT TempBedToResume = 0;

//写喷嘴目标温度
  void GUI_WNozzleTargetTemp(int16_t NozzleTemp)
  {
    if(t_gui.target_nozzle_temp != NozzleTemp)//防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
    {
      sg_grbl::temperature_set_extruder_target((float)NozzleTemp, 0);
      t_gui.target_nozzle_temp = NozzleTemp;
    }
  }
  //写热床目标温度
  void GUI_WHotbedTargetTemp(int16_t Hotbedtemp)
  {
    if(t_gui.target_hot_bed_temp != Hotbedtemp)//防止二次设置
    {
      sg_grbl::temperature_set_bed_target((float)Hotbedtemp);
      t_gui.target_hot_bed_temp = Hotbedtemp;
    }
  }

  void PauseToCoolDown(float cooldownFactor)
  {
    TempHotendToResume = (int16_t)sg_grbl::temperature_get_extruder_target(0);
    TempBedToResume = (int16_t)sg_grbl::temperature_get_bed_target();
    GUI_WNozzleTargetTemp((float)TempHotendToResume * cooldownFactor); // float
    if(!t_custom_services.disable_hot_bed)
    {
      GUI_WHotbedTargetTemp((float)TempBedToResume * cooldownFactor); // float
    }
  }

  void PauseToResumeTemp(void)
  {
    GUI_WNozzleTargetTemp(TempHotendToResume);
    if(!t_custom_services.disable_hot_bed)
      GUI_WHotbedTargetTemp(TempBedToResume);
  }

  void PauseToResumeNozTemp(void)
  {
    GUI_WNozzleTargetTemp(TempHotendToResume);
  }

//防止长时间保持高温状态，导致材料融化使挤出头多了一层颜色20170502
  void protect_nozzle(FLOAT hour)
  {
    static UINT32 beginTime = 0;
    if(t_gui.nozzle_temp < 45)
    {
      beginTime = 0;
    }
    if((t_gui.nozzle_temp > 45) && (beginTime == 0))
    {
      beginTime = sys_task_get_tick_count()/1000;
    }

    if(((sys_task_get_tick_count()/1000) - beginTime) > (UINT16)(3600*hour)) // float
    {
      beginTime = 0;
      if(t_gui.nozzle_temp > 45)
      {
        guiControl.coolDown();
      }
    }
  }
#ifdef __cplusplus
} //extern "C" {
#endif

GUIControl::GUIControl()
{

}

extern bool Zaxis_RunOnce;
// 解锁步进电机
void GUIControl::disableSteppers(void)
{
  USER_DbgLog("DisableStep ok!");
  Zaxis_RunOnce = false;
  m84_disable_steppers();
  t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
}

extern float Laser_Print_Position;
// 移动光轴
void GUIControl::moveXYZ()
{
  // 判断是否已经归零
  if (0U == t_gui_p.G28_ENDSTOPS_COMPLETE)
  {
    sys_send_gcode_cmd("G28 isInternal");
  }
  // 限制xyz最小最大值
  for(INT i = 0; i < 3; ++i)
  {
    if (t_gui.move_xyz_pos[i] < motion_3d_model.xyz_min_pos[i])
    {
      t_gui.move_xyz_pos[i] = (INT)motion_3d_model.xyz_min_pos[i];
    }
    if (t_gui.move_xyz_pos[i] > motion_3d_model.xyz_max_pos[i])
    {
      t_gui.move_xyz_pos[i] = (INT)motion_3d_model.xyz_max_pos[i];
    }
  }
  // 发送移动指令
  static CHAR moveXYZ[55];
  memset(moveXYZ,0,sizeof(moveXYZ));
  (void)snprintf(moveXYZ, sizeof(moveXYZ), "G1 F1500 X%f Y%f Z%f isInternal", (FLOAT)t_gui.move_xyz_pos[X_AXIS], (FLOAT)t_gui.move_xyz_pos[Y_AXIS], (FLOAT)t_gui.move_xyz_pos[Z_AXIS]);
  sys_send_gcode_cmd(moveXYZ);
  if(t_sys_data_current.IsLaser)
    Laser_Print_Position = t_gui.move_xyz_pos[Z_AXIS];
}

// 预热abs
void GUIControl::preHeatABS(void)
{
  USER_DbgLog("PreHeatABS ok!");
  GUI_WNozzleTargetTemp(sg_grbl::abs_preheat_hotend_temp);
  if (!t_custom_services.disable_hot_bed)
  {
    GUI_WHotbedTargetTemp(sg_grbl::abs_preheat_hpb_temp);
  }
  gcode::set_fan_speed(0);
}

// 预热pla
void GUIControl::preHeatPLA(void)
{
  USER_DbgLog("PreHeatPLA ok!");
  GUI_WNozzleTargetTemp(sg_grbl::pla_preheat_hotend_temp);
  if (!t_custom_services.disable_hot_bed)
  {
    GUI_WHotbedTargetTemp(sg_grbl::pla_preheat_hpb_temp);
  }
  gcode::set_fan_speed(0);
}

// 预热bed
void GUIControl::preHeatBed(void)
{
  USER_DbgLog("PreHeatBed ok!");
  if (!t_custom_services.disable_hot_bed)
  {
    GUI_WHotbedTargetTemp(100);
  }
  gcode::set_fan_speed(0);
}

void GUIControl::printSetForM14(void)
{
  GUI_WNozzleTargetTemp(SettingInfoToSYS.TargetNozzleTemp);
  gcode::set_fan_speed(SettingInfoToSYS.FanSpeed); // 写风扇速度
  gcode::feed_multiply = SettingInfoToSYS.PrintSpeed; //INT PrintSpeed :100->1 200->2 写打印速度
}

void GUIControl::printSetForNotM14Left(void)
{
  GUI_WNozzleTargetTemp(SettingInfoToSYS.TargetNozzleTemp);
  gcode::feed_multiply = SettingInfoToSYS.PrintSpeed; //INT PrintSpeed :100->1 200->2 写打印速度
}

void GUIControl::printSetForNotM14Right(void)
{
  GUI_WHotbedTargetTemp(SettingInfoToSYS.TargetHotbedTemp);
  gcode::set_fan_speed(SettingInfoToSYS.FanSpeed);
	lightControl.led_switch();
}

void GUIControl::printSetForCavity(void)
{
  if(t_gui.target_cavity_temp_on != SettingInfoToSYS.TargetCavityOnTemp)//防止二次设置
  {
    t_gui.target_cavity_temp_on = SettingInfoToSYS.TargetCavityOnTemp;
  }

  if(t_gui.target_cavity_temp_on)
  {
    if(t_gui.target_cavity_temp != SettingInfoToSYS.TargetCavityTemp)//防止二次设置
    {
      t_gui.target_cavity_temp = SettingInfoToSYS.TargetCavityTemp;
      // 设置温度
      sg_grbl::temperature_set_cavity_target(t_gui.target_cavity_temp);
    }
  }
  else
  {
    if(t_gui.target_cavity_temp != 0)//防止二次设置
    {
      t_gui.target_cavity_temp = 0;
      sg_grbl::temperature_set_cavity_target(t_gui.target_cavity_temp);
    }
  }
}

//冷却
void GUIControl::coolDown(void)
{
  GUI_WNozzleTargetTemp(0);
  GUI_WHotbedTargetTemp(0);
  gcode::set_fan_speed(0);
}

void GUIControl::refreshGuiInfo(void)
{
  // RefMachRunTime
  t_gui.machine_run_time =(sys_task_get_tick_count()/1000);
  if(t_gui.machine_run_time>0X12CC0300)
  {
    t_gui.machine_run_time=0;
  }
  // UpdateGUITemp
  t_gui.nozzle_temp = (INT)sg_grbl::temperature_get_extruder_current(0);
  t_gui.target_nozzle_temp = (INT)sg_grbl::temperature_get_extruder_target(0);
  if(!t_custom_services.disable_hot_bed)
  {
    t_gui.hot_bed_temp = (INT)sg_grbl::temperature_get_bed_current();
    t_gui.target_hot_bed_temp = (INT)sg_grbl::temperature_get_bed_target();
  }

  t_gui.target_cavity_temp = (INT) sg_grbl::temperature_get_cavity_target();
  t_gui.cavity_temp = (INT)sg_grbl::temperature_get_cavity_current();

  // UpdateGUIPrintSpeed
  t_gui.print_speed_value=gcode::feed_multiply;
  // UpdateGUIFanSpeed
  t_gui.fan_speed_value=gcode::fan_speed;
}

GUIControl guiControl;


