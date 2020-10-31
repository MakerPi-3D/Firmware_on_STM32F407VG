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

  static void machine_M2030HY_init_grbl()
  {
    float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_M2030HY;
    float buf5[] = DEFAULT_MAX_FEEDRATE_M2030HY;
    long buf6[] = DEFAULT_MAX_ACCELERATION_M2030HY;
    (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
    (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
    (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
  }

  //M2030HY机型E电机用的是混色的E电机(直径8.8mm),XY轴的同步轮也不一样
  void machine_M2030HY_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)(motion_3d_model.xyz_max_pos[X_AXIS] - 8);
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)(motion_3d_model.xyz_max_pos[Y_AXIS] - 8);
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)(motion_3d_model.xyz_max_pos[Z_AXIS]);
    motion_3d_model.enable_invert_dir[X_AXIS] = false; //M2030HY机型的X轴方向不一样
    motion_3d_model.enable_invert_dir[Y_AXIS] = true; //M2030HY机型的Y轴方向不一样
    t_custom_services.enable_led_light = true;//有LED灯照明功能
    machine_M2030HY_init_grbl();
  }


  #ifdef __cplusplus
} //extern "C" {
  #endif

}
