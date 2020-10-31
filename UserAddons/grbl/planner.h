
#ifndef planner_h
#define planner_h

#include <stdbool.h>
#include "planner_running_status.h"

namespace sg_grbl
{

#ifdef __cplusplus
  extern "C" {
#endif

    void planner_init(void);                                                ///< 初始化电机规划系统
    void planner_set_position(const volatile float (&_current_position)[MAX_NUM_AXIS]);              ///< G92指令，设置xyzeb坐标
    void planner_set_axis_position(const volatile float value, const volatile int axis);
    void planner_buffer_line(volatile planner_running_status_t * const running_status);     ///< 移动指令坐标数据
    bool planner_blocks_queued(void);                                       ///< 返回队列是否有数据
    int planner_moves_planned(void);                                        ///< 返回当前运动队列有多少条数据
    bool is_planner_moves_planned_full(void);

    // 自动调平接口
    void plan_apply_rotation_xyz(volatile float &x, volatile float &y, volatile float &z);    ///< xyz变换矩阵
    void plan_buffer_line_get_xyz(volatile float &x, volatile float &y, volatile float &z);   ///< 获取自动调平变换坐标
    void plan_set_process_auto_bed_level_status(const volatile bool status);
    float plan_get_current_save_xyz(const volatile int axis);
    void plan_set_current_save_xyz(const volatile int axis, const volatile float value);
    void plan_set_bed_level_position(const volatile float &left_probe_position, const volatile float &right_probe_position, const volatile float &front_probe_position,
                                     const volatile float &back_probe_position, const volatile float &middle_probe_position);
    void plan_set_bed_level_equation(void);
    void plan_bed_level_matrix_set_identity(void);

#ifdef __cplusplus
  } //extern "C" {
#endif

}

#endif





