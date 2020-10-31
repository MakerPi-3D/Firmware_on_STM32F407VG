#include "Configuration.h"
#include "ConfigurationStore.h"
#include "globalvariables_ccmram.h"

#include <string.h>
namespace sg_machine
{
  #ifdef __cplusplus
  extern "C" {
  #endif

  extern float axis_steps_per_unit_buf[MAX_NUM_AXIS];
  extern float max_feedrate_buf[MAX_NUM_AXIS];
  extern unsigned long max_acc_units_per_sq_second_buf[MAX_NUM_AXIS];

  static void machine_K5_init_grbl()
  {
    if (0U != t_sys_data_current.enable_color_mixing)
    {
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_X5;
      float buf5[] = DEFAULT_MAX_FEEDRATE_X5;
      long buf6[] = DEFAULT_MAX_ACCELERATION_X5;
      (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
      (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
      (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
      sg_grbl::max_xy_jerk = 5.0f;
      sg_grbl::max_z_jerk = 0.2f;
      sg_grbl::max_e_jerk = 2.5f;
      sg_grbl::max_b_jerk = 2.5f;
      sg_grbl::acceleration = 300;
      sg_grbl::retract_acceleration = 500;
    }
    else
    {
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_K5;
      float buf5[] = DEFAULT_MAX_FEEDRATE_K5;
      long buf6[] = DEFAULT_MAX_ACCELERATION_K5;
      (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
      (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
      (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
      sg_grbl::max_xy_jerk = 5.0f;
      sg_grbl::max_z_jerk = 0.2f;
      sg_grbl::max_e_jerk = 2.5f;
      sg_grbl::max_b_jerk = 2.5f;
      sg_grbl::acceleration = 300;
      sg_grbl::retract_acceleration = 500;
    }
  }

  void machine_K5_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)(motion_3d_model.xyz_max_pos[X_AXIS] - 3);
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)(motion_3d_model.xyz_max_pos[Y_AXIS] - 3);
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)(motion_3d_model.xyz_max_pos[Z_AXIS]);
    // K5机型X轴Home方向与普通机器相反，会导致打印模型X轴镜像
    // 正常机器home方向为Max-》Min
    // K5机器home方向为Min-》Max
    motion_3d_model.xyz_home_dir[X_AXIS] = 1;
    motion_3d_model.enable_invert_dir[X_AXIS] = false;
    motion_3d_model.enable_invert_dir[Y_AXIS] = true;
    t_custom_services.enable_led_light = true;   //有LED灯照明功能

    if (t_sys_data_current.ui_number)
      t_sys.lcd_ssd1963_43_480_272 = t_sys_data_current.ui_number;
    else
      t_sys.lcd_ssd1963_43_480_272 = 1;

    if (0 == t_sys_data_current.logo_id) // k5机型没有logo，显示sg默认logo
    {
      if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id)
      {
        t_sys_data_current.logo_id = 7;
      }
      else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id)
      {
        t_sys_data_current.logo_id = 9;
      }
      else
      {
        t_sys_data_current.logo_id = 8;
      }
    }

    machine_K5_init_grbl();
  }


  #ifdef __cplusplus
} //extern "C" {
  #endif

}
