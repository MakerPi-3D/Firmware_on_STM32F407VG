#include "autobedleveling.h"
#include "infrared_auto_bed_level.h"
#include <math.h>
#include "globalvariables.h"
#include "machinecustom.h"
#include "user_debug.h"
#include "autobedlevelinterface.h"
#include "gcodebufferhandle.h"
#include "threed_engine.h"
#include "interface.h"
#include "user_interface.h"
#include "ConfigurationStore.h"
#include "config_model_tables.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "ff.h"
#include "view_commonf.h"
#include "gcode_global_params.h"
#ifdef ENABLE_AUTO_BED_LEVELING
#ifdef __cplusplus
extern "C" {
#endif

#include "planner.h"
#include "stepper.h"
#include "temperature.h"
#include "view_common.h"

#define POS1_X 0
#define POS1_Y 0
#define POS3_X 106
#define POS3_Y 26
#define POS2_X 24
#define POS2_Y 190
  // 平台触点信息
  FLOAT left_probe_bed_position;                                                                                       ///< 探头接触热床左边位置
  FLOAT right_probe_bed_position;                                                                                      ///< 探头接触热床右边位置
  FLOAT back_probe_bed_position;                                                                                       ///< 探头接触热床后面位置
  FLOAT front_probe_bed_position;                                                                                      ///< 探头接触热床前面位置
  FLOAT middle_probe_bed_position;
  FLOAT Limit_Z_Off;

  void auto_bed_level_adjust_init(void)
  {
    if(1 == t_sys_data_current.enable_bed_level)
    {
      autoBedLevelingAdjust.setModelBedPlatform(t_sys_data_current.model_id);
      sg_grbl::plan_set_bed_level_equation();
      motion_3d.is_cal_bed_platform = true;
    }
    if(K5 == t_sys_data_current.model_id)
    {
      if(1 == t_sys_data_current.enable_color_mixing)
      {
//			Limit_Z_Off = 2.4f;              //2019/06/19的固件差值为2.4f
        Limit_Z_Off = 0.15f;               //其它时候的固件差值为0.15f
      }
      else
      {
//			Limit_Z_Off = 5.7f;       //2018/12/25之前的固件差值为5.7f
        Limit_Z_Off = 1.15f;              //2018/12/25之后的固件差值为1.15f
      }
    }
  }

  void start_calculate_bed_level(void)
  {
    autoBedLevelingAdjust.startCalculateBedLevel();
  }

  void get_bed_level_position_z_at_lf(void)
  {
    autoBedLevelingAdjust.get_bed_level_position_z_at_lf();
  }

  void get_bed_level_position_z_at_lb(void)
  {
    autoBedLevelingAdjust.get_bed_level_position_z_at_lb();
  }

  void get_bed_level_position_z_at_middle(void)
  {
    autoBedLevelingAdjust.get_bed_level_position_z_at_middle();
  }

  void get_bed_level_position_z_at_rb(void)
  {
    autoBedLevelingAdjust.get_bed_level_position_z_at_rb();
  }

  void get_bed_level_position_z_at_rf(void)
  {
    autoBedLevelingAdjust.get_bed_level_position_z_at_rf();
  }

  void calculate_bed_level_finish(void)
  {
    autoBedLevelingAdjust.calculateBedLevelFinish();
  }

  bool is_cal_bed_platform(void)
  {
    if(1 == t_sys_data_current.enable_bed_level)
    {
      if(!motion_3d.is_cal_bed_platform)
      {
        return false;
      }
    }
    return true;
  }

//  void bed_level_change_z_min_endstop(bool &z_min_endstop)
//  {
////      if(1 == t_sys_data_current.enable_bed_level && !autoBedLevelingAdjust.isCalBedPlatform)
////      {//舵机校准时，使用PA6作为限位开关检测引脚，因热敏电阻会干扰，这里先注释20170608
////        z_min_endstop = !HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6);
////      }
//    if(!autoBedLevelingAdjust.isCalBedPlatform)//调平台时探针的状态可以代替限位
//    {
//      z_min_endstop |= !HAL_GPIO_ReadPin(E1_STEP_PIN_GPIO,E1_STEP_PIN);//读取探针的状态
//    }
//  }


  static UINT8 IsAutoBedLevelZero=0; //是否是自动调平归零界面
  void SetAutoBedLevelZeroNozzleTemp(void)
  {
    IsAutoBedLevelZero=1;
    if(t_custom_services.disable_hot_bed)  //M14
    {
      PrintSet_M14();
    }
    else
    {
      PrintSet_NotM14_Left();
    }
    IsAutoBedLevelZero=0;
  }

  bool is_auto_bed_level_zero(void)
  {
    if(1 == t_sys_data_current.enable_bed_level)
    {
      return IsAutoBedLevelZero;
    }
    else
    {
      return false;
    }
  }

//  void set_process_auto_bed_level_status(bool status)
//  {
//    if(1 == t_sys_data_current.enable_bed_level)
//      autoBedLeveling.hasProcessAutoBedLeveling(status);
//  }
  static FLOAT target_temperature2 = 0.0F;

  void gcodeCMD_g28_bed_level_start(INT tmp_extruder)
  {
    if(!motion_3d.updown_g28_first_time)
    {
      if(1 == t_sys_data_current.enable_bed_level)
      {
        sg_grbl::st_enable_endstops(true);
        sg_grbl::plan_set_process_auto_bed_level_status(false);
        target_temperature2 = sg_grbl::temperature_get_extruder_target(tmp_extruder);
        if((sg_grbl::temperature_get_extruder_current(tmp_extruder) < AUTO_BED_LEVELING_EXTRUDE_MINTEMP) || (target_temperature2 < AUTO_BED_LEVELING_EXTRUDE_MINTEMP))
        {
          autoBedLeveling.Gcode_M109(AUTO_BED_LEVELING_EXTRUDE_HEATTEMP);
        }
      }
    }
  }

  void gcodeCMD_g28_bed_level_end(void)
  {
    if(1 == t_sys_data_current.enable_bed_level)
    {
      if(!motion_3d.is_cal_bed_platform)
      {
        autoBedLeveling.setActiveExtruder(gcode::active_extruder);
        autoBedLeveling.Gcode_G29();
      }
      sg_grbl::plan_set_process_auto_bed_level_status(true);
      autoBedLeveling.Gcode_M109(target_temperature2);
      //st_enable_endstops(false);
    }
  }

  void gcodeCMD_g28_interface(UINT8 active_extruder)
  {
    autoBedLeveling.setActiveExtruder(active_extruder);
    autoBedLeveling.Gcode_G28();
  }

  void gcodeCMD_m602_bed_level_interface(UINT8 active_extruder)
  {
    autoBedLeveling.setActiveExtruder(active_extruder);
    autoBedLevelingAdjust.PrepareCalculateBedLevel();
  }

//  void get_coordinates_set_destination(INT axis, bool axis_code_seen, FLOAT axis_code_value, bool is_relative_mode)
//  {
//    autoBedLeveling.get_coordinates_set_destination(axis, axis_code_seen, axis_code_value, is_relative_mode);
//  }

  extern volatile FLOAT axis_steps_per_unit[MAX_NUM_AXIS];

  extern void process_command_G28(void);

  extern void SetAutoBedLevelZeroNozzleTemp(void);
  void gui_bed_level_nozzle_heat(void)
  {
    CHAR buffer[20];
    if(gui_is_refresh())
    {
      display_picture(82);

      SetTextDisplayRange(244,49,12*3,24,&NozzleTempTextRange);
      ReadTextDisplayRangeInfo(NozzleTempTextRange,NozzleTempTextRangeBuf);
      //显示喷嘴温度
      snprintf(buffer, sizeof(buffer), "%3d",(INT)t_gui.nozzle_temp);
      CopyTextDisplayRangeInfo(NozzleTempTextRange,NozzleTempTextRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((UINT8*)buffer, NozzleTempTextRange,TextRangeBuf);
      //显示喷嘴目标温度
      snprintf(buffer, sizeof(buffer), "/%3d",(INT)t_gui.target_nozzle_temp);
      DisplayTextDefault ((UINT8*)buffer,244+(12*3),49);
    }

    if(touchxy(165,35,310,85))
    {
      gui_set_curr_display(SetAutoBedLevelZeroNozzleTemp);
    }

    if(gui_is_rtc())
    {
      if(autoBedLeveling.isNozzleHeatFinish )
      {
        gui_set_curr_display(gui_bed_level_auto_home);
        autoBedLeveling.isNozzleHeatFinish = false;
        return;
      }
      //显示喷嘴温度
      snprintf(buffer, sizeof(buffer), "%3d",(INT)t_gui.nozzle_temp);
      CopyTextDisplayRangeInfo(NozzleTempTextRange,NozzleTempTextRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((UINT8*)buffer, NozzleTempTextRange,TextRangeBuf);
      return;
    }
  }

  bool isAutoHome = false;
  void gui_bed_level_auto_home(void)
  {
    if(gui_is_refresh())
    {
      display_picture(83);
    }

    if(isAutoHome)
    {
      respond_gui_send_sem(CalBedLevelFinish);
      isAutoHome = false;
    }

    if(fabs(sg_grbl::st_get_position_length(Z_AXIS)) <  0.1F )
    {
      sys_send_gcode_cmd("G1 Z20 I0 H0 isInternal");
    }

    if(sg_grbl::st_get_position_length(Z_AXIS) == 20.0F)
    {
      if(1 == autoBedLevelingAdjust.bedLevelStatus)
      {
        gui_set_curr_display(gui_bed_level_cal_z_at_left_front);
      }
      if(7 == autoBedLevelingAdjust.bedLevelStatus)
      {
        autoBedLevelingAdjust.bedLevelStatus = 0U;
        gui_set_curr_display(maindisplayF);
      }
    }
  }

#if 0
  void gui_bed_level_auto_home_finish(void)
  {
    if(gui_is_refresh())
    {
      display_picture(84);
    }

    if(touchxy(190,228,290,276))
    {
      if(autoBedLevelingAdjust.bedLevelStatus == 0)
      {
        gui_set_curr_display(maindisplayF);
      }
      else
      {
        gui_set_curr_display(gui_bed_level_cal_z_at_left_front);
      }
    }
  }
#endif

// 正在校准判断标志
  bool isCalBedPlatformZ = false;
  bool isCalBedPlatformZFinish = false;
  void gui_bed_level_cal_z(void)
  {
    if(gui_is_refresh())
    {
      display_picture(91);
    }

    if(sg_grbl::st_get_position_length(Z_AXIS) == gcode::get_current_position(Z_AXIS))
    {
      isCalBedPlatformZFinish = true;
    }

    if(isCalBedPlatformZ)
    {
      if(autoBedLevelingAdjust.bedLevelStatus == 2)
      {
        respond_gui_send_sem(CalBedLevelZAtLF);
      }
      else if(autoBedLevelingAdjust.bedLevelStatus == 3)
      {
        respond_gui_send_sem(CalBedLevelZAtLB);
      }
      else if(autoBedLevelingAdjust.bedLevelStatus == 4)
      {
        respond_gui_send_sem(CalBedLevelZAtMiddle);
      }
      else if(autoBedLevelingAdjust.bedLevelStatus == 5)
      {
        respond_gui_send_sem(CalBedLevelZAtRB);
      }
      else if(autoBedLevelingAdjust.bedLevelStatus == 6)
      {
        respond_gui_send_sem(CalBedLevelZAtRF);
      }
      isCalBedPlatformZ = false;
    }
    else
    {
      if(isCalBedPlatformZFinish)
      {
        if(autoBedLevelingAdjust.bedLevelStatus == 2)
        {
          gui_set_curr_display(gui_bed_level_cal_z_at_left_back);
        }
#if 0
//			else if(autoBedLevelingAdjust.bedLevelStatus == 3 )
//			{
//				gui_set_curr_display(gui_bed_level_cal_z_at_middle);
//			}
//			else if(autoBedLevelingAdjust.bedLevelStatus == 4)
//			{
//				gui_set_curr_display(gui_bed_level_cal_z_at_right_back);
//			}
#endif
        else if(autoBedLevelingAdjust.bedLevelStatus == 3 )//原来是5，改为3才能正常运行
        {
          gui_set_curr_display(gui_bed_level_cal_z_at_right_front);
        }
        else if(autoBedLevelingAdjust.bedLevelStatus == 6 )//原来是6，右前点改为中间点后要改为4。2017/6/1
        {
          gui_set_curr_display(gui_bed_level_cal_z_finish);
        }
        isCalBedPlatformZFinish = false;
      }
    }
  }

  void gui_bed_level_cal_z_finish(void)
  {
    if(gui_is_refresh())
    {
      display_picture(90);
    }

    if(touchxy(84,215,184,264))
    {
      gui_set_curr_display(gui_bed_level_auto_home);
      autoBedLevelingAdjust.bedLevelStatus = 7;
      isAutoHome = true;
    }
  }

  void gui_bed_level_cal_z_at_left_front(void)
  {
    if(gui_is_refresh())
    {
      display_picture(85);
    }

    if(touchxy(84,215,184,264))
    {
      gui_set_curr_display(gui_bed_level_cal_z);
      isCalBedPlatformZ = true;
      isCalBedPlatformZFinish = false;
      autoBedLevelingAdjust.bedLevelStatus = 2;
      return ;
    }
  }

  void gui_bed_level_cal_z_at_left_back(void)
  {
    if(gui_is_refresh())
    {
      display_picture(86);
    }

    if(touchxy(84,215,184,264))
    {
      gui_set_curr_display(gui_bed_level_cal_z);
      isCalBedPlatformZ = true;
      isCalBedPlatformZFinish = false;
      autoBedLevelingAdjust.bedLevelStatus = 3;
      return ;
    }
  }

#if 0
  void gui_bed_level_cal_z_at_middle(void)
  {
    if(gui_is_refresh())
    {
      display_picture(87);
    }

    if(touchxy(84,215,184,264))
    {
      gui_set_curr_display(gui_bed_level_cal_z);
      isCalBedPlatformZ = true;
      isCalBedPlatformZFinish = false;
      autoBedLevelingAdjust.bedLevelStatus = 4;
      return ;
    }
  }

  void gui_bed_level_cal_z_at_right_back(void)
  {
    if(gui_is_refresh())
    {
      display_picture(88);
    }

    if(touchxy(84,215,184,264))
    {
      gui_set_curr_display(gui_bed_level_cal_z);
      isCalBedPlatformZ = true;
      isCalBedPlatformZFinish = false;
      autoBedLevelingAdjust.bedLevelStatus = 5;
      return ;
    }
  }

#endif

  void gui_bed_level_cal_z_at_right_front(void)
  {
    if(gui_is_refresh())
    {
      display_picture(89);
    }

    if(touchxy(84,215,184,264))
    {
      gui_set_curr_display(gui_bed_level_cal_z);
      isCalBedPlatformZ = true;
      isCalBedPlatformZFinish = false;
      autoBedLevelingAdjust.bedLevelStatus = 6;
      return ;
    }
  }

#ifdef __cplusplus
}
#endif

// 问题：
// 1、平台XY与机器Z向量夹角小于90度，会出现Z越大，XY零点矩阵会出现负数，机器无法到达该位置，导致打印异常
// 2、平台XY与机器Z向量夹角大于90度，会出现Z越大，XY零点矩阵会出现大于行程，机器无法到达该位置，导致打印异常
// 问题1解决方案：设置XY零点偏移位置用于容错，该偏移量可在平台校正时候计算出来，可能会导致实际打印面积变小

AutoBedLeveling::AutoBedLeveling()
{
//  OK_KEY = false;
  isSerial = false;

  feedrate = 1500.0F;
  active_extruder = 0;

  sg_grbl::plan_bed_level_matrix_set_identity();
  for(INT i = 0; i < MAX_NUM_AXIS; ++i)
    plan_position[i] = 0.0F;

//  x_probe_offset_from_extruder = 0.0F;
//  y_probe_offset_from_extruder = 0.0F;
//  z_probe_offset_from_extruder = Z_PROBE_OFFSET_FROM_EXTRUDER;
////  z_at_xLeft_yFront = 0.0F;
////  z_at_xRight_yFront = 0.0F;
////  z_at_xLeft_yBack = 0.0F;
//  z_raise_before_probing = Z_RAISE_BEFORE_PROBING;
//  z_raise_between_probings = Z_RAISE_BETWEEN_PROBINGS;
//  z_movedown_until_find_bed = Z_MOVEDOWN_UNTIL_FIND_BED;
//  xy_travel_speed = XY_TRAVEL_SPEED;

//  left_probe_bed_position = 0.0F;
//  right_probe_bed_position = 0.0F;
//  front_probe_bed_position = 0.0F;
//  back_probe_bed_position = 0.0F;
//  middle_probe_bed_position = 0.0F;

  for(INT i = 0; i < XYZ_NUM_AXIS; ++i)
  {
//    current_save_xyz[i] = 0.0F;
//    destination_save_xyz[i] = 0.0F;
//    current_xyz_compensate[i] = 0.0F;
//    current_xyz_real[i] = 0.0F;
  }
  isNozzleHeatFinish = false;
//  bedLevelStatus = 0;
//  targetTemperature = 0.0F;
}
AutoBedLeveling::~AutoBedLeveling(void)
{

}

void AutoBedLeveling::Gcode_M104(FLOAT value)
{
  sg_grbl::temperature_set_extruder_target(value, active_extruder); // 设置喷嘴目标温度
  t_gui.target_nozzle_temp = value;
}

void AutoBedLeveling::Gcode_M109(FLOAT value)
{
//  isNozzleHeatFinish = false;
//  setTargetHotend(value, active_extruder); // 设置喷嘴目标温度
//  t_gui.target_nozzle_temp = value;

//	//由于Gcode_M109命令只在需要归零时才调用，所以每次归零需要加热，加热时显示加热状态。
//	menufunc_t lastdisplay = currentdisplay;
//	if(currentdisplay != gui_bed_level_nozzle_heat)
//	{
//    gui_set_curr_display(gui_bed_level_nozzle_heat);
//	}
//  // 等待加热完成
//  /* See if we are heating up or cooling down */
////  bool target_direction = isHeatingHotend(active_extruder); // true if heating, false if cooling
//  ULONG codenum = sys_task_get_tick_count();
//  while(degHotend(active_extruder) < t_gui.target_nozzle_temp-15)//加热到离目标温度15度时终止等待
//  {
//    if((sys_task_get_tick_count() - codenum) > 1000UL )
//    {
//      //Print Temp Reading and remaining time every 1 second while heating up/cooling down
//      codenum = sys_task_get_tick_count();
//    }
//    (void)sys_os_delay(1);
////    target_direction = (isHeatingHotend(active_extruder));
//  }
//	//对应上面，取消显示加热状态界面
//	if(lastdisplay != gui_bed_level_nozzle_heat)
//	{
//    gui_set_curr_display(lastdisplay);
//	}
  isNozzleHeatFinish = true;
}

void AutoBedLeveling::do_blocking_move_relative(FLOAT offset_x, FLOAT offset_y, FLOAT offset_z)
{
  autoBedLevelingAdjust.do_blocking_move_to(gcode::get_current_position(X_AXIS) + offset_x, gcode::get_current_position(Y_AXIS) + offset_y, gcode::get_current_position(Z_AXIS) + offset_z); // 当前位置偏移XYZ
}

void AutoBedLeveling::get_bed_level_position_z(void)
{
  autoBedLevelingAdjust.get_bed_level_position_z_at_lf();
  autoBedLevelingAdjust.get_bed_level_position_z_at_lb();
//  get_bed_level_position_z_at_middle();
//  get_bed_level_position_z_at_rb();
  autoBedLevelingAdjust.get_bed_level_position_z_at_rf();
  SaveBedLevelZValue();
}

void AutoBedLeveling::do_blocking_move_away_from_zero(void)
{
  // 平台触碰到限位开关，Z偏移20mm，XY偏移5mm
  for(INT i = XYZ_NUM_AXIS - 1; i > 0; i--)
  {
    if(sg_grbl::st_is_min_endstop(i))
    {
      sg_grbl::st_enable_endstops(false);
      feedrate = gcode::homing_feedrate[i];
      if(i == Z_AXIS)
      {
        gcode::set_current_position(i, gcode::get_current_position(i) + 20);
      }
      else
      {
        gcode::set_current_position(i, gcode::get_current_position(i) + 5);
      }
      gcode::process_buffer_line_normal_4_dest(feedrate/60); // float
      gcode::set_current_position(i, gcode::get_destination_position(i));
      sg_grbl::st_enable_endstops(true);
    }
  }
  gcode::plan_set_current_position();
}

void AutoBedLeveling::do_blocking_move_to_zero(void)
{
  autoBedLevelingAdjust.setup_for_endstop_move();

  for(INT i = 0; i < motion_3d.axis_num; ++i)
    gcode::set_destination_position(i, gcode::get_current_position(i));

  do_blocking_move_away_from_zero();

  // 平台重新移动到零点
  for(INT i = 0; i < XYZ_NUM_AXIS; ++i)
  {
    if(i < 3)
      gcode::set_destination_position(i, -motion_3d_model.xyz_max_pos[i]);

    feedrate = gcode::homing_feedrate[i];
    gcode::process_buffer_line_normal_4_dest(feedrate/60); // float
    gcode::set_destination_position(i, 0.0f);
    gcode::set_current_position(i, gcode::get_destination_position(i));
    gcode::plan_set_current_position();
  }

  autoBedLevelingAdjust.clean_up_after_endstop_move();
}

void AutoBedLeveling::Gcode_G28_move_to_xy_zero(void)
{
  FLOAT current_xyz_pos[XYZ_NUM_AXIS] = {0.0F, 0.0F, 0.0F};
  current_xyz_pos[Z_AXIS] = sg_grbl::plan_get_current_save_xyz(Z_AXIS); // 设置保存的当前XYZ位置
  sg_grbl::plan_apply_rotation_xyz(current_xyz_pos[X_AXIS], current_xyz_pos[Y_AXIS], current_xyz_pos[Z_AXIS]); // 矩阵变换
  current_xyz_pos[X_AXIS] = -motion_3d_model.xyz_max_pos[X_AXIS]; // 往X零点移动，确保撞击限位开关
  current_xyz_pos[Y_AXIS] = 5;
  current_xyz_pos[Z_AXIS] += 10; // Z轴补偿10mm，避免平台太倾斜喷嘴撞击平台

  // 避免高度高于z最大行程，导致撞zmax限位
  if(current_xyz_pos[Z_AXIS] >= motion_3d_model.xyz_move_max_pos[Z_AXIS])
  {
    current_xyz_pos[Z_AXIS] = motion_3d_model.xyz_move_max_pos[Z_AXIS];
  }
  // 获取XY轴归零速度
  feedrate = gcode::homing_feedrate[X_AXIS];
  if(gcode::homing_feedrate[Y_AXIS]<feedrate)
  {
    feedrate = gcode::homing_feedrate[Y_AXIS];
  }

  // 移动位置，设置X位置为零
  for(int8_t i=0; i < XYZ_NUM_AXIS; ++i)
    gcode::set_destination_position(i, current_xyz_pos[i]);
  gcode::process_buffer_line_normal_4_dest(feedrate/60); // float
  gcode::set_destination_position(X_AXIS, 0.0f);
  gcode::plan_set_destination_position();

  // 移动位置，设置Y位置为零
  gcode::set_destination_position(Y_AXIS, -motion_3d_model.xyz_max_pos[Y_AXIS]);// 往Y零点移动，确保撞击限位开关
  gcode::process_buffer_line_normal_4_dest(feedrate/60);
  gcode::set_destination_position(Y_AXIS, 0.0f);
  gcode::plan_set_destination_position();

  // 设置当前位置current position
  for(int8_t i = 0; i < XYZ_NUM_AXIS; ++i)
    gcode::set_current_position(i, gcode::get_destination_position(i));

  // 设置保存XY位置
  for(int8_t i = 0; i < (XYZ_NUM_AXIS - 1); ++i)
  {
    sg_grbl::plan_set_current_save_xyz(i, 0.0F);
//    destination_save_xyz[i] = 0.0F;
  }
}

void AutoBedLeveling::Gcode_G28(void)
{
  bool code_seen_xyz[3];
  for(INT i = 0; i < 3; ++i)
  {
    code_seen_xyz[i] = gcode::parseGcodeBufHandle.codeSeen(gcode::axis_codes[i]);
  }


  bool home_all_axis = !(code_seen_xyz[X_AXIS] || code_seen_xyz[Y_AXIS] || code_seen_xyz[Z_AXIS]);
  autoBedLevelingAdjust.setup_for_endstop_move();

  for(int8_t i=0; i < motion_3d.axis_num; ++i)
    gcode::set_destination_position(i, gcode::get_current_position(i));

  do_blocking_move_away_from_zero(); // 离开零点

  if((home_all_axis)||( code_seen_xyz[X_AXIS] || code_seen_xyz[Y_AXIS]) )  //first diagonal move
  {
    Gcode_G28_move_to_xy_zero(); // XY归零
  }

  if((home_all_axis) || (code_seen_xyz[Z_AXIS]))
  {
    gcode::set_destination_position(Z_AXIS, 0.0f);
    if((!sg_grbl::st_is_min_endstop(X_AXIS)) || (!sg_grbl::st_is_min_endstop(Y_AXIS))) //执行Z归零前，确保xy方向归零
    {
      Gcode_G28_move_to_xy_zero();
    }

    // 判断Z是否在零点
    if(!sg_grbl::st_is_min_endstop(Z_AXIS))
    {
      gcode::set_destination_position(Z_AXIS, -motion_3d_model.xyz_max_pos[Z_AXIS]);
      feedrate = gcode::homing_feedrate[Z_AXIS];
      gcode::process_buffer_line_normal_4_dest(feedrate/60); // float
      gcode::set_destination_position(Z_AXIS, 0.0f);
    }
    gcode::set_current_position(Z_AXIS, gcode::get_destination_position(Z_AXIS));
    gcode::plan_set_current_position();

    // 重置current_save_xyz、destination_save_xyz，避免出现异常
    for(INT i = 0; i < XYZ_NUM_AXIS; ++i)
    {
      sg_grbl::plan_set_current_save_xyz(i, 0.0F);
//      destination_save_xyz[i] = 0.0F;
    }
  }
  autoBedLevelingAdjust.clean_up_after_endstop_move();
}

void AutoBedLeveling::Gcode_G29(void)
{
  if(!gpio_is_zmin_pin_exit())
  {
    return ;
  }
  sg_grbl::st_synchronize();

  // 判断xyz是否已经触碰限位
  if(sg_grbl::st_is_xyz_min_endstops_hit())
  {
    return ;
  }

  // 获取当前位置
  sg_grbl::plan_bed_level_matrix_set_identity();
  gcode::set_current_position(X_AXIS, sg_grbl::st_get_position_length(X_AXIS));
  gcode::set_current_position(Y_AXIS, sg_grbl::st_get_position_length(Y_AXIS));
  gcode::set_current_position(Z_AXIS, sg_grbl::st_get_position_length(Z_AXIS));
  gcode::plan_set_current_position();

  // 获取平台触点z高度
  get_bed_level_position_z();
  // 设置平台矩阵
  sg_grbl::plan_set_bed_level_equation();
  motion_3d.is_cal_bed_platform = true;
  // 修正当前位置
  autoBedLevelingAdjust.correcting_position();

  // 回到零点
  do_blocking_move_to_zero();

  sg_grbl::st_clear_xyz_min_endstops_hit();
  return ;
}

void AutoBedLeveling::Gcode_G30(void)
{
  sg_grbl::st_synchronize();
  // TODO:
  // make sure the bed_level_rotation_matrix is identity or the planner will get set incorectly
  autoBedLevelingAdjust.setup_for_endstop_move();
  feedrate = gcode::homing_feedrate[Z_AXIS];
  autoBedLevelingAdjust.run_z_probe();
  autoBedLevelingAdjust.clean_up_after_endstop_move();
}

void AutoBedLeveling::copy_curr_position(void)
{
  for(INT i=0; i<MAX_NUM_AXIS; ++i)
    plan_position[i]=gcode::get_current_position(i);
}

void AutoBedLeveling::mark_position(CONST FLOAT &pos,CONST INT &index)
{
  plan_position[index]=pos;
}

void AutoBedLeveling::setActiveExtruder(UINT8 value)
{
  active_extruder = value;
}

AutoBedLeveling autoBedLeveling;
/*********************************************************************************************************************************************/
/*																			@class AutoBedLevelingAdjust
 *@自动调平校准平台时调用的功能
*/
/********************************************************************************************************************************************/
/*
 *@class: AutoBedLevelingAdjust
 *@function:AutoBedLevelingAdjust
 *@brief: 构造函数，对类成员初始化
 *@data: 2017/05/22
*/
AutoBedLevelingAdjust::AutoBedLevelingAdjust(void)
{
  bedLevelStatus = 0;
  motion_3d.is_cal_bed_platform = true;
//	anglenum = 5;

  x_probe_offset_from_extruder = 0.0F;
  y_probe_offset_from_extruder = 0.0F;
  z_probe_offset_from_extruder = Z_PROBE_OFFSET_FROM_EXTRUDER;
//  z_at_xLeft_yFront = 0.0F;
//  z_at_xRight_yFront = 0.0F;
//  z_at_xLeft_yBack = 0.0F;
  z_raise_before_probing = Z_RAISE_BEFORE_PROBING;
  z_raise_between_probings = Z_RAISE_BETWEEN_PROBINGS;
  z_movedown_until_find_bed = Z_MOVEDOWN_UNTIL_FIND_BED;
  xy_travel_speed = XY_TRAVEL_SPEED;

  left_probe_bed_position = 0.0F;
  right_probe_bed_position = 0.0F;
  front_probe_bed_position = 0.0F;
  back_probe_bed_position = 0.0F;
  middle_probe_bed_position = 0.0F;

  targetTemperature = 0.0F;
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:~AutoBedLevelingAdjust
 *@brief: 析构函数，在类成员使用后做清理工作，如释放new申请的内存等等
 *@data: 2017/05/22
*/
AutoBedLevelingAdjust::~AutoBedLevelingAdjust(void)
{

}
/*
 *@class: AutoBedLevelingAdjust
 *@function:setModelBedPlatform
 *@brief: 初始化平台触点的位置
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::setModelBedPlatform(INT modelID)
{
  if((M2030HY == modelID) || (M2030 == modelID))
  {
    left_probe_bed_position = 14.0F;
    right_probe_bed_position = 194.0F;//原来是194.0；201763测试舵机改为144
    front_probe_bed_position = 24.0F;//原来是14.0；20170603测试舵机时改为24.0
    back_probe_bed_position = 190.0F;
    middle_probe_bed_position = 104.0F;
  }
  else if((M3145S == modelID) || (M3145T == modelID))
  {
    left_probe_bed_position = 20.0F;
    right_probe_bed_position = 260.0F;
    front_probe_bed_position = 1.0F;
    back_probe_bed_position = 210.0F;
    middle_probe_bed_position = 150.0F;
  }

  sg_grbl::plan_set_bed_level_position(left_probe_bed_position, right_probe_bed_position, front_probe_bed_position, back_probe_bed_position, middle_probe_bed_position);
  infrared_auto_bed_init_position(left_probe_bed_position, right_probe_bed_position, front_probe_bed_position, back_probe_bed_position, middle_probe_bed_position);
}

/*
 *@class: AutoBedLevelingAdjust
 *@function:startCalculateBedLevel()
 *@brief: prepare to start adjust bedlevel
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::startCalculateBedLevel(void)
{
  sys_send_gcode_cmd("G28 isInternal");
  sys_send_gcode_cmd("M602 isInternal");
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:PrepareCalculateBedLevel()
*@brief: 为开始计算平台矩阵做准备工作，如：开限位，加热，标志校准进行状态值为1
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::PrepareCalculateBedLevel(void)
{
  sg_grbl::st_enable_endstops(true);
  sg_grbl::st_check_endstop_z_hit_min();
  sg_grbl::plan_set_process_auto_bed_level_status(false);
  targetTemperature = sg_grbl::temperature_get_extruder_target(gcode::active_extruder);
  if((sg_grbl::temperature_get_extruder_current(gcode::active_extruder) < AUTO_BED_LEVELING_EXTRUDE_MINTEMP) || (targetTemperature < AUTO_BED_LEVELING_EXTRUDE_MINTEMP))
  {
    autoBedLeveling.Gcode_M109(AUTO_BED_LEVELING_EXTRUDE_HEATTEMP);
  }
  else
  {
    autoBedLeveling.isNozzleHeatFinish = true;
  }
  bedLevelStatus = 1;
  motion_3d.is_cal_bed_platform =false;
//  isAutoHomeFinish = false;
//  process_command_G28();
//  isAutoHomeFinish = true;
//  Gcode_G29();
//  hasProcessAutoBedLeveling(true);
//  Gcode_M109(target_temperature1);
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:get_bed_level_position_z_at_lf
 *@brief: 获取平台在左前方碰喷嘴的坐标
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::get_bed_level_position_z_at_lf(void)
{
  // 打开限位开关触发
  setup_for_endstop_move();

  float tmp = t_sys_data_current.bed_level_z_at_left_front;
  do_blocking_move_to_xy_getZ(left_probe_bed_position, front_probe_bed_position, tmp);

  // 关闭限位开关触发
  clean_up_after_endstop_move();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:get_bed_level_position_z_at_lb
 *@brief: 获取平台在左后方碰喷嘴的坐标
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::get_bed_level_position_z_at_lb(void)
{
  // 打开限位开关触发
  setup_for_endstop_move();

  float tmp = t_sys_data_current.bed_level_z_at_left_back;
  do_blocking_move_to_xy_getZ(left_probe_bed_position, back_probe_bed_position,  tmp);

  // 关闭限位开关触发
  clean_up_after_endstop_move();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:get_bed_level_position_z_at_middle
 *@brief: 获取平台在中间碰喷嘴的坐标
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::get_bed_level_position_z_at_middle(void)
{
  // 打开限位开关触发
  setup_for_endstop_move();

  // the current position will be updated by the blocking move so the head will not lower on this next call.
  float tmp = t_sys_data_current.bed_level_z_at_middle;
  do_blocking_move_to_xy_getZ(middle_probe_bed_position, middle_probe_bed_position,  tmp);

  // 关闭限位开关触发
  clean_up_after_endstop_move();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:get_bed_level_position_z_at_rb
 *@brief: 获取平台在右后方碰喷嘴的坐标
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::get_bed_level_position_z_at_rb(void)
{
  // 打开限位开关触发
  setup_for_endstop_move();

  // the current position will be updated by the blocking move so the head will not lower on this next call.
  float tmp = t_sys_data_current.bed_level_z_at_right_back;
  do_blocking_move_to_xy_getZ(right_probe_bed_position, back_probe_bed_position, tmp);

  // 关闭限位开关触发
  clean_up_after_endstop_move();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:get_bed_level_position_z_at_rf
 *@brief: 获取平台在右前方碰喷嘴的坐标
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::get_bed_level_position_z_at_rf(void)
{
  // 打开限位开关触发
  setup_for_endstop_move();

  // the current position will be updated by the blocking move so the head will not lower on this next call.
  float tmp = t_sys_data_current.bed_level_z_at_right_front;
  do_blocking_move_to_xy_getZ(right_probe_bed_position, front_probe_bed_position, tmp);

  // 关闭限位开关触发
  clean_up_after_endstop_move();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:correcting_position
 *@brief: see function
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::correcting_position(void)
{
  //plan_bed_level_matrix = plan_bed_level_matrix.transpose(plan_bed_level_matrix);
  gcode::set_current_position(X_AXIS, sg_grbl::st_get_position_length(X_AXIS));
  gcode::set_current_position(Y_AXIS, sg_grbl::st_get_position_length(Y_AXIS));
  gcode::set_current_position(Z_AXIS, sg_grbl::st_get_position_length(Z_AXIS));

  // but the bed at 0 so we don't go below it.
  gcode::set_current_position(Z_AXIS, gcode::get_current_position(Z_AXIS) - z_probe_offset_from_extruder);
  gcode::plan_set_current_position();
  sg_grbl::st_synchronize();

#ifdef ENABLE_AUTO_BED_LEVELING_DEBUG
  USER_DbgLog("c_pos_x = %f; c_pos_y = %f; c_pos_z = %f",gcode::get_current_position(X_AXIS),
              gcode::get_current_position(Y_AXIS), gcode::get_current_position(Z_AXIS));
#endif //ENABLE_AUTO_BED_LEVELING_DEBUG

  // The following code correct the Z height difference from z-probe position and hotend tip position.
  // The Z height on homing is measured by Z-Probe, but the probe is quite far from the hotend.
  // When the bed is uneven, this height must be corrected.
  FLOAT current_position_tmp[XYZ_NUM_AXIS] = {0.0F, 0.0F, 0.0F};
  FLOAT real_z = 0.0F;
  //get the real Z (since the auto bed leveling is already correcting the plane)
  real_z = sg_grbl::st_get_position_length(Z_AXIS);
  current_position_tmp[X_AXIS] = gcode::get_current_position(X_AXIS) + (FLOAT)x_probe_offset_from_extruder; // float
  current_position_tmp[Y_AXIS] = gcode::get_current_position(Y_AXIS) + (FLOAT)y_probe_offset_from_extruder; // float
  current_position_tmp[Z_AXIS] = gcode::get_current_position(Z_AXIS);
  //Apply the correction sending the probe offset
  sg_grbl::plan_apply_rotation_xyz(current_position_tmp[X_AXIS], current_position_tmp[Y_AXIS], current_position_tmp[Z_AXIS]);
  //The difference is added to current position and sent to planner.
  float value = gcode::get_current_position(Z_AXIS) + current_position_tmp[Z_AXIS] - real_z;
  gcode::set_current_position(Z_AXIS, value);

#ifdef ENABLE_AUTO_BED_LEVELING_DEBUG
  USER_DbgLog("cc_pos_z = %f",gcode::get_current_position(Z_AXIS));
#endif //ENABLE_AUTO_BED_LEVELING_DEBUG
  gcode::plan_set_current_position();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:calculateBedLevelFinish
 *@brief: 校准完成，保存校准值到sd卡，设置平台矩阵等等
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::calculateBedLevelFinish(void)
{
  SaveBedLevelZValue();
  // 设置平台矩阵
  sg_grbl::plan_set_bed_level_equation();
  motion_3d.is_cal_bed_platform = true;
  // 修正当前位置
  correcting_position();

  // 回到零点
  autoBedLeveling.do_blocking_move_to_zero();

  sg_grbl::st_clear_xyz_min_endstops_hit();
  sg_grbl::plan_set_process_auto_bed_level_status(true);
  autoBedLeveling.Gcode_M104(targetTemperature);//targetTemperature
}

/*
 *@class: AutoBedLevelingAdjust
 *@function:setup_for_endstop_move
 *@brief: 设置平台向限位移动的准备工作，设置速度，打开限位检测
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::setup_for_endstop_move(void)
{
  autoBedLeveling.feedrate = 1500.0F;
  sg_grbl::st_enable_endstops(true);
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:clean_up_after_endstop_move
*@brief: 对应上面的函数，关闭限位开关检测
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::clean_up_after_endstop_move(void)
{
  sg_grbl::st_enable_endstops(false);
}

/*
 *@class: AutoBedLevelingAdjust
 *@function:do_blocking_move_to_xy_getZ
 *@brief: move xy to a offset position. then get z positiong when bed touch limit up.
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::do_blocking_move_to_xy_getZ(FLOAT x, FLOAT y, FLOAT &z)
{
//  if(gcode::get_current_position(Z_AXIS) < z_raise_before_probing)
//    do_blocking_move_to(gcode::get_current_position(X_AXIS), gcode::get_current_position(Y_AXIS), z_raise_before_probing);
  UINT8 n;
  do_blocking_move_to(x + x_probe_offset_from_extruder, y + y_probe_offset_from_extruder, gcode::get_current_position(Z_AXIS)); // float
//  run_z_probe();
//  z = gcode::get_current_position(Z_AXIS);
  for(n=0; n<2; ++n)
  {
    run_z_probe();
    if(n==0)
    {
      z = gcode::get_current_position(Z_AXIS);
    }
    else
    {
      z += gcode::get_current_position(Z_AXIS); // float
    }
  }
  z/=n; // float
//  run_z_probe();
//  z += gcode::get_current_position(Z_AXIS);
//  run_z_probe();
//  z = (z + gcode::get_current_position(Z_AXIS)) / 3.0F;
  do_blocking_move_to(gcode::get_current_position(X_AXIS), gcode::get_current_position(Y_AXIS), gcode::get_current_position(Z_AXIS)+z_raise_before_probing);
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:run_z_probe
 *@brief:
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::run_z_probe(void)
{
  // 重置矩阵
  sg_grbl::plan_bed_level_matrix_set_identity();

  // 初始化Z下降速度
  autoBedLeveling.feedrate = gcode::homing_feedrate[Z_AXIS];//

  // 拷贝当前位置
  autoBedLeveling.copy_curr_position();

  // move down until you find the bed
  autoBedLeveling.mark_position(z_movedown_until_find_bed, Z_AXIS);
  gcode::process_buffer_line_normal(autoBedLeveling.plan_position, autoBedLeveling.feedrate/60); // float

  // we have to let the planner know where we are right now as it is not where we said to go.
  autoBedLeveling.mark_position(sg_grbl::st_get_position_length(Z_AXIS), Z_AXIS);
  sg_grbl::planner_set_position(autoBedLeveling.plan_position);

  // move up the retract distance
  autoBedLeveling.plan_position[Z_AXIS] += (FLOAT)sg_grbl::z_home_retract_mm * 10;
  gcode::process_buffer_line_normal(autoBedLeveling.plan_position, autoBedLeveling.feedrate/60/3); // float

  // move back down slowly to find bed
  autoBedLeveling.feedrate = gcode::homing_feedrate[Z_AXIS] / 8;
  autoBedLeveling.plan_position[Z_AXIS] -= (FLOAT)sg_grbl::z_home_retract_mm * 20; // float
  gcode::process_buffer_line_normal(autoBedLeveling.plan_position, autoBedLeveling.feedrate/60); // float

  // 获取当前点位置，设置Z位置
  gcode::set_current_position(Z_AXIS, sg_grbl::st_get_position_length(Z_AXIS));
  // make sure the planner knows where we are as it may be a bit different than we last said to move to
  gcode::plan_set_current_position();
}
/*
 *@class: AutoBedLevelingAdjust
 *@function:do_blocking_move_to
 *@brief: 移动到XYZ位置，速度取XY移动速度
 *@data: 2017/05/22
*/
void AutoBedLevelingAdjust::do_blocking_move_to(FLOAT x, FLOAT y, FLOAT z)
{
  FLOAT oldFeedRate = autoBedLeveling.feedrate;
  autoBedLeveling.feedrate = xy_travel_speed;
  gcode::set_current_position(Z_AXIS, z);
  gcode::process_buffer_line_normal_4_curr(autoBedLeveling.feedrate/180); // float
  gcode::set_current_position(X_AXIS, x);
  gcode::set_current_position(Y_AXIS, y);
//  gcode::set_current_position(Z_AXIS, z);
  gcode::process_buffer_line_normal_4_curr(autoBedLeveling.feedrate/60);
  autoBedLeveling.feedrate = oldFeedRate;
}

AutoBedLevelingAdjust autoBedLevelingAdjust;
/********************************************************End AutoBedLevelingAdjust*****************************************/



#endif //#ifdef ENABLE_AUTO_BED_LEVELING
