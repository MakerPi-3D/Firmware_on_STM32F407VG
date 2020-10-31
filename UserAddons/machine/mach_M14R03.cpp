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

  static void machine_M14R03_init_grbl()
  {
    float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_M14R03;
    float buf5[] = DEFAULT_MAX_FEEDRATE_M14R03;
    long buf6[] = DEFAULT_MAX_ACCELERATION_M14R03;
    (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
    (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
    (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
  }

  void machine_M14R03_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)(motion_3d_model.xyz_max_pos[X_AXIS] - 8);
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)(motion_3d_model.xyz_max_pos[Y_AXIS] - 8);
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)(motion_3d_model.xyz_max_pos[Z_AXIS] - 8);
    //没有断电续功能，因为ZMAX已被用于门检测 //没有混色功能，因为5V小风扇接口已被用于警示灯
    t_sys_data_current.enable_color_mixing = 0; //强制关闭混色功能
    motion_3d.axis_num = 4;
    t_custom_services.enable_warning_light = true; //有警示灯
    motion_3d.enable_check_door_open = true; //有门检测
    machine_M14R03_init_grbl();
  }



  #ifdef __cplusplus
} //extern "C" {
  #endif

}
