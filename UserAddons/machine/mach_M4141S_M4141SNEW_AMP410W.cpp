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

  static void machine_M4141S_init_grbl()
  {
    float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_M41S;
    float buf5[] = DEFAULT_MAX_FEEDRATE_M41S;
    long buf6[] = DEFAULT_MAX_ACCELERATION_M41S;
    (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
    (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
    (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
  }

  void machine_M4141S_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)motion_3d_model.xyz_max_pos[X_AXIS];
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)motion_3d_model.xyz_max_pos[Y_AXIS];
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)motion_3d_model.xyz_max_pos[Z_AXIS];
    motion_3d_model.enable_invert_dir[X_AXIS] = false; //4141S机型的X轴方向不一样
    motion_3d_model.enable_invert_dir[Y_AXIS] = false; //4141S机型的Y轴方向不一样（2017525根据梁海衡M4141s设置的Y轴方向，与旧M4141S相反）
    motion_3d_model.enable_invert_dir[Z_AXIS] = false; //4141S机型的Z轴方向不一样
    machine_M4141S_init_grbl();
  }

  static void machine_M4141S_NEW_init_grbl()
  {
    float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_M41S_NEW;
    float buf5[] = DEFAULT_MAX_FEEDRATE_M41S_NEW;
    long buf6[] = DEFAULT_MAX_ACCELERATION_M41S_NEW;
    (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
    (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
    (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
  }

  void machine_M4141S_NEW_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)motion_3d_model.xyz_max_pos[X_AXIS];
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)motion_3d_model.xyz_max_pos[Y_AXIS];
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)motion_3d_model.xyz_max_pos[Z_AXIS];
    motion_3d_model.enable_invert_dir[X_AXIS] = false; //4141S机型的X轴方向不一样
    motion_3d_model.enable_invert_dir[Y_AXIS] = false; //4141S机型的Y轴方向不一样（2017525根据梁海衡M4141s设置的Y轴方向，与旧M4141S相反）
    motion_3d_model.enable_invert_dir[Z_AXIS] = false; //4141S机型的Z轴方向不一样
    machine_M4141S_NEW_init_grbl();
  }

  static void machine_AMP410W_init_grbl()
  {
    float buf4[] = DEFAULT_AXIS_STEPS_PER_UNIT_AMP410W;
    float buf5[] = DEFAULT_MAX_FEEDRATE_AMP410W;
    long buf6[] = DEFAULT_MAX_ACCELERATION_AMP410W;
    (void)memcpy(axis_steps_per_unit_buf, buf4, sizeof(buf4));
    (void)memcpy(max_feedrate_buf, buf5, sizeof(buf5));
    (void)memcpy(max_acc_units_per_sq_second_buf, buf6, sizeof(buf6));
  }

  void machine_AMP410W_init()
  {
    motion_3d_model.xyz_move_max_pos[X_AXIS] = (int)motion_3d_model.xyz_max_pos[X_AXIS];
    motion_3d_model.xyz_move_max_pos[Y_AXIS] = (int)motion_3d_model.xyz_max_pos[Y_AXIS];
    motion_3d_model.xyz_move_max_pos[Z_AXIS] = (int)motion_3d_model.xyz_max_pos[Z_AXIS];
    motion_3d_model.enable_invert_dir[X_AXIS] = false; //4141S机型的X轴方向不一样
    motion_3d_model.enable_invert_dir[Y_AXIS] = false; //4141S机型的Y轴方向不一样（2017525根据梁海衡M4141s设置的Y轴方向，与旧M4141S相反）
    motion_3d_model.enable_invert_dir[Z_AXIS] = false; //4141S机型的Z轴方向不一样
    //旧的41S机器使用万物打印的机型
    // motion_3d_model.enable_invert_dir[X_AXIS]=false;  //4141S机型的X轴方向不一样
    // motion_3d_model.enable_invert_dir[Y_AXIS]=true;   //
    // motion_3d_model.enable_invert_dir[Z_AXIS]=false;  //4141S机型的Z轴方向不一样
    machine_AMP410W_init_grbl();
  }



  #ifdef __cplusplus
} //extern "C" {
  #endif

}
