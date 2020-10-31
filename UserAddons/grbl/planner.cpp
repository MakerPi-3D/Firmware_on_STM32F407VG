
/* The ring buffer implementation gleaned from the wiring_serial library by David A. Mellis. */

/*
 Reasoning behind the mathematics in this module (in the key of 'Mathematica'):

 s == speed, a == acceleration, t == time, d == distance

 Basic definitions:

 Speed[s_, a_, t_] := s + (a*t)
 Travel[s_, a_, t_] := Integrate[Speed[s, a, t], t]

 Distance to reach a specific speed with a constant acceleration:

 Solve[{Speed[s, a, t] == m, Travel[s, a, t] == d}, d, t]
 d -> (m^2 - s^2)/(2 a) --> estimate_acceleration_distance()

 Speed after a given distance of travel with constant acceleration:

 Solve[{Speed[s, a, t] == m, Travel[s, a, t] == d}, m, t]
 m -> Sqrt[2 a d + s^2]

 DestinationSpeed[s_, a_, d_] := Sqrt[2 a d + s^2]

 When to start braking (di) to reach a specified destionation speed (s2) after accelerating
 from initial speed s1 without ever stopping at a plateau:

 Solve[{DestinationSpeed[s1, a, di] == DestinationSpeed[s2, a, d - di]}, di]
 di -> (2 a d - s1^2 + s2^2)/(4 a) --> intersection_distance()

 IntersectionDistance[s1_, s2_, a_, d_] := (2 a d - s1^2 + s2^2)/(4 a)
 */
#include "planner.h"
#include "planner_running_status.h"
#include "stepper.h"
#include "grbl_acc_dec.h"
#include "threed_engine.h"
#include "ConfigurationStore.h"
#include "vector_3.h"
#include "Configuration.h"
#include "sys_function.h"
#include "config_model_tables.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "sg_util.h"
#include "globalvariables.h"

namespace sg_grbl
{

  #ifdef __cplusplus
  extern "C" {
  #endif

  //===========================================================================
  //=============================public variables ============================
  //===========================================================================

  //===========================================================================
  //=================semi-private variables, used in inline  functions    =====
  //===========================================================================

  //===========================================================================
  //=============================private variables ============================
  //===========================================================================
  // The current position of the tool in absolute steps
  static volatile long plan_position[MAX_NUM_AXIS];   //rescaled from extern when axis_steps_per_unit are changed by gcode
  static volatile long plan_target[MAX_NUM_AXIS];
  static volatile float previous_speed[MAX_NUM_AXIS]; // Speed of previous path line segment
  static volatile float previous_nominal_speed; // Nominal speed of previous path line segment

  //////////////////////////////自动调平变量//////////////////////////////////

  // 平台矩阵数组
  static matrix_3x3 plan_bed_level_matrix ;                     ///< 热床偏移矩阵
  static matrix_3x3 plan_bed_level_matrix_reversed ;            ///< 热床偏移逆矩阵
  // XYZ坐标存储变量
  float current_save_xyz[XYZ_NUM_AXIS] = {0.0F};         ///< 保存当前XYZ位置，校准前
  float current_xyz_compensate[XYZ_NUM_AXIS] = {0.0F};   ///< 保存当前XYZ位置补偿
  float current_xyz_real[XYZ_NUM_AXIS] = {0.0F};         ///< 保存当前XYZ真实位置，校准后

  static bool is_process_auto_bed_leveling = false;             ///< 执行平台校准开关
  // 平台触点信息
  static float left_probe_bed_position = 0.0F;                  ///< 探头接触热床左边位置
  static float right_probe_bed_position = 0.0F;                 ///< 探头接触热床右边位置
  static float front_probe_bed_position = 0.0F;                 ///< 探头接触热床前面位置
  static float back_probe_bed_position = 0.0F;                  ///< 探头接触热床后面位置
  //  static float middle_probe_bed_position = 0.0F;              ///< 探头接触热床中间位置

  //===========================================================================
  //=============================private function =============================
  //===========================================================================

  // Returns the index of the next block in the ring buffer
  // NOTE: Removed modulo (%) operator, which uses an expensive divide and multiplication.
  static int8_t next_block_index(volatile int8_t block_index)
  {
    ++block_index;

    if (block_index == BLOCK_BUFFER_SIZE)
    {
      block_index = 0;
    }

    return (block_index);
  }

  // Returns the index of the previous block in the ring buffer
  static int8_t prev_block_index(volatile int8_t block_index)
  {
    if (block_index == 0)
    {
      block_index = BLOCK_BUFFER_SIZE;
    }

    block_index--;
    return (block_index);
  }

  // Calculates trapezoid parameters so that the entry- and exit-speed is compensated by the provided factors.
  // The kernel called by planner_recalculate() when scanning the plan from last to first entry.
  //void planner_reverse_pass_kernel(block_t *previous, block_t *current, block_t *next)
  static void planner_reverse_pass_kernel(volatile block_t *const current, const volatile block_t *const next)
  {
    if (!current)
    {
      return;
    }

    if (next)
    {
      // If entry speed is already at the maximum entry speed, no need to recheck. Block is cruising.
      // If not, block in state of acceleration or deceleration. Reset entry speed to maximum and
      // check for maximum allowable speed reductions to ensure maximum possible planned speed.
      if (current->entry_speed != current->max_entry_speed)
      {
        // If nominal length true, max junction speed is guaranteed to be reached. Only compute
        // for max allowable speed if block is decelerating and nominal length is false.
        if ((!current->nominal_length_flag) && (current->max_entry_speed > next->entry_speed))
        {
          float max_allowable_speed_temp = trapezoid_or_s_curve_max_allowable_speed(current, -current->acceleration, next->entry_speed, current->millimeters);
          current->entry_speed = sg_util::min_p(current->max_entry_speed, max_allowable_speed_temp);
        }
        else
        {
          current->entry_speed = current->max_entry_speed;
        }

        current->recalculate_flag = true;
      }
    } // Skip last block. Already initialized and set for recalculation.
  }

  // planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This
  // implements the reverse pass.
  static void planner_reverse_pass(void)
  {
    uint8_t block_index = block_buffer_head;
    //Make a local copy of block_buffer_tail, because the interrupt can alter it
    sys_task_enter_critical();
    unsigned char tail = block_buffer_tail;
    sys_task_exit_critical();

    if ((((block_buffer_head - tail) + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1)) > 3)
    {
      block_index = (block_buffer_head - 3) & (BLOCK_BUFFER_SIZE - 1);
      volatile block_t *block[3] =
      {
        NULL, NULL, NULL
      };

      while (block_index != tail)
      {
        block_index = prev_block_index(block_index);
        block[2] = block[1];
        block[1] = block[0];
        block[0] = &block_buffer[block_index];
        planner_reverse_pass_kernel(block[1], block[2]);
      }
    }
  }

  // The kernel called by planner_recalculate() when scanning the plan from first to last entry.
  //void planner_forward_pass_kernel(block_t *previous, block_t *current, block_t *next)
  static void planner_forward_pass_kernel(const volatile block_t *const previous, volatile block_t *const current)
  {
    if (!previous)
    {
      return;
    }

    // If the previous block is an acceleration block, but it is not long enough to complete the
    // full speed change within the block, we need to adjust the entry speed accordingly. Entry
    // speeds have already been reset, maximized, and reverse planned by reverse planner.
    // If nominal length is true, max junction speed is guaranteed to be reached. No need to recheck.
    if (!previous->nominal_length_flag)
    {
      if (previous->entry_speed < current->entry_speed)
      {
        float max_allowable_speed_temp = trapezoid_or_s_curve_max_allowable_speed((block_t *)previous, -previous->acceleration, previous->entry_speed, previous->millimeters);
        float entry_speed = sg_util::min_p(current->entry_speed, max_allowable_speed_temp);

        // Check for junction speed change
        if (current->entry_speed != entry_speed)
        {
          current->entry_speed = (float)entry_speed;
          current->recalculate_flag = true;
        }
      }
    }
  }

  // planner_recalculate() needs to go over the current plan twice. Once in reverse and once forward. This
  // implements the forward pass.
  static void planner_forward_pass(void)
  {
    uint8_t block_index = block_buffer_tail;
    volatile block_t *block[3] =
    {
      NULL, NULL, NULL
    };

    while (block_index != block_buffer_head)
    {
      block[0] = block[1];
      block[1] = block[2];
      block[2] = &block_buffer[block_index];
      planner_forward_pass_kernel(block[0], block[1]);
      block_index = next_block_index(block_index);
    }

    planner_forward_pass_kernel(block[1], block[2]);
  }

  // Recalculates the trapezoid speed profiles for all blocks in the plan according to the
  // entry_factor for each junction. Must be called by planner_recalculate() after
  // updating the blocks.
  static void planner_recalculate_trapezoids(void)
  {
    INT8 block_index = (INT8)block_buffer_tail;
    volatile block_t *current;
    volatile block_t *next = NULL;

    while (block_index != block_buffer_head)
    {
      current = next;
      next = &block_buffer[block_index];

      if (current)
      {
        // Recalculate if current block entry or exit junction speed has changed.
        if (current->recalculate_flag || next->recalculate_flag)
        {
          // NOTE: Entry and exit factors always > 0 by all previous logic operations.
          trapezoid_or_s_curve_calculate_for_block(current, current->entry_speed / current->nominal_speed,
              next->entry_speed / current->nominal_speed); // float
          current->recalculate_flag = false; // Reset current only to ensure next trapezoid is computed
        }
      }

      block_index = next_block_index(block_index);
    }

    // Last/newest block in buffer. Exit speed is set with MINIMUM_PLANNER_SPEED. Always recalculated.
    if (next != NULL)
    {
      trapezoid_or_s_curve_calculate_for_block(next, next->entry_speed / next->nominal_speed, // float
          MINIMUM_PLANNER_SPEED / next->nominal_speed); // float
      next->recalculate_flag = false;
    }
  }

  // Recalculates the motion plan according to the following algorithm:
  //
  //   1. Go over every block in reverse order and calculate a junction speed reduction (i.e. block_t.entry_factor)
  //      so that:
  //     a. The junction jerk is within the set limit
  //     b. No speed reduction within one block requires faster deceleration than the one, true constant
  //        acceleration.
  //   2. Go over every block in chronological order and dial down junction speed reduction values if
  //     a. The speed increase within one block would require faster accelleration than the one, true
  //        constant acceleration.
  //
  // When these stages are complete all blocks have an entry_factor that will allow all speed changes to
  // be performed using only the one, true constant acceleration, and where no junction jerk is jerkier than
  // the set limit. Finally it will:
  //
  //   3. Recalculate trapezoids for all blocks.
  static void planner_recalculate(void)
  {
    planner_reverse_pass();
    planner_forward_pass();
    planner_recalculate_trapezoids();
  }

  // 保护喷嘴，喷嘴温度小于最小温度限制时，EB轴不运动
  static void plan_prevent_dangerous_extrude(const volatile long target, volatile long &position, const int axis, const float extruder_temp)
  {
    if (target != position)
    {
      if (extruder_temp < motion_3d.extrude_min_temp)
      {
        position = target; //behave as if the move really took place, but ignore E part
      }

      #ifdef PREVENT_LENGTHY_EXTRUDE

      if (labs(target - position) > (axis_steps_per_unit[axis]*motion_3d_model.extrude_maxlength)) // float
      {
        position = target; //behave as if the move really took place, but ignore E part
      }

      #endif
    }
  }

  static void plan_init_target(const volatile float (&plan_buffer_position)[MAX_NUM_AXIS], const volatile float extruder_temp)
  {
    // The target position of the tool in absolute steps
    // Calculate target position in absolute steps
    // this should be done after the wait, because otherwise a M92 code within the gcode disrupts this calculation somehow
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      plan_target[i] = lroundf(plan_buffer_position[i] * axis_steps_per_unit[i]); // float
    }

    #ifdef PREVENT_DANGEROUS_EXTRUDE

    if (!motion_3d.enable_board_test)
    {
      plan_prevent_dangerous_extrude(plan_target[E_AXIS], plan_position[E_AXIS], (int)E_AXIS, extruder_temp);

      if (1 == t_sys_data_current.enable_color_mixing)
      {
        plan_prevent_dangerous_extrude(plan_target[B_AXIS], plan_position[B_AXIS], (int)B_AXIS, extruder_temp);
      }
    }

    #endif
  }

  // 计算各轴需要移动位移
  static void plan_calc_millimeters(volatile float (&delta_mm)[MAX_NUM_AXIS], volatile block_t *const block, volatile float &millimeters, const volatile int extrudemultiply)
  {
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      #ifndef NEWCOREXY
      delta_mm[i] = (float)(plan_target[i] - plan_position[i]) / axis_steps_per_unit[i]; // float
      #else

      if (X_AXIS == i)
      {
        delta_mm[X_AXIS] = (plan_target[X_AXIS] - plan_position[X_AXIS])  / axis_steps_per_unit[X_AXIS] - (plan_target[Y_AXIS] - plan_position[Y_AXIS])  / axis_steps_per_unit[Y_AXIS];
      }
      else if (Y_AXIS == i)
      {
        delta_mm[Y_AXIS] = (plan_target[Y_AXIS] - plan_position[Y_AXIS]) / axis_steps_per_unit[Y_AXIS];
      }
      else
      {
        delta_mm[i] = (float)(plan_target[i] - plan_position[i]) / axis_steps_per_unit[i];
      }

      #endif

      if ((E_AXIS == i) || (B_AXIS == i))
      {
        delta_mm[i] = (delta_mm[i] * extrudemultiply) / 100.0F; // float
      }
    }

    // Prepare to set up new block
    if ((block->steps_axis[X_AXIS] <= dropsegments) && (block->steps_axis[Y_AXIS] <= dropsegments) && (block->steps_axis[Z_AXIS] <= dropsegments))
    {
      millimeters = fabs(delta_mm[E_AXIS]) + (t_sys_data_current.enable_color_mixing ? fabs(delta_mm[B_AXIS]) : 0); // float
    }
    else
    {
      millimeters = sqrt(((delta_mm[X_AXIS] * delta_mm[X_AXIS]) + (delta_mm[Y_AXIS] * delta_mm[Y_AXIS])) + (delta_mm[Z_AXIS] * delta_mm[Z_AXIS])); // float
    }
  }

  static void plan_calc_direction_bits(volatile unsigned char &direction_bits)
  {
    direction_bits = 0;

    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      #ifndef NEWCOREXY

      if (plan_target[i] < plan_position[i])
      {
        direction_bits |= (1 << (int)i);
      }

      #else

      if (X_AXIS == i)
      {
        if ((plan_target[Y_AXIS] - plan_position[Y_AXIS]) - (plan_target[X_AXIS] - plan_position[X_AXIS])  > 0)
        {
          direction_bits |= (1 << X_AXIS);
        }
      }
      else if (Y_AXIS == i)
      {
        if ((plan_target[Y_AXIS] - plan_position[Y_AXIS]) < 0)
        {
          direction_bits |= (1 << Y_AXIS);
        }
      }
      else
      {
        if (plan_target[i] < plan_position[i])
        {
          direction_bits |= (1 << (int)i);
        }
      }

      #endif
    }
  }

  static float plan_calc_safe_speed(const volatile float (&current_speed)[MAX_NUM_AXIS])
  {
    float safe_speed = max_xy_jerk / 2; // float

    if (fabs(current_speed[Z_AXIS]) > (max_z_jerk / 2)) // float
    {
      safe_speed = sg_util::min_p(safe_speed, max_z_jerk / 2); // float
    }

    if (fabs(current_speed[E_AXIS]) > (max_e_jerk / 2)) // float
    {
      safe_speed = sg_util::min_p(safe_speed, max_e_jerk / 2); // float
    }

    if (1 == t_sys_data_current.enable_color_mixing)
    {
      if (fabs(current_speed[B_AXIS]) > (max_b_jerk / 2)) // float
      {
        safe_speed = sg_util::min_p(safe_speed, max_b_jerk / 2); // float
      }
    }

    return safe_speed;
  }

  static void plan_calc_safe_speed(const volatile float (&current_speed)[MAX_NUM_AXIS], const volatile int moves_queued, const volatile float safe_speed, const volatile float nominal_speed, volatile float &max_entry_speed)
  {
    float vmax_junction = safe_speed;
    float vmax_junction_factor = 1.0f;

    if ((moves_queued > 1) && (previous_nominal_speed > 0.0001f))
    {
      float jerk = sqrt(pow((current_speed[X_AXIS] - previous_speed[X_AXIS]), 2) + pow((current_speed[Y_AXIS] - previous_speed[Y_AXIS]), 2)); // float
      //    if((fabs(previous_speed[X_AXIS]) > 0.0001) || (fabs(previous_speed[Y_AXIS]) > 0.0001)) {
      vmax_junction = nominal_speed;

      //    }
      if (jerk > max_xy_jerk)
      {
        vmax_junction_factor = (max_xy_jerk / jerk); // float
      }

      if (fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS]) > max_z_jerk) // float
      {
        float ZSpeedDiffVal = fabs(current_speed[Z_AXIS] - previous_speed[Z_AXIS]); // float
        vmax_junction_factor = sg_util::min_p(vmax_junction_factor, (max_z_jerk / ZSpeedDiffVal)); // float
      }

      if (fabs(current_speed[E_AXIS] - previous_speed[E_AXIS]) > max_e_jerk) // float
      {
        const float ESpeedDiffVal = fabs(current_speed[E_AXIS] - previous_speed[E_AXIS]); // float
        vmax_junction_factor = sg_util::min_p(vmax_junction_factor, (max_e_jerk / ESpeedDiffVal)); // float
      }

      if (1 == t_sys_data_current.enable_color_mixing)
      {
        if (fabs(current_speed[B_AXIS] - previous_speed[B_AXIS]) > max_b_jerk) // float
        {
          float BSpeedDiffVal = fabs(current_speed[B_AXIS] - previous_speed[B_AXIS]); // float
          vmax_junction_factor = sg_util::min_p(vmax_junction_factor, (max_b_jerk / BSpeedDiffVal)); // float
        }
      }

      vmax_junction = sg_util::min_p(previous_nominal_speed, vmax_junction * vmax_junction_factor); // Limit speed to max previous speed
    }

    max_entry_speed = vmax_junction;
  }

  static void plan_calc_acceleration(volatile block_t *const block)
  {
    // 计算加速度（step/s2）
    // 如果XYZ步进数都为0，加速度取retract_acceleration；不为0时，取加速度最小轴
    // steps_per_mm = step_event_count / millimeters，单位（step/mm）
    float steps_per_mm = block->step_event_count / block->millimeters;

    if ((block->steps_axis[X_AXIS] == 0) && (block->steps_axis[Y_AXIS] == 0) && (block->steps_axis[Z_AXIS] == 0))
    {
      block->acceleration_st = retract_acceleration * steps_per_mm; // convert to: acceleration steps/sec^2
    }
    else
    {
      block->acceleration_st = acceleration * steps_per_mm; // convert to: acceleration steps/sec^2

      // Limit acceleration per axis
      for (int i = 0; i < motion_3d.axis_num; ++i)
      {
        if (block->steps_axis[i] && axis_steps_per_sqr_second[i] < block->acceleration_st)
        {
          const float comp = (float)axis_steps_per_sqr_second[i] * (float)block->step_event_count;

          if (block->acceleration_st * block->steps_axis[i] > comp)
          {
            block->acceleration_st = comp / (float)block->steps_axis[i];
          }
        }
      }
    }

    // 重置加速度
    #ifdef S_CURVE_ALGORITHM
    // 默认S型加减速
    block->acceleration_type = 1; //s-curve

    // 距离小于8mm,改用梯形算法，避免频繁加减速，需要考虑微小线段问题
    if (block->millimeters <= 20)
      block->acceleration_type = 0; //trazoid

    // Z轴大于0启动梯形
    if (block->steps_axis[Z_AXIS] > 0)
      block->acceleration_type = 0; //trazoid

    //    // 存在Z轴，用T型加减速
    //    if( delta_mm[Z_AXIS] > 0 )
    //      block->acceleration_type=0; //trazoid

    // S型加减速，设置加速度为原来的四分之一
    if (block->acceleration_type)
      block->acceleration_st /= 4.0F;

    #endif // #ifdef S_CURVE_ALGORITHM
    block->acceleration = block->acceleration_st / steps_per_mm; // float
    block->acceleration_rate = (long)(block->acceleration_st * (16777216.0f / ((float)F_CPU / 8.0f))); // float
  }

  // 计算各轴步进数及总步进数
  // 初始化各轴给进步进数（step），总步进数step_event_count取最大步进数轴
  static void plan_init_steps(volatile block_t *const block, const volatile int extrudemultiply)
  {
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      #ifndef NEWCOREXY
      block->steps_axis[i] = labs(plan_target[i] - plan_position[i]);
      #else

      // corexy planning
      // these equations follow the form of the dA and dB equations on http://www.corexy.com/theory.html
      if (X_AXIS == i)
      {
        block->steps_axis[X_AXIS] = labs((plan_target[X_AXIS] - plan_position[X_AXIS])  - (plan_target[Y_AXIS] - plan_position[Y_AXIS]));
      }
      else if (Y_AXIS == i)
      {
        block->steps_axis[Y_AXIS] = labs((plan_target[Y_AXIS] - plan_position[Y_AXIS]));
      }
      else
      {
        block->steps_axis[i] = labs(plan_target[i] - plan_position[i]);
      }

      #endif

      if ((E_AXIS == i) || (B_AXIS == i))
      {
        block->steps_axis[i] = (block->steps_axis[i] * extrudemultiply) / 100.0f; // float
      }

      if (0 == i)
      {
        block->step_event_count = block->steps_axis[i];
      }
      else
      {
        block->step_event_count = (unsigned long)sg_util::max_p(block->step_event_count, (unsigned long)block->steps_axis[i]);
      }
    }
  }

  static void plan_calc_nominal_speed_and_rate(volatile block_t *const block, volatile float (&current_speed)[MAX_NUM_AXIS], const volatile float (&delta_mm)[MAX_NUM_AXIS], const volatile int moves_queued, const volatile float feed_rate)
  {
    // Calculate speed in mm/second for each axis. No divide by zero due to previous checks.
    float inverse_millimeters = 1.0f / block->millimeters; // Inverse millimeters to remove multiple divides
    float inverse_second = feed_rate * inverse_millimeters;
    // slow down when de buffer starts to empty, rather than wait at the corner for a buffer refill
    #ifdef SLOWDOWN
    //  segment time im micro seconds
    unsigned long segment_time = (unsigned long)lroundf(1000000.0f / inverse_second);

    if ((moves_queued > 1) && (moves_queued < (BLOCK_BUFFER_SIZE * 0.5f))) // float
    {
      if (segment_time < minsegmenttime)
      {
        // buffer is draining, add extra time.  The amount of time added increases if the buffer is still emptied more.
        inverse_second = 1000000.0f / (segment_time + (unsigned long)lroundf((2.0f * (minsegmenttime - segment_time)) / (t_sys.is_planner_slow_down ? t_sys.serial_moves_queued : moves_queued)));
      }
    }

    #endif // END OF SLOW DOWN SECTION
    block->nominal_speed = block->millimeters * inverse_second; // (mm/sec) Always > 0
    block->nominal_rate = (unsigned long)ceil(block->step_event_count * inverse_second); // (step/sec) Always > 0
    // Calculate and limit speed in mm/sec for each axis
    float speed_factor = 1.0f; //factor <=1 do decrease speed

    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      current_speed[i] = delta_mm[i] * inverse_second; // float

      if (fabs(current_speed[i]) > max_feedrate[i])
      {
        const float fabs_current_speed = fabs(current_speed[i]);
        speed_factor = sg_util::min_p(speed_factor, max_feedrate[i] / fabs_current_speed); // float
      }
    }

    // Correct the speed
    if (speed_factor < 1.0f)
    {
      for (unsigned char i = 0; i < motion_3d.axis_num; ++i)
      {
        current_speed[i] *= speed_factor; // float
      }

      block->nominal_speed *= speed_factor; // float
      block->nominal_rate = (unsigned long)(block->nominal_rate * speed_factor); // float
    }
  }

  static void plan_init_feed_rate(volatile block_t *const block, volatile float &feed_rate)
  {
    const float max_travel_feed_rate = 300.0f;
    const float max_travel_feed_rate_x = 80.0f;

    if (block->steps_axis[E_AXIS] == 0)
    {
      if (feed_rate < mintravelfeedrate)
      {
        feed_rate = mintravelfeedrate;
      }

      if (M4141S_NEW == t_sys_data_current.model_id)
      {
        if (feed_rate > 80.0F)
        {
          feed_rate = 80.0F;
        }
      }

      if (1 == t_sys_data_current.enable_color_mixing)
      {
        if (feed_rate > max_travel_feed_rate_x)
        {
          feed_rate = max_travel_feed_rate_x;
        }
      }
      else
      {
        if (9 == t_sys_data_current.model_id) //M14R03机型id==9
        {
          if (feed_rate > (max_travel_feed_rate - 220.0f)) // float
          {
            feed_rate = max_travel_feed_rate - 220.0f; // float
          }
        }
        else if (feed_rate > max_travel_feed_rate)
        {
          feed_rate = max_travel_feed_rate;
        }
        else
        {
          // return
        }
      }
    }
    else
    {
      if (feed_rate < minimumfeedrate)
      {
        feed_rate = minimumfeedrate;
      }
    }
  }

  static void plan_set_position_get_xyz(const volatile float (&_current_position)[MAX_NUM_AXIS], volatile long (&position_xyz)[MAX_NUM_AXIS])
  {
    if (1 == t_sys_data_current.enable_bed_level)
    {
      if (is_process_auto_bed_leveling)
      {
        float auto_bed_level_position[XYZ_NUM_AXIS] = {0.0f, 0.0f, 0.0f};

        for (int i = 0; i < XYZ_NUM_AXIS; ++i)
        {
          auto_bed_level_position[i] = _current_position[i];
          current_save_xyz[i] = _current_position[i]; // 保存为当前位置
        }

        plan_apply_rotation_xyz(auto_bed_level_position[X_AXIS], auto_bed_level_position[Y_AXIS], auto_bed_level_position[Z_AXIS]);

        for (int i = 0; i < 2; ++i)
        {
          if (auto_bed_level_position[i] < 0)
          {
            current_xyz_compensate[i] = -auto_bed_level_position[i];
            auto_bed_level_position[i] = 0;
          }
          else
          {
            current_xyz_compensate[i] = 0.0f;
          }
        }

        for (int i = 0; i < 3; ++i)
        {
          current_xyz_real[i] = auto_bed_level_position[i];
          position_xyz[i] = ceil(auto_bed_level_position[i] * axis_steps_per_unit[i]); // float
        }
      }
      else
      {
        for (int i = 0; i < XYZ_NUM_AXIS; ++i)
        {
          current_save_xyz[i] = _current_position[i]; // 保存为当前位置
          current_xyz_real[i] = _current_position[i];
          current_xyz_compensate[i] = 0.0f;
        }
      }
    }
    else
    {
      for (int i = 0; i < XYZ_NUM_AXIS; ++i)
      {
        current_save_xyz[i] = _current_position[i]; // 保存为当前位置
      }
    }
  }


  //===========================================================================
  //============================= public function =============================
  //===========================================================================

  void planner_init(void)
  {
    block_buffer_head = 0;
    block_buffer_tail = 0;

    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      plan_position[i] = 0;// clear plan_position
      plan_target[i] = 0;// clear plan_position
      previous_speed[i] = 0.0f;
    }

    previous_nominal_speed = 0.0f;
  }

  void planner_buffer_line(volatile planner_running_status_t *const running_status)
  {
    int next_buffer_head;
    float plan_buffer_position_tmp[MAX_NUM_AXIS] ;

    for (int i = 0; i < MAX_NUM_AXIS; i++)
    {
      plan_buffer_position_tmp[i] = running_status->axis_position[i];
    }

    plan_buffer_line_get_xyz(plan_buffer_position_tmp[X_AXIS], plan_buffer_position_tmp[Y_AXIS], plan_buffer_position_tmp[Z_AXIS]);
    // Calculate the buffer head after we push this byte
    next_buffer_head = next_block_index(block_buffer_head);

    // If the buffer is full: good! That means we are well ahead of the robot.
    // Rest here until there is room in the buffer.
    while (block_buffer_tail == next_buffer_head)
    {
      (void)sys_os_delay(10);
    }

    // 初始化目标位置
    plan_init_target(plan_buffer_position_tmp, running_status->extruder0_temp);

    // 初始化运行状态值，断电续打
    for (int i = 0; i < MAX_NUM_AXIS; i++)
    {
      running_status->axis_position[i] = plan_buffer_position_tmp[i];
    }

    copy_run_status_to_other(running_status, &runningStatus[block_buffer_head]);
    // Prepare to set up new block
    volatile block_t *const block = &block_buffer[block_buffer_head];
    // default non-h-bot planning
    plan_init_steps(block, running_status->extruder_multiply);

    // 如果总步进数脉冲小于5，跳过当前指令
    // Bail if this is a zero-length block
    if ((block->step_event_count <= dropsegments))
      return;

    // Mark block as not busy (Not executed by the stepper interrupt)
    block->busy = false;
    block->active_extruder = running_status->extruder;
    // Compute direction bits for this block
    plan_calc_direction_bits(block->direction_bits);

    //enable active axes
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      #ifndef NEWCOREXY

      if (block->steps_axis[i] != 0)
      {
        st_enable_axis(i, true);
      }

      #else

      if (X_AXIS == i || Y_AXIS == i)
      {
        if (block->steps_axis[i] != 0)
        {
          st_enable_axis(X_AXIS, true);
          st_enable_axis(Y_AXIS, true);
        }
      }
      else
      {
        if (block->steps_axis[i] != 0)
          st_enable_axis(i, true);
      }

      #endif

      if ((!t_sys_data_current.enable_color_mixing) && (E_AXIS == i))
      {
        break;
      }
    }

    plan_init_feed_rate(block, running_status->feed_rate);
    float delta_mm[MAX_NUM_AXIS];
    // 初始化各轴移动位移
    plan_calc_millimeters(delta_mm, block, block->millimeters, running_status->extruder_multiply);
    float current_speed[MAX_NUM_AXIS];
    int moves_queued = ((block_buffer_head - block_buffer_tail) + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
    plan_calc_nominal_speed_and_rate(block, current_speed, delta_mm, moves_queued, running_status->feed_rate);
    // Start with a safe speed
    float safe_speed = plan_calc_safe_speed(current_speed);
    safe_speed = sg_util::min_p(safe_speed, block->nominal_speed);
    plan_calc_safe_speed(current_speed, moves_queued, safe_speed, block->nominal_speed, block->max_entry_speed);
    plan_calc_acceleration(block);
    // Initialize block entry speed. Compute based on deceleration to user-defined MINIMUM_PLANNER_SPEED.
    float v_allowable = trapezoid_or_s_curve_max_allowable_speed(block, -block->acceleration, MINIMUM_PLANNER_SPEED, block->millimeters);
    block->entry_speed = (float)sg_util::min_p(block->max_entry_speed, v_allowable);

    // Initialize planner efficiency flags
    // Set flag if block will always reach maximum junction speed regardless of entry/exit speeds.
    // If a block can de/ac-celerate from nominal speed to zero within the length of the block, then
    // the current block and next block junction speeds are guaranteed to always be at their maximum
    // junction speeds in deceleration and acceleration, respectively. This is due to how the current
    // block nominal speed limits both the current and next maximum junction speeds. Hence, in both
    // the reverse and forward planners, the corresponding block junction speed will always be at the
    // the maximum junction speed and may always be ignored for any speed reduction checks.
    if (block->nominal_speed <= v_allowable)
    {
      block->nominal_length_flag = true;
    }
    else
    {
      block->nominal_length_flag = false;
    }

    block->recalculate_flag = true; // Always calculate trapezoid for new block
    // Update previous path unit_vector and nominal speed
    (void)memcpy((float*)previous_speed, current_speed, sizeof(previous_speed)); // previous_speed[] = current_speed[]
    previous_nominal_speed = block->nominal_speed;
    //calc_adv_rate();
    trapezoid_or_s_curve_calculate_for_block(block, block->entry_speed / block->nominal_speed,
        safe_speed / block->nominal_speed); // float
    // Move buffer head
    block_buffer_head = (unsigned char)next_buffer_head;

    // Update plan_position
    for (int i = 0; i < MAX_NUM_AXIS; i++)
    {
      plan_position[i] = plan_target[i];
    }

    // 前瞻规划
    planner_recalculate();
    st_wake_up();
  }

  void planner_set_position(const volatile float (&_current_position)[MAX_NUM_AXIS])
  {
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      if (1 == t_sys_data_current.enable_color_mixing)
      {
        if ((i == (int)B_AXIS) && (_current_position[(int)B_AXIS] == -1))
        {
          continue;
        }
      }

      plan_position[i] = lroundf(_current_position[i] * axis_steps_per_unit[i]); // float
    }

    plan_set_position_get_xyz(_current_position, plan_position);
    st_set_position(plan_position);
    previous_nominal_speed = 0.0; // Resets planner junction speeds. Assumes start from rest.

    for (int i = 0; i < motion_3d.axis_num; ++i)
      previous_speed[i] = 0.0f;
  }

  void planner_set_axis_position(const volatile float value, const volatile int axis)
  {
    plan_position[axis] = lroundf(value * axis_steps_per_unit[axis]); // float
    st_set_position_axis(plan_position[axis], axis);
  }

  void plan_apply_rotation_xyz(volatile float &x, volatile float &y, volatile float &z)
  {
    vector_3 vector = vector_3(x, y, z);
    vector.apply_rotation(plan_bed_level_matrix);
    x = vector.x;
    y = vector.y;
    z = vector.z;
  }

  void plan_bed_level_matrix_set_identity(void)
  {
    plan_bed_level_matrix.set_to_identity();
    plan_bed_level_matrix_reversed.set_to_identity();
  }

  void plan_set_bed_level_position(const volatile float &left_probe_position, const volatile float &right_probe_position, const volatile float &front_probe_position,
                                   const volatile float &back_probe_position, const volatile float &middle_probe_position)
  {
    left_probe_bed_position = left_probe_position;
    right_probe_bed_position = right_probe_position;
    back_probe_bed_position = back_probe_position;
    front_probe_bed_position = front_probe_position;
    //    middle_probe_bed_position = middle_probe_position;
  }

  void plan_set_bed_level_equation(void)
  {
    if (sg_util::is_float_data_equivalent(t_sys_data_current.bed_level_z_at_left_front, 0.0f) &&
        sg_util::is_float_data_equivalent(t_sys_data_current.bed_level_z_at_left_back, 0.0f) &&
        sg_util::is_float_data_equivalent(t_sys_data_current.bed_level_z_at_right_front, 0.0f) &&
        sg_util::is_float_data_equivalent(t_sys_data_current.bed_level_z_at_right_back, 0.0f) &&
        sg_util::is_float_data_equivalent(t_sys_data_current.bed_level_z_at_middle, 0.0f))
    {
      return;
    }

    // 1丝=0.01毫米 1毫米=10丝米=100丝(忽米)
    // 过滤小于5个丝误差
    if (fabs(t_sys_data_current.bed_level_z_at_left_front) < 0.05f)
    {
      t_sys_data_current.bed_level_z_at_left_front = 0.0f;
    }

    if (fabs(t_sys_data_current.bed_level_z_at_left_back) < 0.05f)
    {
      t_sys_data_current.bed_level_z_at_left_back = 0.0f;
    }

    if (fabs(t_sys_data_current.bed_level_z_at_right_back) < 0.05f)
    {
      t_sys_data_current.bed_level_z_at_right_back = 0.0f;
    }

    if (fabs(t_sys_data_current.bed_level_z_at_right_front) < 0.05f)
    {
      t_sys_data_current.bed_level_z_at_right_front = 0.0f;
    }

    // 初始化矩阵
    plan_bed_level_matrix.set_to_identity();
    // 初始化3个点坐标
    vector_3 xLeftyFront = vector_3(left_probe_bed_position, front_probe_bed_position, t_sys_data_current.bed_level_z_at_left_front);
    vector_3 xLeftyBack = vector_3(left_probe_bed_position, back_probe_bed_position, t_sys_data_current.bed_level_z_at_left_back);
    vector_3 xRightyBack = vector_3(right_probe_bed_position, back_probe_bed_position, t_sys_data_current.bed_level_z_at_right_back);
    vector_3 xRightyFront = vector_3(right_probe_bed_position, front_probe_bed_position, t_sys_data_current.bed_level_z_at_right_front);
    // 计算3点构成平面法向量
    vector_3 xPositive = (xRightyFront - xLeftyFront).get_normal();
    //vector_3 yPositive = (xLeftyBack - xLeftyFront).get_normal();
    vector_3 yPositive = (xRightyBack - xRightyFront).get_normal();
    vector_3 planeNormal = vector_3::cross(yPositive, xPositive).get_normal();

    if (planeNormal.z < 0)
    {
      planeNormal.x = -planeNormal.x;
      planeNormal.y = -planeNormal.y;
      planeNormal.z = -planeNormal.z;
    }

    // 计算平台矩阵
    plan_bed_level_matrix_reversed.set_to_identity();
    plan_bed_level_matrix_reversed = matrix_3x3::create_look_at(planeNormal, yPositive);
    // and set our bed level equation to do the right thing
    plan_bed_level_matrix = matrix_3x3::create_inverse(plan_bed_level_matrix_reversed);
    //#define ENABLE_AUTO_BED_LEVELING_DEBUG //自动调平开关测试模式
    #ifdef ENABLE_AUTO_BED_LEVELING_DEBUG
    USER_DbgLog("pos1_x = %f; pos1_y = %f; pos1_z = %f", (float)POS1_X, (float)POS1_Y, t_sys_data_current.bed_level_z_at_left_front);
    USER_DbgLog("pos2_x = %f; pos2_y = %f; pos2_z = %f", (float)POS2_X, (float)POS2_Y, t_sys_data_current.bed_level_z_at_left_back);
    USER_DbgLog("pos3_x = %f; pos3_y = %f; pos3_z = %f", (float)POS3_X, (float)POS3_Y, t_sys_data_current.bed_level_z_at_right_front);
    //int i;  for(i =0; i < 9; ++i)
    //USER_DbgLog("matrix %d = %f",i,autoBedLeveling.plan_bed_level_matrix.matrix[i]);
    USER_DbgLog("matrix0 = %f; matrix1 = %f; matrix2 = %f", (float)plan_bed_level_matrix.matrix[0], (float)plan_bed_level_matrix.matrix[1], plan_bed_level_matrix.matrix[2]);
    USER_DbgLog("matrix3 = %f; matrix4 = %f; matrix5 = %f", (float)plan_bed_level_matrix.matrix[3], (float)plan_bed_level_matrix.matrix[4], plan_bed_level_matrix.matrix[5]);
    USER_DbgLog("matrix6 = %f; matrix7 = %f; matrix8 = %f", (float)plan_bed_level_matrix.matrix[6], (float)plan_bed_level_matrix.matrix[7], plan_bed_level_matrix.matrix[8]);
    #endif // ENABLE_AUTO_BED_LEVELING_DEBUG
  }

  float plan_get_current_save_xyz(const volatile int axis)
  {
    return current_save_xyz[axis];
  }

  void plan_set_current_save_xyz(const volatile int axis, const volatile float value)
  {
    current_save_xyz[axis] = value;
  }

  void plan_set_process_auto_bed_level_status(const volatile bool status)
  {
    if (1 == t_sys_data_current.enable_bed_level)
    {
      is_process_auto_bed_leveling = status;
    }
  }

  void plan_buffer_line_get_xyz(volatile float &x, volatile float &y, volatile float &z)
  {
    if (1 == t_sys_data_current.enable_bed_level)
    {
      static bool isNotAutoBedLevel = false;

      if (is_process_auto_bed_leveling)
      {
        // 当前位置不变化，不移动
        if ((!isNotAutoBedLevel) &&
            sg_util::is_float_data_equivalent(current_save_xyz[X_AXIS], x) &&
            sg_util::is_float_data_equivalent(current_save_xyz[Y_AXIS], y) &&
            sg_util::is_float_data_equivalent(current_save_xyz[Z_AXIS], z))
        {
          x = current_xyz_real[X_AXIS];
          y = current_xyz_real[Y_AXIS];
          z = current_xyz_real[Z_AXIS];
          return;
        }

        if (isNotAutoBedLevel)
        {
          isNotAutoBedLevel = false;
        }

        // 保存当前位置
        current_save_xyz[X_AXIS] = x;
        current_save_xyz[Y_AXIS] = y;
        current_save_xyz[Z_AXIS] = z;
        // 矩阵变换，转化移动坐标
        plan_apply_rotation_xyz(x, y, z);

        if (x < 0) // 转换坐标X如果为负数，不移动，保存当前该值，以便下一次补偿
        {
          current_xyz_compensate[X_AXIS] = -x;
          x = 0;
        }
        else // 确定是否需要补偿，清除补偿值
        {
          x += current_xyz_compensate[X_AXIS]; // float
          current_xyz_compensate[X_AXIS] = 0.0f;
        }

        if (y < 0) // 转换坐标X如果为负数，不移动，保存当前该值，以便下一次补偿
        {
          current_xyz_compensate[Y_AXIS] = -y;
          y = 0;
        }
        else // 确定是否需要补偿，清除补偿值
        {
          y += current_xyz_compensate[Y_AXIS]; // float
          current_xyz_compensate[Y_AXIS] = 0.0f;
        }

        // 保存真实移动坐标
        current_xyz_real[X_AXIS] = x;
        current_xyz_real[Y_AXIS] = y;
        current_xyz_real[Z_AXIS] = z;
      }
      else
      {
        // 保存当前位置
        current_save_xyz[X_AXIS] = x;
        current_save_xyz[Y_AXIS] = y;
        current_save_xyz[Z_AXIS] = z;
        // 保存真实移动坐标
        current_xyz_real[X_AXIS] = x;
        current_xyz_real[Y_AXIS] = y;
        current_xyz_real[Z_AXIS] = z;

        for (int i = 0; i < XYZ_NUM_AXIS; i++)
        {
          current_xyz_compensate[i] = 0.0f;
        }

        isNotAutoBedLevel = true;
      }
    }
    else
    {
      // 保存当前位置
      current_save_xyz[X_AXIS] = x;
      current_save_xyz[Y_AXIS] = y;
      current_save_xyz[Z_AXIS] = z;
    }
  }

  // Returns true if the buffer has a queued block, false otherwise
  bool planner_blocks_queued(void)
  {
    return (block_buffer_head != block_buffer_tail);
  }

  //return the nr of buffered moves
  int planner_moves_planned(void)
  {
    return ((block_buffer_head - block_buffer_tail) + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1);
  }

  //1.队空条件：block_buffer_head==block_buffer_tail
  //2.队满条件：(block_buffer_head+1) %BLOCK_BUFFER_SIZE==block_buffer_tail，其中BLOCK_BUFFER_SIZE为循环队列的最大长度
  //3.计算队列长度：（block_buffer_head-block_buffer_tail+BLOCK_BUFFER_SIZE）%BLOCK_BUFFER_SIZE
  //4.入队：（block_buffer_head+1）%BLOCK_BUFFER_SIZE
  //5.出队：（block_buffer_tail+1）%BLOCK_BUFFER_SIZE
  bool is_planner_moves_planned_full(void)
  {
    return (((block_buffer_head + BLOCK_BUFFER_SIZE) - block_buffer_tail) & (BLOCK_BUFFER_SIZE - 1)) < (BLOCK_BUFFER_SIZE - 1);
  }
  #ifdef __cplusplus
} //extern "C" {
  #endif

}
