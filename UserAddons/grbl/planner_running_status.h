#ifndef PLANNER_RUNNING_STATUS
#define PLANNER_RUNNING_STATUS

#include <stdio.h>
#include <string.h>
#include "threed_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

  // 运行状态
  typedef struct
  {
    float axis_position[MAX_NUM_AXIS];  // 坐标位置
    float bed_temp;                     // 热床温度
    float extruder0_temp;               // 喷嘴温度
    float feed_rate;                    // 进料速度
    unsigned long sd_position;          // 文件位置
    int extruder_multiply;              // 移动速度百分比
    int feed_multiply;                  // 进料百分比
    int fan_speed;                      // 风扇速度
    unsigned char extruder;             // 喷头ID
    unsigned char is_serial;            // 是否串口指令
  } planner_running_status_t;

  __inline void copy_run_status_to_other(volatile planner_running_status_t * const source, volatile planner_running_status_t * const target)
  {
    for(int i = 0; i < MAX_NUM_AXIS; i++)
    {
      target->axis_position[i] = source->axis_position[i];
    }
    target->bed_temp = source->bed_temp;
    target->extruder0_temp = source->extruder0_temp;
    target->feed_rate = source->feed_rate;
    target->sd_position = source->sd_position;
    target->extruder_multiply = source->extruder_multiply;
    target->feed_multiply = source->feed_multiply;
    target->fan_speed = source->fan_speed;
    target->extruder = source->extruder;
    target->is_serial = source->is_serial;
  }

  extern void reset_run_status(volatile planner_running_status_t * const source);

#ifdef __cplusplus
} // extern "C" {
#endif


#endif // PLANNER_RUNNING_STATUS

