#ifndef GCODE_GLOBAL_PARAMS_H
#define GCODE_GLOBAL_PARAMS_H

#include <stdbool.h>
#include <stdint.h>
#include "threed_engine.h"
#include "Configuration.h"

namespace gcode
{
  const char axis_codes[MAX_NUM_AXIS] = {'X', 'Y', 'Z', 'E','B'};
  const float homing_feedrate[MAX_NUM_AXIS] = HOMING_FEEDRATE_X;

  extern volatile int fan_speed;                                                       /*!< 风扇速度 */
  extern volatile int16_t active_extruder;                                             /*!< 运动喷头id */
  extern volatile int16_t tmp_extruder;                                                /*!< 加热喷头id */
  extern volatile float feed_rate;                                                     /*!< 进料速度 */
  extern volatile int feed_multiply; //100->1 200->2                                   /*!< 进料速度百分比 */
  extern volatile int extruder_multiply; //100->1 200->2                               /*!< 移动速度百分比 */
  extern volatile float add_homing[XYZ_NUM_AXIS];                                      /*!< xyz零点偏移 */
  extern volatile float current_position[MAX_NUM_AXIS];                                /*!< 当前坐标 */
  extern volatile float destination[MAX_NUM_AXIS];                                     /*!< 目标坐标 */
  extern volatile uint8_t is_serial_cmd;                                               /*!< 是否为串口指令 */
  extern volatile bool relative_mode;                                                  /*!< 相对模式开关 */
  extern volatile uint32_t print_file_pos;                                             /*!< 打印文件位置 */
  extern volatile uint32_t stepper_inactive_time;                                      /*!< 电机解锁延时 */
  extern volatile uint32_t printing_material_length;                                   /*!< 打印耗材长度 */
  extern volatile uint32_t previous_xTaskGetTickCount_cmd;                             /*!< 指令延时时间 */
	extern volatile bool is_confirm_load_filament;                                       /*!< 中途换料确定键 */
	extern volatile uint8_t m600_filament_change_status;                                 /*!< 中途换料状态 */
	extern volatile bool m600_is_midway_change_material;                                 /*!< 是否处于中途换料状态 */

  // 设置风扇速度
  extern void set_fan_speed(const int value);
  // current_position
  extern float get_current_position(const volatile uint8_t axis);
  extern void set_current_position(const volatile uint8_t axis, const volatile float value);
  extern void plan_set_current_position(void);
  // destination_position
  extern float get_destination_position(const volatile uint8_t axis);
  extern void set_destination_position(const volatile uint8_t axis,const volatile float value);
  extern void plan_set_destination_position(void);
  // process_buffer_line
  extern void process_buffer_line(const volatile float (&position)[MAX_NUM_AXIS], const volatile float _feed_rate, const volatile int _feed_multiply);
  extern void process_buffer_line_normal(const volatile float (&position)[MAX_NUM_AXIS], const volatile float _feed_rate);
  extern void process_buffer_line_normal_4_curr(const volatile float feed_rate);
  extern void process_buffer_line_normal_4_dest(const volatile float feed_rate);
  // 同步坐标
  extern void plan_st_synchronize(void);
  // 指令延时管理
  extern void manage_inactivity(void);
}

#endif // GCODE_GLOBAL_PARAMS_H


