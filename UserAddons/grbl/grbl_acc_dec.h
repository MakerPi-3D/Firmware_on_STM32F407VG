#ifndef GRBL_ACC_DEC_H
#define GRBL_ACC_DEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "planner_block_buffer.h"

#ifdef __cplusplus
} // extern "C" {
#endif

namespace sg_grbl
{

  class Trapezoid
  {
  public:
    Trapezoid();
    void calculate_for_block(volatile block_t * const block, float entry_factor, float exit_factor);
    void generator_reset(volatile block_t * const st_current_block, volatile char &step_loops);
    void calculare_timer(volatile block_t * const st_current_block, unsigned long step_events_completed, volatile char &step_loops);
    /**
     * Calculate the maximum allowable speed at this point, in order
     * to reach 'target_velocity' using 'acceleration' within a given
     * 'distance'.
     */
    float max_allowable_speed(float accel, float target_velocity, float distance);
  private:
    /**
     * Calculate the distance (not time) it takes to accelerate
     * from initial_rate to target_rate using the given acceleration:
     */
    inline float estimate_acceleration_distance(float initial_rate, float target_rate, float accel);
    /**
     * Return the point at which you must start braking (at the rate of -'acceleration') if
     * you start at 'initial_rate', accelerate (until reaching the point), and want to end at
     * 'final_rate' after traveling 'distance'.
     *
     * This is used to compute the intersection point between acceleration and deceleration
     * in cases where the "trapezoid" has no plateau (i.e., never reaches maximum speed)
     */
    inline float intersection_distance(float initial_rate, float final_rate, float accel, float distance);
  private:
    long acceleration_time;
    long deceleration_time;
    unsigned short acc_step_rate; // needed for deccelaration start point
    unsigned short OCR1A_nominal;
    char step_loops_nominal;
  };
  extern Trapezoid trapezoid;

#ifdef S_CURVE_ALGORITHM
  class S_CURVE
  {
  public:
    S_CURVE();
    void calculate_for_block(volatile block_t * const block, float entry_factor, float exit_factor);
    void generator_reset(volatile block_t * const st_current_block, char &step_loops);
    void calculare_timer(volatile block_t * const st_current_block, unsigned long step_events_completed, char &step_loops);
    float max_allowable_speed(float accel, float target_velocity, float distance);
  private:
    inline float estimate_acceleration_distance(float initial_rate, float target_rate, float accel);
    inline float intersection_distance(float initial_rate, float final_rate, float accel, float distance);
  private:
    long acc_acceleration_time;
    long acc_deceleration_time;
    long dec_acceleration_time;
    long dec_deceleration_time;
    unsigned short s_acc_step_rate;
    unsigned short s_dec_step_rate;
    unsigned short OCR1A_nominal;
    char step_loops_nominal;
  };
  extern S_CURVE s_curve;

#endif // #ifdef S_CURVE_ALGORITHM

#ifdef __cplusplus
  extern "C" {
#endif

    inline void trapezoid_or_s_curve_generator_reset(volatile block_t *const st_current_block, volatile char &step_loops)
    {
#ifdef S_CURVE_ALGORITHM
      if(0 == st_current_block->acceleration_type)
        trapezoid.generator_reset(st_current_block, step_loops);
      else
        s_curve.generator_reset(st_current_block, step_loops);
#else
      trapezoid.generator_reset(st_current_block, step_loops);
#endif // #ifdef S_CURVE_ALGORITHM
    }

    inline void trapezoid_or_s_curve_calculare_timer(volatile block_t * const st_current_block, unsigned long step_events_completed, volatile char &step_loops)
    {
#ifdef S_CURVE_ALGORITHM
      if(0 == st_current_block->acceleration_type)
        trapezoid.calculare_timer(st_current_block, step_events_completed, step_loops);
      else
        s_curve.calculare_timer(st_current_block, step_events_completed, step_loops);
#else
      trapezoid.calculare_timer(st_current_block, step_events_completed, step_loops);
#endif // #ifdef S_CURVE_ALGORITHM
    }

    inline float trapezoid_or_s_curve_max_allowable_speed(volatile block_t * const block, float accel, float target_velocity, float distance)
    {
#ifdef S_CURVE_ALGORITHM
      if(0 == block->acceleration_type)
        return trapezoid.max_allowable_speed(accel, target_velocity, distance);
      else
        return s_curve.max_allowable_speed(accel, target_velocity, distance);
#else
      return trapezoid.max_allowable_speed(accel, target_velocity, distance);
#endif // #ifdef S_CURVE_ALGORITHM
    }

    inline void trapezoid_or_s_curve_calculate_for_block(volatile block_t * const block, float entry_factor, float exit_factor)
    {
#ifdef S_CURVE_ALGORITHM
      if(0 == block->acceleration_type)
        trapezoid.calculate_for_block(block, entry_factor, exit_factor);
      else
        s_curve.calculate_for_block(block, entry_factor, exit_factor);
#else
      trapezoid.calculate_for_block(block, entry_factor, exit_factor);
#endif // #ifdef S_CURVE_ALGORITHM
    }

//inline void trapezoid_or_s_curve_set_acceleration_type(block_t *block, const  float (&delta_mm)[MAX_NUM_AXIS])
//{
//#ifdef S_CURVE_ALGORITHM
//  // 默认S型加减速
//  block->acceleration_type = 1; //s-curve

//  // 距离小于8mm,改用梯形算法，避免频繁加减速，需要考虑微小线段问题
//  if(block->millimeters <= 8)
//    block->acceleration_type = 0; //trazoid

//  // 存在Z轴，用T型加减速
//  if( delta_mm[Z_AXIS] > 0 )
//    block->acceleration_type=0; //trazoid

//#endif // #ifdef S_CURVE_ALGORITHM
//}

//inline void trapezoid_or_s_curve_reset_acceleration_st(block_t *block)
//{
//#ifdef S_CURVE_ALGORITHM
//  // S型加减速，设置加速度为原来的四分之一
//  if(block->acceleration_type)
//    block->acceleration_st /= 4;
//#endif // #ifdef S_CURVE_ALGORITHM
//}

#ifdef __cplusplus
  } // extern "C" {
#endif

#endif // GRBL_ACC_DEC_H
}


