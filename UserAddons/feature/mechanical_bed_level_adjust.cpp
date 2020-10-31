#include "mechanical_bed_level_adjust.h"
#include "infrared_z_zero_adjust.h"
#include <math.h>
#include "interface.h"
#include "view_common.h"
#include "view_commonf.h"
#include "stepper.h"
#include "user_debug.h"
#include "controlfunction.h"
#include "config_model_tables.h"
#include "user_interface.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "alter.h"
#include "controlxyz.h"
#include "cmsis_os.h"
#include "planner.h"
#include "ConfigurationStore.h"
#include "Alter.h"
#include "globalvariables.h"
#include "planner.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif
  extern FLOAT Limit_Z_Off;
  extern UINT8 MinTempWarningPopSet,MaxTempWarningPopSet;
  extern void z_down_to_bottom(void);
  extern void SaveCalculateZMaxPos(float ZMaxPosValue);
#ifdef CAL_Z_ZERO_OFFSET
  static const float INFRARED_Z_DOWN_HIGH = 10.0f;            /*!< 校准平台下降值 */
  static volatile bool is_mechanical_bed_level_adjust_start = false;   /*!< 启动红外调平校准 */
  static volatile int mechanical_bed_level_adjust_status = 0; /*!< 红外调平校准状态 */
  static float mechanical_target[MAX_NUM_AXIS]= {0.0f};       /*!< 红外调平目标坐标 */
  static float mechanical_feedrate = 80*60;                   /*!< 红外调平移动速度 */

  static int8_t mechanical_z_high_index_order[4] = {0, 1, 2, 3}; /*!< 四点Z高度索引顺序 */
  static const float  mechanical_left_bed_pos = 20.0f;              /*!< 喷嘴碰平台左边 */
  static const float  mechanical_right_bed_pos = 260.0f;            /*!< 喷嘴碰平台右边 */
  static const float  mechanical_front_bed_pos = 1.0f;              /*!< 喷嘴碰平台前边 */
  static const float  mechanical_back_bed_pos = 210.0f;             /*!< 喷嘴碰平台后边 */
  static int bed_level_adjust_count = 0;

  typedef struct
  {
    float z_high_offset;
    float infrared_x_pos;
    float infrared_y_pos;
    unsigned char down_PointID;
    unsigned char up_PointID;
    uint8_t is_z_high_get;//是否获取z高度
  } Move_Pos;

  Move_Pos level_pos[4]=
  {
    {0.0f, mechanical_left_bed_pos,  mechanical_front_bed_pos , 102, 111, 0},
    {0.0f, mechanical_left_bed_pos,  mechanical_back_bed_pos  , 86, 99, 0},
    {0.0f, mechanical_right_bed_pos, mechanical_back_bed_pos  , 87, 100, 0},
    {0.0f, mechanical_right_bed_pos, mechanical_front_bed_pos , 88, 101, 0},
  };

  bool mechanical_bed_level_mark(void)
  {
    return is_mechanical_bed_level_adjust_start;
  }

  static void Auto_Level_Init(void)
  {
    if((2 == t_sys_data_current.enable_bed_level)&&(t_sys_data_current.model_id==K5))
    {
#ifdef CAL_Z_ZERO_OFFSET
      GPIO_InitTypeDef GPIO_InitStruct;
      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
      /*Configure GPIO pin : PE2*/
      GPIO_InitStruct.Pin = GPIO_PIN_6;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif
    }
  }

  void Level_Before_Warn(void)
  {
    static float tmp_poweroff_rec_z_max_value = 0.0f;
    if(gui_is_refresh())
    {
      display_picture(116);
      tmp_poweroff_rec_z_max_value = t_sys_data_current.poweroff_rec_z_max_value;
      t_sys_data_current.poweroff_rec_z_max_value = motion_3d_model.z_max_pos_origin - 20.0f; // 调平归零设置最大高度为默认机型高度减去20mm，避免刮喷嘴
    }

    if(touchxy(40,180,200,290))
    {
      Auto_Level_Init();
      is_mechanical_bed_level_adjust_start = true;
      Zaxis_RunOnce = false;
    }
    else if(touchxy(270,180,430,290))
    {
      Gui_Mode = MODE_FILUM_LEVEL;
      gui_set_curr_display(unloadfilament0F);
      goto_page_homing();
      respond_gui_send_sem(BackFilamentValue);
      return ;
    }
    else if(touchxy(30,0,180,80))
    {
      t_sys_data_current.poweroff_rec_z_max_value = tmp_poweroff_rec_z_max_value;
      t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
      gui_set_curr_display(settingF);
    }
  }

  void Replace_Warn(void)
  {
    if(gui_is_refresh())
    {
      mechanical_feedrate = 70*60;
      motion_3d.is_open_infrared_z_min_check = false; // 打开红外调平检测
      sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
      mechanical_target[Z_AXIS] = 50;
      gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/60);
      display_picture(117);
    }
    if(touchxy(150,160,340,320))
    {
      mechanical_bed_level_adjust_status = 2;
    }
  }

  // 开始校准Z平台
  void mechanical_bed_level_adjust_start(void)
  {
    MinTempWarningPopSet=1;
    MaxTempWarningPopSet=1;
    gui_set_curr_display(Level_Before_Warn);
  }

  // 初始化校准平台点坐标
  void mechanical_bed_level_adjust_init(void)
  {
    if(M2030HY == t_sys_data_current.model_id || M2030 == t_sys_data_current.model_id)
    {
      level_pos[0].infrared_x_pos=level_pos[1].infrared_x_pos=10.0f;
      level_pos[2].infrared_x_pos=level_pos[3].infrared_x_pos=154.0f;
      level_pos[0].infrared_y_pos=level_pos[3].infrared_y_pos=10.0f;
      level_pos[1].infrared_y_pos=level_pos[2].infrared_y_pos=150.0f;
    }
    else if(K5 == t_sys_data_current.model_id)
    {
      level_pos[0].infrared_x_pos=level_pos[1].infrared_x_pos=10.0f;//5.0f;
      level_pos[2].infrared_x_pos=level_pos[3].infrared_x_pos=170.0f;//185.0f;
      level_pos[0].infrared_y_pos=level_pos[3].infrared_y_pos=30.0f;
      level_pos[1].infrared_y_pos=level_pos[2].infrared_y_pos=180.0f;
    }
    else if(M3145K == t_sys_data_current.model_id)
    {
      level_pos[0].infrared_x_pos=level_pos[1].infrared_x_pos=0.0f;
      level_pos[2].infrared_x_pos=level_pos[3].infrared_x_pos=300.0f;
      level_pos[0].infrared_y_pos=level_pos[3].infrared_y_pos=0.0f;
      level_pos[1].infrared_y_pos=level_pos[2].infrared_y_pos=250.0f;
    }
    else if(M3145S == t_sys_data_current.model_id)
    {
      level_pos[0].infrared_x_pos=level_pos[1].infrared_x_pos=0.0f;
      level_pos[2].infrared_x_pos=level_pos[3].infrared_x_pos=280.0f;
      level_pos[0].infrared_y_pos=level_pos[3].infrared_y_pos=0.0f;
      level_pos[1].infrared_y_pos=level_pos[2].infrared_y_pos=250.0f;
    }
    else if(M3145T == t_sys_data_current.model_id)
    {
      level_pos[0].infrared_x_pos=level_pos[1].infrared_x_pos=60.0f;
      level_pos[2].infrared_x_pos=level_pos[3].infrared_x_pos=290.0f;
      level_pos[0].infrared_y_pos=level_pos[3].infrared_y_pos=70.0f;
      level_pos[1].infrared_y_pos=level_pos[2].infrared_y_pos=190.0f;
    }

  }

  // 获取点(x,y)高度z
  static inline void getPosHigh(const float &x, const float &y, float &z, float z_offset = 0.0f)
  {
    // 平台下降到INFRARED_Z_DOWN_HIGH
    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
    sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
    mechanical_target[Z_AXIS] = INFRARED_Z_DOWN_HIGH;
    gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/60);

    // 喷嘴移动到(x,y)
    mechanical_target[X_AXIS] = x;
    mechanical_target[Y_AXIS] = y;
    gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/60);

    // 平台上升到-INFRARED_Z_DOWN_HIGH
    sg_grbl::st_check_endstop_z_hit_min(); // 清空限位缓存
    motion_3d.is_open_infrared_z_min_check = true; // 打开红外调平检测
    sg_grbl::st_enable_endstops(true); // 打开限位检测，避免由于逻辑错乱，限位检测关闭导致无法限位
    mechanical_target[Z_AXIS] = -INFRARED_Z_DOWN_HIGH*2;
    gcode::process_buffer_line_normal(mechanical_target, 480/60);

    if(sg_grbl::st_check_endstop_z_hit_min())
    {
      z = sg_grbl::st_get_endstops_len(Z_AXIS);
      mechanical_target[Z_AXIS] = z + z_offset;
    }
    sg_grbl::planner_set_position(mechanical_target);

    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
    (void)sys_os_delay(100);
  }

  // 移动到点（x,y,z）
  static inline void travel_to_pos(const float &x, const float &y, const float &z)
  {
    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
    mechanical_target[Z_AXIS] = INFRARED_Z_DOWN_HIGH;
    gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/60);

    sg_grbl::st_enable_endstops(false);
    mechanical_target[X_AXIS] = x;
    mechanical_target[Y_AXIS] = y;
    gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/60);

    mechanical_target[Z_AXIS] = z;
    gcode::process_buffer_line_normal(mechanical_target,480/60);
  }

  static void mechanical_bed_level_adjust_pos(int z_high_id, int pic_id, int compare_id)
  {
    bool showPic = true;
    bool is_z_up = (level_pos[compare_id].z_high_offset > level_pos[z_high_id].z_high_offset);
    //平台上升3次，测出平台高度与第一点比较
    while(1)
    {
      //差值小于0.1，无需调
      if( 0.1f >= abs(level_pos[z_high_id].z_high_offset - level_pos[compare_id].z_high_offset))
      {
        buzz(150);
        break;
      }
      else
      {
        if(showPic)
        {
          showPic = 0;
          display_picture(pic_id);
        }
        if((!gpio_mechanical_level_detection()) == is_z_up)
        {
          sys_os_delay(10);
          if((!gpio_mechanical_level_detection()) == is_z_up)
          {
            for(int i = 0; i < 4; i++)
            {
              level_pos[i].is_z_high_get = 0;
            }
            buzz(150);
            break;
          }
        }
      }
      (void)sys_os_delay(10);
    }
  }

  // 归零显示页面
  void mechanical_bed_level_adjust_auto_home(void)
  {
    if(gui_is_refresh())
    {
      display_picture(83);
    }
    if(!is_mechanical_bed_level_adjust_start && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && sg_grbl::planner_moves_planned() == 0)
    {
      extern void cal_z_zero_interface(void);
      gui_set_curr_display(cal_z_zero_interface);
    }
  }

  // 归零显示页面
  void mechanical_bed_level_adjust_auto_home1(void)
  {
    if(gui_is_refresh())
    {
      display_picture(115);
    }
    if(!is_mechanical_bed_level_adjust_start && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && sg_grbl::planner_moves_planned() == 0)
    {
      extern void cal_z_zero_interface(void);
      gui_set_curr_display(cal_z_zero_interface);
    }
    if(0U == t_gui_p.G28_ENDSTOPS_COMPLETE)
    {
      motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
      sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
      mechanical_target[Z_AXIS] += 20;
      gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/120);

      sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
      mechanical_target[X_AXIS] = 120;
      mechanical_target[Y_AXIS] = 100;
      gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/120);
    }
    t_gui_p.G28_ENDSTOPS_COMPLETE = 1U;

  }

  void Auto_bed_level_z_zero(void)
  {
    motion_3d.is_open_infrared_z_min_check = true; // 打开红外调平检测
    sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
    mechanical_target[Z_AXIS] = 0-25;
    gcode::process_buffer_line_normal(mechanical_target,mechanical_feedrate/60);

    mechanical_target[Z_AXIS]=0;
    sg_grbl::planner_set_position(mechanical_target);
  }

  // 获取Z高度索引顺序
  void mechanical_get_z_high_index_order(void)
  {
    for(int8_t i=0; i<4; i++)
    {
      if(i+1 < 4)
      {
        for(int8_t j=i+1; j<4; j++)
        {
          if(level_pos[mechanical_z_high_index_order[j]].z_high_offset > level_pos[mechanical_z_high_index_order[i]].z_high_offset)
          {
            int8_t tmp = mechanical_z_high_index_order[i];
            mechanical_z_high_index_order[i] = mechanical_z_high_index_order[j];
            mechanical_z_high_index_order[j] = tmp;
          }
        }
      }
    }
  }

  // 显示调平图片
  static void mechanical_bed_level_adjust_show_pic(void)
  {
    if(1 == bed_level_adjust_count)
    {
      display_picture(119);
    }
    else if(2 == bed_level_adjust_count)
    {
      display_picture(120);
    }
  }

  static void get_pos_z_high(const int pos_index)
  {
    if(level_pos[pos_index].is_z_high_get == 0)
    {
      getPosHigh(level_pos[pos_index].infrared_x_pos, level_pos[pos_index].infrared_y_pos, level_pos[pos_index].z_high_offset);
      level_pos[pos_index].is_z_high_get = 1;
      (void)sys_os_delay(100);
      buzz(100);
    }
  }

  // 获取四点z高度
  void mechanical_bed_level_get_four_pos_z(void)
  {
    mechanical_bed_level_adjust_show_pic();

    mechanical_feedrate = 100*60;

    Auto_bed_level_z_zero();

    for(char i=0; i<4; i++)
    {
      get_pos_z_high(i);
      mechanical_z_high_index_order[i] = i;
    }
    mechanical_get_z_high_index_order();
  }

  // 获取四点z高度
  void mechanical_bed_level_get_four_pos_z_gui(void)
  {
    if(gui_is_refresh())
    {
      if(5 == mechanical_bed_level_adjust_status || 9 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status++;
      }
    }
  }

  // 两点比较调平
  static void mechanical_bed_level_compare_adjust(int base_id, int compare_id, float z_high_compare)
  {
    // 移动到compare_id点
    travel_to_pos(level_pos[compare_id].infrared_x_pos, level_pos[compare_id].infrared_y_pos, z_high_compare);
    // 获取图片id
    int tmp_pic_id = level_pos[compare_id].down_PointID;
    if(level_pos[compare_id].z_high_offset - level_pos[base_id].z_high_offset > 0.0f)
    {
      tmp_pic_id = level_pos[compare_id].up_PointID;
    }
    // 调整compare_id点高度
    mechanical_bed_level_adjust_pos(compare_id, tmp_pic_id, base_id);
    level_pos[compare_id].z_high_offset = z_high_compare;
    (void)sys_os_delay(100);
  }

  // 调平4点
  static void mechanical_bed_level_adjust_four_pos(bool is_coarse_tuning)
  {
    mechanical_bed_level_get_four_pos_z();//重新获取四点高度
    int8_t min_z_pos_index = mechanical_z_high_index_order[0];// Z最低点索引

    int coarse_tuning_count = 1;
    if(is_coarse_tuning)//粗调3次，避免调平出现螺母太松问题
    {
      coarse_tuning_count = 3;
    }

    for(int8_t i=0; i<coarse_tuning_count; i++)
    {
      for(char cnt=1; cnt<4; cnt++)// 最低点与其他3点比较调平
      {
        int8_t index = mechanical_z_high_index_order[cnt];
        if( 0.1f > abs(level_pos[index].z_high_offset - level_pos[min_z_pos_index].z_high_offset))
        {
          continue;
        }
        mechanical_bed_level_adjust_show_pic();
        float z = level_pos[min_z_pos_index].z_high_offset;
        if(is_coarse_tuning)
        {
          z = abs(level_pos[index].z_high_offset - level_pos[min_z_pos_index].z_high_offset);
          if(0 == i)
          {
            z = z * 3 / 4;
          }
          else if(1 == i)
          {
            z = z / 2;
          }
          else if(2 == i)
          {
            z = z / 4;
          }
          z = level_pos[min_z_pos_index].z_high_offset - z;
        }
        mechanical_bed_level_compare_adjust(min_z_pos_index, index, z);
      }
    }
  }

  void mechanical_bed_level_adjust_move(void)
  {
    if(mechanical_bed_level_adjust_status==7)
    {
      mechanical_bed_level_adjust_four_pos(true);//四点调平, 粗调
      mechanical_bed_level_adjust_status = 8;
    }
    else if(mechanical_bed_level_adjust_status==11)
    {
      for(int i = 0; i < 2; i++)
      {
        mechanical_bed_level_adjust_four_pos(false);//四点调平
      }
      mechanical_bed_level_adjust_status = 12;
    }
  }

  // 调平完成
  void mechanical_bed_level_adjust_finish(void)
  {
    static unsigned long BeepWaringTime=0;
    if(gui_is_refresh())
    {
      display_picture(118);
      respond_gui_send_sem(OpenBeep);
      BeepWaringTime=xTaskGetTickCount()+5000; //鸣叫5秒
    }

    if(BeepWaringTime < xTaskGetTickCount()) //时间到关闭鸣叫
    {
      respond_gui_send_sem(CloseBeep);
    }
  }

  void mechanical_bed_level_save(void)
  {
    if(gui_is_refresh())
    {
      display_picture(122);
      motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
      sg_grbl::st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
      sg_grbl::planner_set_axis_position(0.0f, Z_AXIS);
      sg_grbl::st_check_endstop_z_hit_max(); // 清空限位缓存
      for(int i = 0; i < MAX_NUM_AXIS; i++)
      {
        sg_grbl::st_set_position_axis(0.0f, i);
      }
      sys_os_delay(50);
      z_down_to_bottom(); // Z下降到底部
      sg_grbl::st_synchronize();
    }

    if(sg_grbl::st_check_endstop_z_hit_max())
    {
      float ZMaxPosValue;
      ZMaxPosValue = sg_grbl::st_get_endstops_len(Z_AXIS)+Limit_Z_Off;
      SaveCalculateZMaxPos(ZMaxPosValue);
      gui_set_curr_display(mechanical_bed_level_adjust_finish); //跳转调平成功
    }
  }

  // 调4点最低
  void Check_Screw_Warn(void)
  {
    if(gui_is_refresh())
    {
      display_picture(121);
    }

    if(touchxy(170,200,340,320))
    {
      mechanical_bed_level_adjust_status=4;
    }
  }

  // xyz归零
  static void mechanical_proc_homing(void)
  {
    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
    mechanical_bed_level_adjust_status = 1;
    gui_set_curr_display(mechanical_bed_level_adjust_auto_home);
    infrared_z_zero_adjust_autohome();
  }

  // 更换调平模块提示
  static void mechanical_switch_level_module_warm(void)
  {
    bed_level_adjust_count = 1;
    for(int8_t i=0; i < motion_3d.axis_num; i++)
    {
      mechanical_target[i] = gcode::get_current_position(i);
    }
    sg_grbl::planner_set_position(mechanical_target);
    if(2 == t_sys_data_current.enable_v5_extruder)
    {
      gcode::set_fan_speed(0);
    }
    gpio_e_motor_fan_control(false);
    mechanical_bed_level_adjust_status = -1;
    gui_set_curr_display(Replace_Warn);
  }

  void mechanical_bed_level_adjust_interface(void)
  {
    if(is_mechanical_bed_level_adjust_start)
    {
      if(0 == mechanical_bed_level_adjust_status)
      {
        mechanical_proc_homing();// xyz归零
      }
      else if(1 == mechanical_bed_level_adjust_status && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && sg_grbl::planner_moves_planned() == 0)
      {
        mechanical_switch_level_module_warm();// 更换调平模块提示
      }
      else if(2 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status = 3;
        gui_set_curr_display(Check_Screw_Warn);// 调4点最低
      }
      else if(4 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status = 5;
        gui_set_curr_display(mechanical_bed_level_get_four_pos_z_gui);// 获取四点z高度
      }
      else if(6 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status = 7;
        gui_set_curr_display(mechanical_bed_level_adjust_move);
      }
      else if(8 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status = 9;
        bed_level_adjust_count = 2;
        gui_set_curr_display(mechanical_bed_level_get_four_pos_z_gui);// 获取四点z高度
      }
      else if(10 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status = 11;
        gui_set_curr_display(mechanical_bed_level_adjust_move);
      }
      // 调平完成
      else if(12 == mechanical_bed_level_adjust_status)
      {
        mechanical_bed_level_adjust_status = -1;
        bed_level_adjust_count = 0;
        //保存高度值
        gui_set_curr_display(mechanical_bed_level_save);
      }
    }
    else
    {
      mechanical_bed_level_adjust_status = 0;
    }
  }

  void gui_adjust_z_zero_high(void)
  {
    if(gui_is_refresh())
    {
      display_picture(128);

      char TextXYZPosbuffer[20];
      memset(TextXYZPosbuffer, 0, sizeof(char)*20);

      snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "Z stroke:");
      DisplayTextDefault ((UINT8*)TextXYZPosbuffer,160,24);

      sys_send_gcode_cmd("G91 isInternal"); // 开启绝对模式
      sys_send_gcode_cmd("G1 F1200 X-10 isInternal"); // Z增加60mm
      sys_send_gcode_cmd("G90 isInternal");     // 关闭绝对模式

      SetTextDisplayRange(304,24,12*5,24,&ZPosTextShape);
      ReadTextDisplayRangeInfo(ZPosTextShape,ZPosRangeBuf);
      snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3.1f",t_sys_data_current.poweroff_rec_z_max_value);
      CopyTextDisplayRangeInfo(ZPosTextShape,ZPosRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextXYZPosbuffer, ZPosTextShape,TextRangeBuf);

    }

    if(gui_is_rtc())	  //根据rtc信号更新数值显示
    {
      char TextXYZPosbuffer[20];
      snprintf(TextXYZPosbuffer, sizeof(TextXYZPosbuffer), "%3.1f",t_sys_data_current.poweroff_rec_z_max_value);
      CopyTextDisplayRangeInfo(ZPosTextShape,ZPosRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextXYZPosbuffer, ZPosTextShape,TextRangeBuf);
    }


    if(touchxy(80,160,240,290))    //上升
    {
      float z = t_sys_data_current.poweroff_rec_z_max_value;
      SaveCalculateZMaxPos(z+0.1f);
      sys_send_gcode_cmd("G92 Z0.1 isInternal");     // 关闭绝对模式
      sys_send_gcode_cmd("G91 isInternal"); // 开启绝对模式
      sys_send_gcode_cmd("G1 F600 Z-0.1 I0 D1 H0 isInternal"); // Z增加60mm
      sys_send_gcode_cmd("G90 isInternal");     // 关闭绝对模式
    }

    if(touchxy(260,160,410,290))    //下降
    {
      float z = t_sys_data_current.poweroff_rec_z_max_value;
      SaveCalculateZMaxPos(z-0.1f);
      sys_send_gcode_cmd("G91 isInternal"); // 开启绝对模式
      sys_send_gcode_cmd("G1 F600 Z+0.1 I0 D1 H0 isInternal"); // Z增加60mm
      sys_send_gcode_cmd("G90 isInternal");     // 关闭绝对模式
      sys_send_gcode_cmd("G92 Z0 isInternal");     // 关闭绝对模式
    }

    if(touchxy(30,0,180,80))    //返回
    {
      gui_set_curr_display(settingF);
    }
  }


#endif

#ifdef __cplusplus
} // extern "C" {
#endif






