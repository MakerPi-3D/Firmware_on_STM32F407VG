#include "Configuration.h"
#include "ConfigurationStore.h"
#include "globalvariables_ccmram.h"
#include "config_model_tables.h"
#include "machine.h"
#include "Alter.h"

namespace sg_machine
{
  #ifdef __cplusplus
  extern "C" {
  #endif

  // 常规机型
  extern void machine_M2030_init();
  extern void machine_M2048_init();
  extern void machine_M3145_init();
  extern void machine_M4141_init();

  // 结构相同，M14不带热床，M14R03带热床
  extern void machine_M14_init();
  extern void machine_M14R03_init();

  extern void machine_M14S_init();

  extern void machine_M2030HY_init();

  // M4141S
  extern void machine_M4141S_init();
  extern void machine_M4141S_NEW_init();
  extern void machine_AMP410W_init();

  extern void machine_M3145S_init();

  extern void machine_M3145K_init();

  extern void machine_M41G_init();

  extern void machine_K5_init();

  extern uint8_t MinTempWarningPopSet;
  extern uint8_t HeatFailWarningPopSet;

  float axis_steps_per_unit_buf[MAX_NUM_AXIS] = DEFAULT_AXIS_STEPS_PER_UNIT;
  float max_feedrate_buf[MAX_NUM_AXIS] = DEFAULT_MAX_FEEDRATE;
  unsigned long max_acc_units_per_sq_second_buf[MAX_NUM_AXIS] = DEFAULT_MAX_ACCELERATION;

  // 机器型号参数初始化
  static void machine_model_param_init(void)
  {
    // 3d打印基本参数
    motion_3d.step = DEFAULT_STEP;
    motion_3d.axis_num = 4;
    motion_3d.enable_check_door_open = false;
    motion_3d.disable_z_max_limit = true;
    motion_3d.enable_poweroff_up_down_min_min = false;
    motion_3d.enable_board_test = true;
    // 定制服务参数
    t_custom_services.disable_abs = false;
    t_custom_services.disable_hot_bed = false;
    t_custom_services.enable_warning_light = false;
    t_custom_services.enable_led_light = false;
    // 3d打印机型配置信息
    // 反转轴初始化
    motion_3d_model.enable_invert_dir[X_AXIS] = INVERT_X_DIR;
    motion_3d_model.enable_invert_dir[Y_AXIS] = INVERT_Y_DIR;
    motion_3d_model.enable_invert_dir[Z_AXIS] = INVERT_Z_DIR;
    motion_3d_model.enable_invert_dir[E_AXIS] = INVERT_E0_DIR;
    motion_3d_model.enable_invert_dir[B_AXIS] = INVERT_E1_DIR;
    motion_3d_model.enable_invert_dir[5] = INVERT_E2_DIR;
    // 最大位置初始化
    motion_3d_model.xyz_max_pos[X_AXIS] = X_MAX_POS;
    motion_3d_model.xyz_max_pos[Y_AXIS] = Y_MAX_POS;
    motion_3d_model.xyz_max_pos[Z_AXIS] = Z_MAX_POS;
    // 最小位置初始化
    motion_3d_model.xyz_min_pos[X_AXIS] = X_MIN_POS;
    motion_3d_model.xyz_min_pos[Y_AXIS] = Y_MIN_POS;
    motion_3d_model.xyz_min_pos[Z_AXIS] = Z_MIN_POS;
    // 归零位置初始化
    motion_3d_model.xyz_home_pos[X_AXIS] = X_MIN_POS;
    motion_3d_model.xyz_home_pos[Y_AXIS] = Y_MIN_POS;
    motion_3d_model.xyz_home_pos[Z_AXIS] = Z_MIN_POS;
    // 最大行程初始化
    motion_3d_model.xyz_max_length[X_AXIS] = X_MAX_LENGTH;
    motion_3d_model.xyz_max_length[Y_AXIS] = Y_MAX_LENGTH;
    motion_3d_model.xyz_max_length[Z_AXIS] = Z_MAX_LENGTH;
    // 归零E电机反抽长度
    motion_3d_model.xyz_home_retract_mm[X_AXIS] = X_HOME_RETRACT_MM;
    motion_3d_model.xyz_home_retract_mm[Y_AXIS] = Y_HOME_RETRACT_MM;
    motion_3d_model.xyz_home_retract_mm[Z_AXIS] = Z_HOME_RETRACT_MM;
    // 归零方向初始化
    motion_3d_model.xyz_home_dir[X_AXIS] = X_HOME_DIR;
    motion_3d_model.xyz_home_dir[Y_AXIS] = Y_HOME_DIR;
    motion_3d_model.xyz_home_dir[Z_AXIS] = Z_HOME_DIR;
    // 最大移动位置初始化
    motion_3d_model.xyz_move_max_pos[X_AXIS] = X_MAX_POS;
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = Y_MAX_POS;
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = Z_MAX_POS;
    // Z原始最大位置初始化
    motion_3d_model.z_max_pos_origin = Z_MAX_POS;
    motion_3d.extrude_min_temp = static_cast<float>(EXTRUDE_MINTEMP);
    motion_3d.is_open_infrared_z_min_check = false;

    if (1 == t_sys_data_current.enable_color_mixing)
    {
      motion_3d.axis_num = 5; // 混色机型为5轴
    }

    if (1 == t_sys_data_current.enable_powerOff_recovery)
    {
      motion_3d.disable_z_max_limit = false; //断电续打必须有下限位开关
    }

    if (1 == t_sys_data_current.have_set_machine)
    {
      motion_3d.enable_board_test = false; // 已经设置好机型，关闭电路板测试功能
    }

    #if LASER_MODE

    if (t_sys_data_current.IsLaser)
    {
      MinTempWarningPopSet = 1;
      HeatFailWarningPopSet = 1;
    }

    #endif
  }

  // 机器型号尺寸初始化
  static void machine_model_size_init(void)
  {
    //根据model, 查表获取机型尺寸
    for (int i = 0; i < 3; ++i)
      motion_3d_model.xyz_max_pos[i] = (model_size_table[t_sys_data_current.model_id][i]);

    // 保存初始Z最大位置
    motion_3d_model.z_max_pos_origin = motion_3d_model.xyz_max_pos[Z_AXIS];

    // 自动调平高度可能小于默认最大高度，忽略修改
    if ((t_sys_data_current.poweroff_rec_z_max_value > (motion_3d_model.xyz_max_pos[Z_AXIS] - CAL_Z_MAX_POS_OFFSET)) //除去+8，2017.10.8  //加8是防止校准值小于默认值太多，引起最大行程警报，2017.7.14john
        || (1 == t_sys_data_current.enable_bed_level))
    {
      motion_3d_model.xyz_max_pos[Z_AXIS] = t_sys_data_current.poweroff_rec_z_max_value;
    }
    else
    {
      t_sys_data_current.poweroff_rec_z_max_value = motion_3d_model.xyz_max_pos[Z_AXIS];
    }

    for (int i = 0; i < 3; ++i)
    {
      motion_3d_model.xyz_max_length[i] = motion_3d_model.xyz_max_pos[i];
    }

    motion_3d_model.extrude_maxlength = motion_3d_model.xyz_max_length[X_AXIS] + motion_3d_model.xyz_max_length[Y_AXIS];
  }

  // 初始化机型
  static void machine_model_init(void)
  {
    if (M3145 == t_sys_data_current.model_id)   //M3145 实际行程318 XY可调行程310
    {
      machine_M3145_init();
    }
    else if (M14R03 == t_sys_data_current.model_id || M15 == t_sys_data_current.model_id)     //M14R03 实际行程148 可调行程140
    {
      machine_M14R03_init();
    }
    else if (M14S == t_sys_data_current.model_id)     //M14S 实际行程148 可调行程140
    {
      machine_M14S_init();
    }
    else if (M41G == t_sys_data_current.model_id)
    {
      machine_M41G_init();
    }
    else if (M3145S == t_sys_data_current.model_id || M3036 == t_sys_data_current.model_id)     //M3145S 实际行程318 可调行程310
    {
      machine_M3145S_init();
    }
    else if (M3145K == t_sys_data_current.model_id)
    {
      machine_M3145K_init();
    }
    else if (M2030 == t_sys_data_current.model_id)     //M2030实际行程200 可调行程308
    {
      machine_M2030_init();
    }
    else if (M2030HY == t_sys_data_current.model_id)     //M2030HY实际行程200 可调行程308
    {
      machine_M2030HY_init();
    }
    else if (K5 == t_sys_data_current.model_id)
    {
      machine_K5_init();
    }
    else if (M14 == t_sys_data_current.model_id)
    {
      machine_M14_init();
    }
    else if (M4141S == t_sys_data_current.model_id)
    {
      machine_M4141S_init();
    }
    else if (M4141S_NEW == t_sys_data_current.model_id)
    {
      machine_M4141S_NEW_init();
    }
    else if (AMP410W == t_sys_data_current.model_id)
    {
      machine_AMP410W_init();
    }
    else if (M2048 == t_sys_data_current.model_id)
    {
      machine_M2048_init();
    }
    else if ((M4141 == t_sys_data_current.model_id))
    {
      machine_M4141_init();
    }
    else
    {
      motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)motion_3d_model.xyz_max_pos[X_AXIS];
      motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)motion_3d_model.xyz_max_pos[Y_AXIS];
      motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)motion_3d_model.xyz_max_pos[Z_AXIS];
    }
  }

  // 重置归零坐标
  static void machine_reset_home_pos(void)
  {
    if (motion_3d_model.xyz_home_dir[X_AXIS] == -1)
    {
      #ifdef BED_CENTER_AT_0_0
      motion_3d_model.xyz_home_pos[X_AXIS] = motion_3d_model.xyz_max_length[X_AXIS] * -0.5;
      #else
      motion_3d_model.xyz_home_pos[X_AXIS] = X_MIN_POS;
      #endif //BED_CENTER_AT_0_0
    }
    else
    {
      #ifdef BED_CENTER_AT_0_0
      motion_3d_model.xyz_home_pos[X_AXIS] = motion_3d_model.xyz_max_length[X_AXIS] * 0.5;
      #else
      motion_3d_model.xyz_home_pos[X_AXIS] = motion_3d_model.xyz_move_max_pos[X_AXIS];//maxPos[0];
      #endif //BED_CENTER_AT_0_0
    }

    if (motion_3d_model.xyz_home_dir[Y_AXIS] == -1)
    {
      #ifdef BED_CENTER_AT_0_0
      motion_3d_model.xyz_home_pos[Y_AXIS] = motion_3d_model.xyz_max_length[Y_AXIS] * -0.5;
      #else
      motion_3d_model.xyz_home_pos[Y_AXIS] = Y_MIN_POS;
      #endif //BED_CENTER_AT_0_0
    }
    else
    {
      #ifdef BED_CENTER_AT_0_0
      motion_3d_model.xyz_home_pos[Y_AXIS] = motion_3d_model.xyz_max_length[Y_AXIS] * 0.5;
      #else
      motion_3d_model.xyz_home_pos[Y_AXIS] = motion_3d_model.xyz_move_max_pos[Y_AXIS]; //maxPos[1];
      #endif //BED_CENTER_AT_0_0
    }

    if (motion_3d_model.xyz_home_dir[Z_AXIS] == -1)
    {
      motion_3d_model.xyz_home_pos[Z_AXIS] = Z_MIN_POS;
    }
    else
    {
      motion_3d_model.xyz_home_pos[Z_AXIS] = motion_3d_model.xyz_move_max_pos[Z_AXIS]; //maxPos[2];
    }
  }

  // 重置grbl
  static void machine_reset_grbl()
  {
    #if 0

    // 颗粒机器为测试机型，需要时再开启
    if (0U != t_sys.is_granulator)
    {
      const float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_KELI;
      (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
    }

    #endif

    if (0U != t_sys_data_current.enable_soft_filament) //如果打开了软料功能，则需要改变挤出头电机齿轮直径；
    {
      axis_steps_per_unit_buf[E_AXIS] = E_AXIS_STEPS_PER_UNIT_SOFT;//改变E电机为小齿轮

      if (0U != t_sys_data_current.enable_color_mixing)
      {
        axis_steps_per_unit_buf[B_AXIS] = B_AXIS_STEPS_PER_UNIT_SOFT;//改变B电机为小齿轮
      }
    }

    if (t_sys_data_current.is_2GT)
    {
      axis_steps_per_unit_buf[X_AXIS] = XY_AXIS_STEPS_PER_UNIT_2GT;
      axis_steps_per_unit_buf[Y_AXIS] = XY_AXIS_STEPS_PER_UNIT_2GT;
    }

    for (int i = 0; i < MAX_NUM_AXIS; ++i)
    {
      sg_grbl::axis_steps_per_unit[i] = axis_steps_per_unit_buf[i] * static_cast<float>(motion_3d.step); // float
      sg_grbl::max_feedrate[i] = max_feedrate_buf[i];
      sg_grbl::max_acceleration_units_per_sq_second[i] = max_acc_units_per_sq_second_buf[i];
    }

    // steps per sq second need to be updated to agree with the units per sq second
    sg_grbl::reset_acceleration_rates();
  }

  void machine_init()
  {
    machine_model_param_init();
    machine_model_size_init();
    machine_model_init();
    machine_reset_home_pos();

    // 腔体温度与断料、堵料检测冲突
    if ((1 == t_sys_data_current.enable_cavity_temp) && (t_sys_data_current.model_id != M41G))
    {
      t_sys_data_current.enable_material_check = false;
    }

    // 如果沒有門檢測和斷電開啓，强制開啓上下共限位，兼容電路板兩種接法：
    // 1、老板衹有Zmax；2、新板door占用Zmax，Zmax、Zmin共限位
    if (t_sys_data_current.enable_powerOff_recovery && (!motion_3d.enable_check_door_open))
    {
      motion_3d.enable_poweroff_up_down_min_min = true;
      motion_3d.updown_g28_first_time = 1;
      t_power_off.is_power_off = 0; // 设置非断电状态
    }
    else if (t_sys_data_current.enable_powerOff_recovery && motion_3d.enable_check_door_open)
    {
      // 支持斷電與門檢測，强制開啓上下共限位，機型目前只支持M14R03
      if ((M14R03 == t_sys_data_current.model_id) || (M3145T == t_sys_data_current.model_id))
      {
        motion_3d.enable_poweroff_up_down_min_min = true;
        motion_3d.updown_g28_first_time = 1;
      }
    }

    machine_reset_grbl();
  }


  #ifdef __cplusplus
}//extern "C" {
  #endif

}


