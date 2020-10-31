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

  static void machine_M2048_init_grbl()
  {
    if (0U != t_sys_data_current.enable_color_mixing)
    {
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_M2048X;
      float buf5[] = DEFAULT_MAX_FEEDRATE_M2048X;
      long buf6[] = DEFAULT_MAX_ACCELERATION_M2048X;
      (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
      (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
      (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
    }
    else
    {
      float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_M2048;
      float buf5[] = DEFAULT_MAX_FEEDRATE_M2048;
      long buf6[] = DEFAULT_MAX_ACCELERATION_M2048;
      (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
      (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
      (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
    }
  }

  void machine_M2048_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)motion_3d_model.xyz_max_pos[X_AXIS];
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)motion_3d_model.xyz_max_pos[Y_AXIS];
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)motion_3d_model.xyz_max_pos[Z_AXIS];
    machine_M2048_init_grbl();
  }


  #ifdef __cplusplus
} //extern "C" {
  #endif

}
