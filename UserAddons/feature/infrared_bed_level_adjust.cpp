#include "infrared_bed_level_adjust.h"
#include "infrared_z_zero_adjust.h"
#include <math.h>
#include "interface.h"
#include "view_common.h"
#include "stepper.h"
#include "user_debug.h"
#include "controlfunction.h"
#include "planner.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "Alter.h"
#include "user_interface.h"
#include "globalvariables.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef CAL_Z_ZERO_OFFSET
  static CONST FLOAT INFRARED_Z_DOWN_HIGH = 20.0F;          /*!< 校准平台下降值 */
  static volatile bool is_infrared_bed_level_adjust_start = false;   /*!< 启动红外调平校准 */
  static INT infrared_bed_level_adjust_status = 0;          /*!< 红外调平校准状态 */
  static FLOAT infrared_target[MAX_NUM_AXIS]= {0.0F};       /*!< 红外调平目标坐标 */
  static FLOAT infrared_feedrate = 30*60;                   /*!< 红外调平移动速度 */
  static FLOAT infrared_save_first_pos_z_high = 0.0F;       /*!< 红外调平保存第一点高度 */
  static FLOAT infrared_save_z_high_offset[3] = {0.0F};     /*!< 红外调平保存其他两点与第一点z高度偏差值 */
  static FLOAT  infrared_left_bed_pos = 20.0F;              /*!< 喷嘴碰平台左边 */
  static FLOAT  infrared_right_bed_pos = 260.0F;            /*!< 喷嘴碰平台右边 */
  static FLOAT  infrared_front_bed_pos = 1.0F;              /*!< 喷嘴碰平台前边 */
  static FLOAT  infrared_back_bed_pos = 210.0F;             /*!< 喷嘴碰平台后边 */
//  static FLOAT  infrared_mid_bed_pos_x = 100.0F;             /*!< 喷嘴碰平台后边 */
//  static FLOAT  infrared_mid_bed_pos_y = 100.0F;             /*!< 喷嘴碰平台后边 */
  static bool is_adjust_finish = false;

  bool infrared_bed_level_mark(void)
  {
    return is_infrared_bed_level_adjust_start;
  }
  // 开始校准Z平台
  void infrared_bed_level_adjust_start(void)
  {
    is_infrared_bed_level_adjust_start = true;
    is_adjust_finish = false;
  }

  // 初始化校准平台点坐标
  void infrared_bed_level_adjust_init(void)
  {
    if((M2030HY == t_sys_data_current.model_id) || (M2030 == t_sys_data_current.model_id))
    {
      infrared_left_bed_pos = 10.0F;
      infrared_right_bed_pos = 144.0F;
      infrared_front_bed_pos = 0.0F;
      infrared_back_bed_pos = 150.0F;
    }
    else if(K5 == t_sys_data_current.model_id)
    {
      infrared_left_bed_pos = 0.0F;
      infrared_right_bed_pos = 160.0F;
      infrared_front_bed_pos = 30.0F;
      infrared_back_bed_pos = 160.0F;
    }
    else if(M3145K == t_sys_data_current.model_id)
    {
      infrared_left_bed_pos = 0.0F;
      infrared_right_bed_pos = 300.0F;
      infrared_front_bed_pos = 0.0F;
      infrared_back_bed_pos = 250.0F;
    }
    else if(M3145S == t_sys_data_current.model_id)
    {
      infrared_left_bed_pos = 0.0F;
      infrared_right_bed_pos = 280.0F;
      infrared_front_bed_pos = 0.0F;
      infrared_back_bed_pos = 250.0F;
    }
    else if(M3145T == t_sys_data_current.model_id)
    {
      infrared_left_bed_pos = 60.0F;
      infrared_right_bed_pos = 290.0F;
      infrared_front_bed_pos = 70.0F;
      infrared_back_bed_pos = 190.0F;
//      infrared_mid_bed_pos_x = 182.5F;
//      infrared_mid_bed_pos_y = 130.0F;
    }
  }

  // 获取点(x,y)高度z
  static inline void getPosHigh(CONST FLOAT &x, CONST FLOAT &y, FLOAT &z, FLOAT z_offset = 0.0F)
  {
    // 平台下降到INFRARED_Z_DOWN_HIGH
    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
    sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
    infrared_target[Z_AXIS] = INFRARED_Z_DOWN_HIGH;
    gcode::process_buffer_line_normal(infrared_target,infrared_feedrate/60); // float

    // 喷嘴移动到(x,y)
    infrared_target[X_AXIS] = x;
    infrared_target[Y_AXIS] = y;
    gcode::process_buffer_line_normal(infrared_target,infrared_feedrate/60); // float

    // 平台上升到-INFRARED_Z_DOWN_HIGH
    sg_grbl::st_check_endstop_z_hit_min(); // 清空限位缓存
    motion_3d.is_open_infrared_z_min_check = true; // 打开红外调平检测
    sg_grbl::st_enable_endstops(true); // 打开限位检测，避免由于逻辑错乱，限位检测关闭导致无法限位
    infrared_target[Z_AXIS] = -INFRARED_Z_DOWN_HIGH;
    gcode::process_buffer_line_normal(infrared_target, 2);

    if(sg_grbl::st_check_endstop_z_hit_min())
    {
      z = sg_grbl::st_get_endstops_len(Z_AXIS);
      infrared_target[Z_AXIS] = z + z_offset; // float
//      USER_EchoLogStr("endstop xyz %f %f %f\r\n", infrared_target[X_AXIS], infrared_target[Y_AXIS], infrared_target[Z_AXIS]);
    }
    sg_grbl::planner_set_position(infrared_target);

    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
    (void)sys_os_delay(100);
  }

  // 移动到点（x,y,z）
  static inline void travel_to_pos(CONST FLOAT &x, CONST FLOAT &y, CONST FLOAT &z)
  {
    sg_grbl::st_enable_endstops(false);
    infrared_target[X_AXIS] = x;
    infrared_target[Y_AXIS] = y;
    infrared_target[Z_AXIS] = z;
    gcode::process_buffer_line_normal(infrared_target,infrared_feedrate/60); // float
//    USER_EchoLogStr("travel_to_pos:%f %f %f\r\n", x, y, z);
  }

  void infrared_bed_level_adjust_pos_Serial(INT z_high_id,UINT8 PointID)
  {
//    USER_EchoLogStr("circlenum=%.1f  ",circlenum);
    bool is_z_up = (0.0F > infrared_save_z_high_offset[z_high_id]);
    if(is_z_up)
    {
      USER_EchoLogStr("UP:%d\r\n",PointID);
    }
    else
    {
      USER_EchoLogStr("DOWN:%d\r\n",PointID);
    }
  }

  extern UCHAR Run_once_pic;
  static UCHAR no_adjust_count = 0;
  void infrared_bed_level_adjust_pos(INT z_high_id, INT pic_up_id, INT pic_down_id)
  {
    static bool showPic = true;
    //平台上升3次，测出平台高度与第一点比较
    while(1)
    {
      if(abs( infrared_save_z_high_offset[z_high_id]) <= 0.01F)
      {
        if((infrared_bed_level_adjust_status == 12) || (infrared_bed_level_adjust_status == 14) || (infrared_bed_level_adjust_status == 16) )
        {
          ++no_adjust_count;
        }
        USER_EchoLogStr("Complete, without adjust\r\n");
        USER_EchoLogStr("infrared_save_z_high_offset:%f\r\n",infrared_save_z_high_offset[z_high_id]);
        buzz(150);
        break;
      }
      else
      {
        no_adjust_count--;
        bool is_z_up = (infrared_save_z_high_offset[z_high_id] > 0.01F);//  (0.0F > infrared_save_z_high_offset[z_high_id]);
        if(showPic)
        {
          display_picture(is_z_up?pic_down_id:pic_up_id);
          USER_EchoLogStr("infrared_save_z_high_offset:%f\r\n",infrared_save_z_high_offset[z_high_id]);
          showPic = 0;
        }
        if(gpio_infrared_level_detection()  == (is_z_up?1:0))
        {
          if(gpio_infrared_level_detection()  == (is_z_up?1:0))
          {
            USER_EchoLogStr("Complete, adjust OK!\r\n");
            buzz(150);
            showPic = 1;
            break;
          }
        }
      }
      (void)sys_os_delay(10);
    }
  }

  // 归零显示页面
  void infrared_bed_level_adjust_auto_home(void)
  {
    if(gui_is_refresh())
    {
      display_picture(83);
    }
    if((!is_infrared_bed_level_adjust_start) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && (sg_grbl::planner_moves_planned() == 0))
    {
      extern void cal_z_zero_interface(void);
      gui_set_curr_display(cal_z_zero_interface);
    }
  }

  // 归零显示页面
  void infrared_bed_level_adjust_auto_home1(void)
  {
    if(gui_is_refresh())
    {
      display_picture(115);
    }
    if((!is_infrared_bed_level_adjust_start) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && (sg_grbl::planner_moves_planned() == 0))
    {
      extern void cal_z_zero_interface(void);
      gui_set_curr_display(cal_z_zero_interface);
    }
  }

  void infrared_bed_level_adjust_get_two_pos(void)
  {
    if(gui_is_refresh())
    {
      display_picture(91);//平台正在校准界面，避免在校准时按下按键

      infrared_feedrate = 70*60;
      if((3 == infrared_bed_level_adjust_status) || (22 == infrared_bed_level_adjust_status))
      {
        getPosHigh(infrared_left_bed_pos, infrared_front_bed_pos, infrared_save_first_pos_z_high);
        getPosHigh(infrared_left_bed_pos, infrared_back_bed_pos, infrared_save_z_high_offset[0]);
        infrared_save_z_high_offset[0] -= infrared_save_first_pos_z_high; // float

        (void)sys_os_delay(100);
        buzz(100);

        getPosHigh(infrared_right_bed_pos, infrared_back_bed_pos, infrared_save_z_high_offset[1]);
        infrared_save_z_high_offset[1] -= infrared_save_first_pos_z_high; // float

        (void)sys_os_delay(100);
        buzz(100);

        getPosHigh(infrared_right_bed_pos, infrared_front_bed_pos, infrared_save_z_high_offset[2]);
        infrared_save_z_high_offset[2] -= infrared_save_first_pos_z_high; // float

        (void)sys_os_delay(100);
        buzz(100);

        getPosHigh(infrared_left_bed_pos, infrared_front_bed_pos, infrared_save_first_pos_z_high);
      }
      else
      {
        getPosHigh(infrared_left_bed_pos, infrared_front_bed_pos, infrared_save_first_pos_z_high);
      }
      ++infrared_bed_level_adjust_status;
    }
  }

  void infrared_bed_level_adjust_left_back(void)
  {
    if(gui_is_refresh())
    {
      display_picture(91);//平台正在校准界面，避免在校准时按下按键.

      travel_to_pos(infrared_left_bed_pos, infrared_back_bed_pos, infrared_save_first_pos_z_high);
      infrared_bed_level_adjust_pos_Serial(0, 2);
      infrared_bed_level_adjust_pos(0, 86, 99);
      ++infrared_bed_level_adjust_status;
    }

  }

  void infrared_bed_level_adjust_right_front(void)
  {
    if(gui_is_refresh())
    {
      display_picture(91);//平台正在校准界面，避免在校准时按下按键

      travel_to_pos(infrared_right_bed_pos, infrared_front_bed_pos, infrared_save_first_pos_z_high);
      infrared_bed_level_adjust_pos_Serial(1, 4);
      infrared_bed_level_adjust_pos(1, 88, 101);
      ++infrared_bed_level_adjust_status;
    }
  }

  void infrared_bed_level_adjust_left_front(void)
  {
    if(gui_is_refresh())
    {
      display_picture(91);//平台正在校准界面，避免在校准时按下按键

      travel_to_pos(infrared_left_bed_pos, infrared_front_bed_pos, infrared_save_first_pos_z_high);
      infrared_bed_level_adjust_pos_Serial(0, 1);
      infrared_bed_level_adjust_pos(0, 110, 111);
      ++infrared_bed_level_adjust_status;
    }

  }

  void infrared_bed_level_adjust_right_back(void)
  {
    if(gui_is_refresh())
    {
      display_picture(91);//平台正在校准界面，避免在校准时按下按键
      travel_to_pos(infrared_right_bed_pos, infrared_back_bed_pos, infrared_save_first_pos_z_high);
      infrared_bed_level_adjust_pos_Serial(1, 3);
      infrared_bed_level_adjust_pos(1, 87, 100);
      ++infrared_bed_level_adjust_status;
    }
  }

  void infrared_adjust_finish_serial(void)
  {
    is_adjust_finish = true;
  }

  // 调平完成
  void infrared_bed_level_adjust_finish(void)
  {
    static ULONG BeepWaringTime=0;
    if(gui_is_refresh())
    {
      display_picture(90);
      respond_gui_send_sem(OpenBeep);
      BeepWaringTime=sys_task_get_tick_count()+5000; //鸣叫5秒
      USER_EchoLogStr("adjustbedlevel finish");
    }
    if(BeepWaringTime < sys_task_get_tick_count()) //时间到关闭鸣叫
    {
      respond_gui_send_sem(CloseBeep);
    }

    if(touchxy(84,215,184,264) || is_adjust_finish)
    {
      respond_gui_send_sem(CloseBeep);
      // M3145K调用归零接口，正常向上归零
      if(2 == t_sys_data_current.enable_bed_level)
      {
        sys_send_gcode_cmd("G28 isInternal");
      }
      else
      {
        infrared_z_zero_adjust_autohome();
      }
      t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
      motion_3d.is_open_infrared_z_min_check = false;
      is_infrared_bed_level_adjust_start = false;
      // M3145K有上限位，不进行Z零点校准
      if(2 == t_sys_data_current.enable_bed_level)
      {
        gui_set_curr_display(maindisplayF);
      }
      else
      {
        gui_set_curr_display(infrared_bed_level_adjust_auto_home);
      }
    }
  }

  static INT bed_level_adjust_count = 0;

  // 调平检测四点螺丝有没松动
  void infrared_bed_level_adjust_check(void)
  {
    if(gui_is_refresh())
    {
      display_picture(89);
    }

    if(touchxy(84,215,184,264))
    {
      if(1 == bed_level_adjust_count)
      {
        infrared_bed_level_adjust_status = 2;
      }
      else if(2 == bed_level_adjust_count)
      {
        infrared_bed_level_adjust_status = 21;
      }
    }
  }

  // 调平检测四点螺丝有没松动
  void infrared_adjust_check(void)
  {
    infrared_bed_level_adjust_status = 2;
  }

  void infrared_next_adjust_check(void)
  {
    infrared_bed_level_adjust_status = 21;
  }

  void infrared_bed_level_adjust_interface(void)
  {
    if(is_infrared_bed_level_adjust_start)
    {
      // xyz归零
      if(0 == infrared_bed_level_adjust_status)
      {
        motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
        gui_set_curr_display(infrared_bed_level_adjust_auto_home);
        t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
        infrared_z_zero_adjust_autohome();
        infrared_bed_level_adjust_status = 1;
      }
      // 设置红外校准目标坐标初始值
      else if((1 == infrared_bed_level_adjust_status) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && (sg_grbl::planner_moves_planned() == 0) && (gcode::get_current_position(X_AXIS) == 0))
      {
        bed_level_adjust_count = 1;
        USER_EchoLogStr("G29 Home Done"); // 串口返回G28归零成功
        for(int8_t i=0; i < motion_3d.axis_num; ++i)
        {
          infrared_target[i] = gcode::get_current_position(i);
        }
        sg_grbl::planner_set_position(infrared_target);
        gui_set_curr_display(infrared_bed_level_adjust_check);
        infrared_bed_level_adjust_status = -1;
      }
      // 以左下角为基准点，获取左上、右下点Z高度偏移量
      else if(2 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_get_two_pos);
        infrared_bed_level_adjust_status = 3;
      }
      // 相对左下角，调整左上角点z高度
      else if(4 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_left_back);
        infrared_bed_level_adjust_status = 5;
      }
      // 相对左下角，调整右下角点z高度
      else if(6 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_back);
        infrared_bed_level_adjust_status = 7;
      }
      // 相对左下角，调整右下角点z高度
      else if(8 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_front);
        infrared_bed_level_adjust_status = 9;
      }
      else if(10 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_get_two_pos);
        infrared_bed_level_adjust_status = 11;
        no_adjust_count = 0;
      }
      else if(12 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_left_back);
        infrared_bed_level_adjust_status = 13;
      }
      // 相对左下角，调整右下角点z高度
      else if(14 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_back);
        infrared_bed_level_adjust_status = 15;
      }
      // 相对左下角，调整右下角点z高度
      else if(16 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_front);
        infrared_bed_level_adjust_status = 17;
      }
      else if(18 == infrared_bed_level_adjust_status)
      {
        if(3 == no_adjust_count)
        {
          infrared_bed_level_adjust_status = 37;
        }
        else
        {
          infrared_bed_level_adjust_status = 19;
        }
      }


      // xyz归零
      else if(19 == infrared_bed_level_adjust_status)
      {
        motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
        gui_set_curr_display(infrared_bed_level_adjust_auto_home1);
        t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
        infrared_z_zero_adjust_autohome();
        infrared_bed_level_adjust_status = 20;
        USER_EchoLogStr("G29 Next Adjust"); // 串口返回G28归零成功
      }
      // 设置红外校准目标坐标初始值
      else if((20 == infrared_bed_level_adjust_status) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && (sg_grbl::planner_moves_planned() == 0) && (gcode::get_current_position(X_AXIS) == 0))
      {
        bed_level_adjust_count = 2;
        USER_EchoLogStr("G29 Next Home Done"); // 串口返回G28归零成功
        for(int8_t i=0; i < motion_3d.axis_num; ++i)
        {
          infrared_target[i] = gcode::get_current_position(i);
        }
        sg_grbl::planner_set_position(infrared_target);
        gui_set_curr_display(infrared_bed_level_adjust_check);
        infrared_bed_level_adjust_status = -1;
      }
      // 以右下角为基准点，获取左下、右上点Z高度偏移量
      else if(21 == infrared_bed_level_adjust_status)
      {
        infrared_bed_level_adjust_status = 22;
        gui_set_curr_display(infrared_bed_level_adjust_get_two_pos);

      }
      // 相对左下角，调整左上角点z高度
      else if(23 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_left_back);
        infrared_bed_level_adjust_status = 24;
      }
      // 相对左下角，调整右下角点z高度
      else if(25 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_back);
        infrared_bed_level_adjust_status = 26;
      }
      // 相对左下角，调整右下角点z高度
      else if(27 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_front);
        infrared_bed_level_adjust_status = 28;
      }
      else if(29 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_get_two_pos);
        infrared_bed_level_adjust_status = 30;
        no_adjust_count = 0;
      }
      else if(31 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_left_back);
        infrared_bed_level_adjust_status = 32;
      }
      // 相对左下角，调整右下角点z高度
      else if(33 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_back);
        infrared_bed_level_adjust_status = 34;
      }
      // 相对左下角，调整右下角点z高度
      else if(35 == infrared_bed_level_adjust_status)
      {
        gui_set_curr_display(infrared_bed_level_adjust_right_front);
        infrared_bed_level_adjust_status = 36;
      }
      // 调平完成
      else if(37 == infrared_bed_level_adjust_status)
      {
        bed_level_adjust_count = 0;
        gui_set_curr_display(infrared_bed_level_adjust_finish);
        infrared_bed_level_adjust_status = -1;
      }
    }
    else
    {
      infrared_bed_level_adjust_status = 0;
    }
  }
#endif

#ifdef __cplusplus
} // extern "C" {
#endif






