#include "gcodebufferhandle.h"
#include "Configuration.h"
#include "globalvariables.h"
#include "stepper.h"
#include "planner.h"
#include "sys_function.h"
#include "Alter.h"
#include "mechanical_bed_level_adjust.h"
#include "autobedlevelinterface.h"
#include "config_model_tables.h"
#include "gcode_global_params.h"

#include <math.h>

namespace gcode
{

  static bool hitZEndstop = false;
  static bool code_seen_xyz[3] = {false, false, false};
//    static float code_value_xyz[3] = {0, 0, 0};

  static volatile bool home_all_axis = true;
  extern "C" float Laser_Print_Position;

  void process_command_synchronize(void)
  {
#ifdef NEWCOREXY
    //修复特殊机型COREXY，归零先碰X轴则会一直撞X轴限位开关的问题，
    while(!st_check_queue_is_empty())
    {
      if(stepper_axis_xyz_read_min(X_AXIS) && !stepper_axis_xyz_read_min(Y_AXIS))
      {
        st_quick_stop();
      }
      (void)sys_os_delay(50);
    }
#else
    sg_grbl::st_synchronize();
#endif
  }


  static void axis_is_at_home(INT axis)
  {
    current_position[axis] = motion_3d_model.xyz_home_pos[axis] + add_homing[axis];
  }


  static void homeaxis(const volatile int8_t axis)
  {
    INT axis_home_dir = motion_3d_model.xyz_home_dir[axis];

    current_position[axis] = 0.0F;
    sg_grbl::planner_set_position(current_position);
    sg_grbl::st_synchronize();

    destination[axis] = 1.5F * motion_3d_model.xyz_max_length[axis] * static_cast<FLOAT>(axis_home_dir); // float
    if(t_sys_data_current.IsMechanismLevel && axis==Z_AXIS)
      feed_rate = homing_feedrate[axis] * 6;
    else
      feed_rate = homing_feedrate[axis];
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    current_position[axis] = 0.0F;
    sg_grbl::planner_set_position(current_position);
    sg_grbl::st_synchronize();

    destination[axis] = -motion_3d_model.xyz_home_retract_mm[axis] * static_cast<FLOAT>(axis_home_dir); // float
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    destination[axis] = 2.0F*motion_3d_model.xyz_home_retract_mm[axis] * static_cast<FLOAT>(axis_home_dir); // float
    feed_rate = homing_feedrate[axis]/2.0F ; // float
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    axis_is_at_home(axis);
    destination[axis] = current_position[axis];
    feed_rate = 0.0F;

  }

  void Home_Zaxis_DownToBottom(void)
  {
    INT axis_home_dir = -motion_3d_model.xyz_home_dir[Z_AXIS];
    current_position[Z_AXIS] = 0.0F;
    sg_grbl::planner_set_position(current_position);

    destination[Z_AXIS] = 1.5F * motion_3d_model.xyz_max_length[Z_AXIS] * static_cast<FLOAT>(axis_home_dir); // float
    feed_rate = homing_feedrate[Z_AXIS]* 2.0F; // float
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    current_position[Z_AXIS] = 0.0F;
    sg_grbl::planner_set_position(current_position);
    feed_rate = homing_feedrate[Z_AXIS]/2.0F ; // float
    destination[Z_AXIS] = -motion_3d_model.xyz_home_retract_mm[Z_AXIS] * static_cast<FLOAT>(axis_home_dir); // float
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    destination[Z_AXIS] = 2.0F*motion_3d_model.xyz_home_retract_mm[Z_AXIS] * static_cast<FLOAT>(axis_home_dir); // float
    feed_rate = homing_feedrate[Z_AXIS]/2.0F ; // float
    process_buffer_line_normal(destination, feed_rate/60.0F);

    current_position[Z_AXIS] = t_sys_data_current.poweroff_rec_z_max_value;
    sg_grbl::planner_set_position(current_position);

    Zaxis_RunOnce = true;
  }

  void Home_Zaxis_ToLaser(void)
  {
    sg_grbl::planner_set_position(current_position);
    destination[Z_AXIS] = Laser_Print_Position;
    feed_rate = homing_feedrate[Z_AXIS]* 2.0F; // float
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    current_position[Z_AXIS] = destination[Z_AXIS]; // axis_is_at_home
    feed_rate = 0.0F;
  }


  void Home_Zaxis_UpToTop(void)
  {
    sg_grbl::planner_set_position(current_position);
    destination[Z_AXIS] = 0.0F;
    feed_rate = homing_feedrate[Z_AXIS]* 2.0F; // float
    process_buffer_line_normal(destination, feed_rate/60.0F); // float

    axis_is_at_home(Z_AXIS);
    destination[Z_AXIS] = current_position[Z_AXIS];
    feed_rate = 0.0F;
  }

  void Laser_Mechanical_Process(void)
  {
    if(!Zaxis_RunOnce)
      Home_Zaxis_DownToBottom();
    if(t_sys_data_current.IsLaser)
      Home_Zaxis_ToLaser();
    else if(!mechanical_bed_level_mark())
      Home_Zaxis_UpToTop();
  }


  void GcodeCMD_G28_home_axis(void)
  {
    if((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS])))
    {
      homeaxis(X_AXIS);
    }

    if((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS])))
    {
      homeaxis(Y_AXIS);
    }

    if((home_all_axis) || (parseGcodeBufHandle.codeSeen(axis_codes[Z_AXIS])))
    {
#if (Z_HOME_DIR == 1)
#if 0
      sg_grbl::planner_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
      if((parseGcodeBufHandle.codeSeen(sg_grbl::axis_codes[Z_AXIS]) && parseGcodeBufHandle.codeValueLong() == 0))
      {
        feed_rate = sg_grbl::homing_feedrate[X_AXIS];
        if(sg_grbl::homing_feedrate[Y_AXIS]<feed_rate)
          feed_rate =sg_grbl::homing_feedrate[Y_AXIS];
        destination[Z_AXIS] = 0.0;
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feed_rate/60, active_extruder);
      }
      feed_rate = 0.0;
      st_synchronize();

      memcpy(current_position, destination, sizeof(FLOAT)*3);
#endif
#elif Z_HOME_DIR < 0                      // If homing towards BED do Z last

#ifdef CAL_Z_ZERO_OFFSET
      // M3145K归零向上
      if(2U == t_sys_data_current.enable_bed_level)
      {
        if(!t_sys_data_current.IsMechanismLevel)
        {
          homeaxis(Z_AXIS);
        }
        else if(1U == t_sys.is_bed_level_down_to_zero)
        {
          if (parseGcodeBufHandle.codeSeen('O')||!t_sys_data_current.IsMechanismLevel)
          {
            homeaxis(Z_AXIS);
          }
          else
          {
            Laser_Mechanical_Process();
          }
        }
      }
      else
      {
        homeaxis(Z_AXIS);
      }
#else
      homeaxis(Z_AXIS);
#endif
#endif
    }
  }


  void GcodeCMD_G28_quick_home(void)
  {
#ifdef QUICK_HOME
    if((home_all_axis)||( (parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS])) && (parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS]))) )  //first diagonal move
    {

      // 设置当前点为零点
      current_position[X_AXIS] = 0.0F;
      current_position[Y_AXIS] = 0.0F;
      sg_grbl::planner_set_position(current_position);
      process_command_synchronize();

      // xy再次往负方向移动，直至撞击限位
      destination[X_AXIS] = (1.5F * motion_3d_model.xyz_max_length[X_AXIS]) * static_cast<FLOAT>(motion_3d_model.xyz_home_dir[X_AXIS]); // float
      destination[Y_AXIS] = (1.5F * motion_3d_model.xyz_max_length[Y_AXIS]) * static_cast<FLOAT>(motion_3d_model.xyz_home_dir[Y_AXIS]); // float
      feed_rate = homing_feedrate[X_AXIS];
      if(homing_feedrate[Y_AXIS]<feed_rate)
      {
        feed_rate = homing_feedrate[Y_AXIS];
      }
      process_buffer_line_normal(destination, feed_rate/60.0F); // float
      process_command_synchronize();

      // 设置零点位置
      axis_is_at_home(X_AXIS);
      axis_is_at_home(Y_AXIS);
      sg_grbl::planner_set_position(current_position);
      process_command_synchronize();

      // 移动到零点
      destination[X_AXIS] = current_position[X_AXIS];
      destination[Y_AXIS] = current_position[Y_AXIS];
      process_buffer_line_normal(destination, feed_rate/60.0F); // float
      process_command_synchronize();

      for(int i = 0; i < 3; i++)
      {
        current_position[i] = destination[i];
      }
    }
#endif
  }

  void GcodeCMD_G28_process(void)
  {
    FLOAT saved_feedrate = feed_rate;
    INT saved_feedmultiply = feed_multiply;
    feed_multiply = 100;
    previous_xTaskGetTickCount_cmd = sys_task_get_tick_count();
    sg_grbl::st_enable_endstops(true);
    if(motion_3d.axis_num != 0U)
    {
      for(int i = 0; i < motion_3d.axis_num; i++)
      {
        destination[i] = current_position[i];
      }
    }
    feed_rate = 0.0F;

#if Z_HOME_DIR > 0                      // If homing away from BED do Z first
    if((home_all_axis) || (parseGcodeBufHandle.codeSeen(sg_grbl::axis_codes[Z_AXIS])))
    {
      homeaxis(Z_AXIS);
    }
#endif

    GcodeCMD_G28_quick_home();       // 快速归零
    GcodeCMD_G28_home_axis();        // 各轴归零

#ifdef ENDSTOPS_ONLY_FOR_HOMING
    sg_grbl::st_enable_endstops(false);
#endif

    feed_rate = saved_feedrate;
    feed_multiply = saved_feedmultiply;
    previous_xTaskGetTickCount_cmd = sys_task_get_tick_count();

  }
  static void is_at_z_max(void)
  {
    bool code_seen_all = !((code_seen_xyz[X_AXIS]) || (code_seen_xyz[Y_AXIS]) || (code_seen_xyz[Z_AXIS]));
    if(0U == motion_3d.updown_g28_first_time)
    {
      if(code_seen_all || (code_seen_xyz[Z_AXIS]))
      {
        // 修復停止打印時，Z降到限位位置時，打印第二個圖，Z軸判斷當前位置為零點，導致Z無法上升
        // 1、判斷當前是否處於限位狀態
        // 2、如果當前Z位置大於零
        // 以上2點可判斷Z處於下限位，前提：Z已經判斷過零點位置
        if((sg_grbl::st_get_z_max_endstops_status()) && (current_position[Z_AXIS] > /*motion_3d_model.xyz_max_pos[Z_AXIS] - */ static_cast<FLOAT>(CAL_Z_MAX_POS_OFFSET)))
        {
          sg_grbl::st_enable_endstops(false);
          FLOAT m_feedrate = homing_feedrate[Z_AXIS]/2.0F ; // float
          sg_grbl::planner_set_position(current_position);
          current_position[Z_AXIS] -= 10.0F; // float
          process_buffer_line_normal(current_position, m_feedrate/60.0F); // float
        }
      }
    }
  }

  static bool judge_z_zero_pos(void)
  {
    bool result = false;
    bool code_seen_all = !((code_seen_xyz[X_AXIS]) || (code_seen_xyz[Y_AXIS]) || (code_seen_xyz[Z_AXIS]));
    // 上電執行一次，判斷Z零點位置
    if((0U != motion_3d.enable_poweroff_up_down_min_min) //使能了上下共限位
        && (0U != motion_3d.updown_g28_first_time) //上电后第一次归零
      )
    {
      if(code_seen_all || (code_seen_xyz[Z_AXIS]))
      {
        FLOAT m_feedrate = homing_feedrate[Z_AXIS]/2.0F; // float

        // 一开始碰触限位，不知道是Zmin还是Zmax
        if(hitZEndstop)
        {
          // M4141\M4040平台太重，断电会导致平台下降
          // 上电归零，平台可能会撞击z max限位，如果平台下降0.6密码，会导致异响
          if ((M4141 != t_sys_data_current.model_id) && (M4040 != t_sys_data_current.model_id))
          {
            // 平台向下移动0.6mm
            sg_grbl::st_enable_endstops(false);
            current_position[Z_AXIS] = 0.0F;
            sg_grbl::planner_set_position(current_position);
            current_position[Z_AXIS] = 0.6F;
            process_buffer_line_normal(current_position, m_feedrate/60.0F); // float
          }

          // 判断是否接触到Z限位
          // 是，当前位置在Z最大限位，执行向上操作，然后归零
          // 否，当前位置在Z最小限位，直接归零
          // 向下移动0.6mm，限位依然没松开，判断当前限位为Zmax
          if(sg_grbl::st_get_z_max_endstops_status())// && M3145T != t_sys_data_current.model_id) // 测试
          {
            // 远离Zmax
            sg_grbl::st_enable_endstops(false);
            if ((M4040 == t_sys_data_current.model_id) || (M4141 == t_sys_data_current.model_id))
            {
              current_position[Z_AXIS] -= 8.0F; // float
            }
            else
            {
              current_position[Z_AXIS] -= 3.0F; // float
            }
            process_buffer_line_normal(current_position, m_feedrate/60.0F); // float
          }
        }
        result = true;
      }
    }
    return result;
  }

  void GcodeCMD_G28_set_current_pos(void)
  {
    for(INT i = 0; i < 3; ++i)
    {
      if(home_all_axis)
      {
        // G28，xyz三轴置home位置
        current_position[i]= motion_3d_model.xyz_home_pos[i] + add_homing[i];
      }
      else
      {
        // G28 X0 Y0 Z0，xyz置为对应轴的参数值
        if(parseGcodeBufHandle.codeSeen(axis_codes[i]))
        {
          if(parseGcodeBufHandle.codeValueLong() != 0)
          {
            current_position[i]=parseGcodeBufHandle.codeValue()+add_homing[i]; // float
          }
          else
          {
            current_position[i]= motion_3d_model.xyz_home_pos[i] + add_homing[i]; // float
          }
        }
      }
    }
    sg_grbl::planner_set_position(current_position);
  }

  void g28_process(void)
  {
    t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
    for(INT i = 0; i < 3; ++i)
    {
      code_seen_xyz[i] = parseGcodeBufHandle.codeSeen(axis_codes[i]);
//        if((code_seen_xyz[i]) && (parseGcodeBufHandle.codeValueLong() != 0))
//        {
//          code_value_xyz[i] = parseGcodeBufHandle.codeValue();
//        }
//        else
//        {
//          code_value_xyz[i] = motion_3d_model.xyz_home_pos[i];
//        }
    }
    home_all_axis = !((parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS])) || (parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS])) || (parseGcodeBufHandle.codeSeen(axis_codes[Z_AXIS])));
#ifdef CAL_Z_ZERO_OFFSET
    if(2U == t_sys_data_current.enable_bed_level)
    {
      if(1U == t_sys.is_bed_level_down_to_zero)
      {

      }
      else if(2U == t_sys.is_bed_level_down_to_zero)
      {
        if ((!parseGcodeBufHandle.codeSeen('C')) && ((parseGcodeBufHandle.codeSeen('Z')) || home_all_axis))
        {
          motion_3d.is_open_infrared_z_min_check = false;
          sg_grbl::st_enable_endstops(true);
          FLOAT m_feedrate = homing_feedrate[Z_AXIS] ;
          sg_grbl::planner_set_position(current_position);
          current_position[Z_AXIS] += 20.0F; // float
          process_buffer_line_normal(current_position, m_feedrate/60.0F); // float
          if(parseGcodeBufHandle.codeSeen('O'))
          {
            motion_3d.is_open_infrared_z_min_check = true;
          }
          else
          {
            motion_3d.is_open_infrared_z_min_check = false;
          }
        }
        else
        {
          motion_3d.is_open_infrared_z_min_check = false;
        }
      }
    }
#endif
    gcodeCMD_g28_bed_level_start(static_cast<INT>(tmp_extruder));
    if(0U == motion_3d.updown_g28_first_time) // 已经执行第一次归零
    {
      is_at_z_max(); // 判断是否在z最大位置
    }
    else
    {
      // 上电第一次归零确认z最大限位状态
      if(sg_grbl::st_get_z_max_endstops_status())
      {
        hitZEndstop = true;
      }
    }

    if(1U == t_sys_data_current.enable_bed_level)
    {
      if(0U == motion_3d.updown_g28_first_time)
      {
        if(0U == is_cal_bed_platform())
        {
          GcodeCMD_G28_process();
        }
        else
        {
          gcodeCMD_g28_interface(active_extruder);
        }
      }
      else
      {
        GcodeCMD_G28_process();
      }
    }
    else
    {
      GcodeCMD_G28_process();
    }

    if(judge_z_zero_pos())
    {
      GcodeCMD_G28_process();
      motion_3d.updown_g28_first_time = 0U;
    }
    gcodeCMD_g28_bed_level_end();
    GcodeCMD_G28_set_current_pos();  // 设置xyz归零坐标
    if(t_sys_data_current.IsMechanismLevel ? (home_all_axis || parseGcodeBufHandle.codeSeen(axis_codes[Z_AXIS])) : 1)
      t_gui_p.G28_ENDSTOPS_COMPLETE = 1U;
    if(0 == parseGcodeBufHandle.codeSeen('S')) //G28带S参数则不返回G28 ok
    {

    }
#ifdef CAL_Z_ZERO_OFFSET
    if(2U == t_sys_data_current.enable_bed_level)
    {
      if(t_sys_data_current.IsMechanismLevel && 1U == t_sys.is_bed_level_down_to_zero && (home_all_axis || parseGcodeBufHandle.codeSeen(axis_codes[Z_AXIS])))
      {
        if(t_sys_data_current.IsLaser)
        {
          current_position[Z_AXIS] = Laser_Print_Position;
        }
        else if(!mechanical_bed_level_mark())
        {
          current_position[Z_AXIS] = 0;
        }
        else
        {
          current_position[Z_AXIS] = t_sys_data_current.poweroff_rec_z_max_value;
        }

        sg_grbl::planner_set_position(current_position);
        sg_grbl::st_synchronize();
      }
      else if(2U == t_sys.is_bed_level_down_to_zero)
      {
        // Z零点补偿
        if ((!parseGcodeBufHandle.codeSeen('C')) && ((parseGcodeBufHandle.codeSeen('Z')) || home_all_axis) && (abs(t_sys_data_current.z_offset_value) > 0.0F))
        {

          if(parseGcodeBufHandle.codeSeen('O'))
          {
            sg_grbl::st_enable_endstops(false);
            FLOAT m_feedrate = homing_feedrate[Z_AXIS] ;
            sg_grbl::planner_set_position(current_position);
            current_position[Z_AXIS] = t_sys_data_current.z_offset_value;
            process_buffer_line_normal(current_position, m_feedrate/60.0F); // float

            current_position[Z_AXIS] = 0.0F;
            sg_grbl::planner_set_position(current_position);
            sg_grbl::st_synchronize();
          }
        }
      }
    }
    if(parseGcodeBufHandle.codeSeen(axis_codes[X_AXIS]) || parseGcodeBufHandle.codeSeen(axis_codes[Y_AXIS]) || parseGcodeBufHandle.codeSeen(axis_codes[Z_AXIS]))
    {
      sg_grbl::planner_set_position(current_position);
      sg_grbl::st_synchronize();
    }
#endif
  }


}







