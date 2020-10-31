#ifndef CONFIG_MOTION_3D_H
#define CONFIG_MOTION_3D_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>

  typedef struct
  {
    int16_t axis_num;                              // 機型軸數
    unsigned char step;                            // 機型細分數 1:16;2:32;
    unsigned char enable_poweroff_up_down_min_min; // 是否開啓上下共限位
    unsigned char enable_board_test;               // 是否開啓大板測試
    unsigned char enable_check_door_open;          // 是否有门检测
    unsigned char disable_z_max_limit;             // 是否有Z轴下限位开关
    unsigned char updown_g28_first_time;           // 上下限位公用限位開關，執行G28指令判斷Z零點位置
    unsigned char is_open_infrared_z_min_check;
    bool is_cal_bed_platform;
    float extrude_min_temp;
  } motion_3d_t;

  typedef struct
  {
    float xyz_max_pos[3];                // XYZ最大位置
    float xyz_min_pos[3];                // XYZ最小位置
    float xyz_home_pos[3];               // XYZ零點位置
    float xyz_max_length[3];             // XYZ最大長度
    float xyz_home_retract_mm[3];        // XYZ歸零回抽距離mm
    float xyz_move_max_pos[3];           // XYZ最大移动位置
    float extrude_maxlength;             // prevent extrusion of very large distances.
    float z_max_pos_origin;              // 保存机型默认Z最大点，用于校准Z高度
    unsigned char enable_invert_dir[6];  // 是否反轉軸方向
    signed char xyz_home_dir[3];         // XYZ方向
  } motion_3d_model_t;

#ifdef __cplusplus
} // extern "C" {
#endif
#endif


