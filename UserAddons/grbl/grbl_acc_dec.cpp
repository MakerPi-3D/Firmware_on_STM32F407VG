#include "grbl_acc_dec.h"
#include "sg_util.h"
#include <math.h>
#include "threed_engine.h"
#include "Configuration.h"
#include "sysconfig_data.h"
#include "sys_function.h"
#include "globalvariables.h"
#include "stepper_pin.h"

namespace sg_grbl
{

  #ifdef __cplusplus
  extern "C" {
  #endif

#define NOLESS(v,n) do{ if (v < n) v = n; }while(0)
#define NOMORE(v,n) do{ if (v > n) v = n; }while(0)

  //#define sq(x)  (x*x)

  //#ifndef min
  //#define min(a,b) ((a)<(b)?(a):(b))
  //#endif // min

  //#ifndef max
  //#define max(a,b) ((a)>(b)?(a):(b))
  //#endif // max

  // intRes = longIn1 * longIn2 >> 24
  // uses:
  // r26 to store 0
  // r27 to store bits 16-23 of the 48bit result. The top bit is used to round the two byte result.
  // note that the lower two bytes and the upper byte of the 48bit result are not calculated.
  // this can cause the result to be out by one as the lower bytes may cause carries into the upper ones.
  // B0 A0 are bits 24-39 and are the returned value
  // C1 B1 A1 is longIn1
  // D2 C2 B2 A2 is longIn2
  //
#define MultiU24X32toH16(intRes, longIn1, longIn2)  intRes = (UINT16)((((UINT64)longIn1 * (UINT64)longIn2)>> 24)& 0XFFFF);

  //         __________________________
  //        /|                        |\     _________________         ^
  //       / |                        | \   /|               |\        |
  //      /  |                        |  \ / |               | \       s
  //     /   |                        |   |  |               |  \      p
  //    /    |                        |   |  |               |   \     e
  //   +-----+------------------------+---+--+---------------+----+    e
  //   |               BLOCK 1            |      BLOCK 2          |    d
  //
  //                           time ----->
  //
  //  The trapezoid is the shape the speed curve over time. It starts at block->initial_rate, accelerates
  //  first block->accelerate_until step_events_completed, then keeps going at constant speed until
  //  step_events_completed reaches block->decelerate_after after which it decelerates until the trapezoid generator is reset.
  //  The slope of acceleration is calculated with the leib ramp alghorithm.
  static inline unsigned short calc_timer(unsigned short step_rate, volatile char &step_loops)
  {
    unsigned short timer;
    NOMORE(step_rate, MAX_STEP_FREQUENCY);

    if (step_rate > 20000)
    {
      // If steprate > 20kHz >> step 4 times
      step_rate = (step_rate >> 2) & 0x3fff;
      step_loops = 4;
    }
    else if (step_rate > 10000)
    {
      // If steprate > 10kHz >> step 2 times
      step_rate = (step_rate >> 1) & 0x7fff;
      step_loops = 2;
    }
    else
    {
      step_loops = 1;
    }

    if (t_sys.is_granulator)
    {
      timer = (UINT16)(((2000000 / step_rate) - 1) + ((step_loops * 3) * 2));
    }
    else
    {
      #ifdef USE_PULSE_DELAY
      timer = (UINT16)(2000000 / step_rate - 1 + step_loops * 2 * t_sys.pulse_delay_time);
      #else
      timer = (UINT16)((2000000 / step_rate) - 1);
      #endif
    }

    if (timer < 100) //(20kHz this should never happen)
      timer = 100;

    return timer;
  }

  #ifdef __cplusplus
} // extern "C" {
  #endif

Trapezoid::Trapezoid()
{
  acceleration_time = 0;
  deceleration_time = 0;
  acc_step_rate = 0; // needed for deccelaration start point
  OCR1A_nominal = 0;
  step_loops_nominal = 0;
}

void Trapezoid::calculate_for_block(volatile block_t *const block, float entry_factor, float exit_factor)
{
  unsigned long initial_rate = (unsigned long)ceil(block->nominal_rate * entry_factor); // (step/min)
  unsigned long final_rate = (unsigned long)ceil(block->nominal_rate * exit_factor); // (step/min)
  // Limit minimal step rate (Otherwise the timer will overflow.)
  NOLESS(initial_rate, 120);
  NOLESS(final_rate, 120);
  long accel = ceil(block->acceleration_st);
  INT32 accelerate_steps = ceil(estimate_acceleration_distance(initial_rate, block->nominal_rate, accel));
  INT32 decelerate_steps = floor(estimate_acceleration_distance(block->nominal_rate, final_rate, -accel));
  // Calculate the size of Plateau of Nominal Rate.
  INT32 plateau_steps = block->step_event_count - accelerate_steps - decelerate_steps;

  // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
  // have to use intersection_distance() to calculate when to abort accel and start braking
  // in order to reach the final_rate exactly at the end of this block.
  if (plateau_steps < 0)
  {
    accelerate_steps = ceil(intersection_distance(initial_rate, final_rate, accel, block->step_event_count));
    accelerate_steps = sg_util::max_p(accelerate_steps, 0); // Check limits due to numerical round-off
    accelerate_steps = sg_util::min_p((unsigned long)accelerate_steps, block->step_event_count);//(We can cast here to unsigned, because the above line ensures that we are above zero)
    plateau_steps = 0;
  }

  sys_task_enter_critical();

  if (!block->busy)
  {
    // Don't update variables if block is busy.
    block->accelerate_until = accelerate_steps;
    block->decelerate_after = accelerate_steps + plateau_steps;
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
  }

  sys_task_exit_critical();
}

/**
 * Calculate the distance (not time) it takes to accelerate
 * from initial_rate to target_rate using the given acceleration:
 */
inline float Trapezoid::estimate_acceleration_distance(float initial_rate, float target_rate, float accel)
{
  if (accel == 0) return 0; // accel was 0, set acceleration distance to 0

  return (sg_util::square_p(target_rate) - sg_util::square_p(initial_rate)) / (accel * 2);
}

/**
* Return the point at which you must start braking (at the rate of -'acceleration') if
* you start at 'initial_rate', accelerate (until reaching the point), and want to end at
* 'final_rate' after traveling 'distance'.
*
* This is used to compute the intersection point between acceleration and deceleration
* in cases where the "trapezoid" has no plateau (i.e., never reaches maximum speed)
*/
inline float Trapezoid::intersection_distance(float initial_rate, float final_rate, float accel, float distance)
{
  if (accel == 0) return 0; // accel was 0, set intersection distance to 0

  return ((((accel * 2) * distance) - (initial_rate * initial_rate)) + (final_rate * final_rate)) / (accel * 4);
}

// Initializes the trapezoid generator from the current block. Called whenever a new
// block begins.
void Trapezoid::generator_reset(volatile block_t *const st_current_block, volatile char &step_loops)
{
  deceleration_time = 0;
  // step_rate to timer interval
  OCR1A_nominal = calc_timer((unsigned short) st_current_block->nominal_rate, step_loops);
  // make a note of the number of step loops required at nominal speed
  step_loops_nominal = step_loops;
  acc_step_rate = (unsigned short)st_current_block->initial_rate;
  acceleration_time = calc_timer(acc_step_rate, step_loops);
  stepper_timer_set_period((UINT32)acceleration_time);
}

void Trapezoid::calculare_timer(volatile block_t *const st_current_block, unsigned long step_events_completed, volatile char &step_loops)
{
  unsigned short timer;
  unsigned short step_rate;

  if (step_events_completed <= (unsigned long)st_current_block->accelerate_until)
  {
    MultiU24X32toH16(acc_step_rate, acceleration_time, st_current_block->acceleration_rate);
    acc_step_rate += st_current_block->initial_rate;
    // upper limit
    NOMORE(acc_step_rate, st_current_block->nominal_rate);
    // step_rate to timer interval
    timer = calc_timer(acc_step_rate, step_loops);
    stepper_timer_set_period(timer);
    acceleration_time += timer;
  }
  else if (step_events_completed > (unsigned long)st_current_block->decelerate_after)
  {
    MultiU24X32toH16(step_rate, deceleration_time, st_current_block->acceleration_rate);

    if (step_rate <= acc_step_rate)   // Still decelerating?
    {
      step_rate = acc_step_rate - step_rate;
      NOLESS(step_rate, st_current_block->final_rate);
    }
    else
      step_rate = st_current_block->final_rate;

    // step_rate to timer interval
    timer = calc_timer(step_rate, step_loops);
    stepper_timer_set_period(timer);
    deceleration_time += timer;
  }
  else
  {
    stepper_timer_set_period(OCR1A_nominal);
    // ensure we're running at the correct step rate, even if we just came off an acceleration
    step_loops = step_loops_nominal;
  }
}

float Trapezoid::max_allowable_speed(float accel, float target_velocity, float distance)
{
  return sqrt((target_velocity * target_velocity) - ((2 * accel) * distance)); // float
}

Trapezoid trapezoid;

#ifdef S_CURVE_ALGORITHM

S_CURVE::S_CURVE()
{
  acc_acceleration_time = 0;
  acc_deceleration_time = 0;
  dec_acceleration_time = 0;
  dec_deceleration_time = 0;
  s_acc_step_rate = 0;
  s_dec_step_rate = 0;
  OCR1A_nominal = 0;
  step_loops_nominal = 0;
}

inline float S_CURVE::estimate_acceleration_distance(float initial_rate, float target_rate, float accel)
{
  if (accel == 0) return 0; // accel was 0, set acceleration distance to 0

  return (sq(target_rate) - sq(initial_rate)) / accel;
}

inline float S_CURVE::intersection_distance(float initial_rate, float final_rate, float accel, float distance)
{
  if (accel == 0) return 0; // accel was 0, set intersection distance to 0

  return (accel * distance - sq(initial_rate) + sq(final_rate)) / (accel * 2);
}

void S_CURVE::calculate_for_block(volatile block_t *const block, float entry_factor, float exit_factor)
{
  unsigned long initial_rate = (unsigned long)ceil(block->nominal_rate * entry_factor); // (step/min)
  unsigned long final_rate = (unsigned long)ceil(block->nominal_rate * exit_factor); // (step/min)
  // Limit minimal step rate (Otherwise the timer will overflow.)
  NOLESS(initial_rate, 120);
  NOLESS(final_rate, 120);
  long accel = ceil(block->acceleration_st);
  int32_t accelerate_steps = ceil(estimate_acceleration_distance(initial_rate, block->nominal_rate, accel));
  int32_t decelerate_steps = floor(estimate_acceleration_distance(block->nominal_rate, final_rate, -accel));
  // Calculate the size of Plateau of Nominal Rate.
  int32_t plateau_steps = block->step_event_count - accelerate_steps - decelerate_steps;

  // Is the Plateau of Nominal Rate smaller than nothing? That means no cruising, and we will
  // have to use intersection_distance() to calculate when to abort accel and start braking
  // in order to reach the final_rate exactly at the end of this block.
  if (plateau_steps < 0)
  {
    accelerate_steps = ceil(intersection_distance(initial_rate, final_rate, accel, block->step_event_count));
    accelerate_steps = max(accelerate_steps, 0); // Check limits due to numerical round-off
    accelerate_steps = min((UINT32)accelerate_steps, block->step_event_count);//(We can cast here to unsigned, because the above line ensures that we are above zero)
    block->nominal_rate = (UINT32)sqrt(sq((float)initial_rate) + (float)block->step_event_count * accel);
    plateau_steps = 0;
  }

  sys_task_enter_critical();

  if (!block->busy)
  {
    // Don't update variables if block is busy.
    block->accelerate_until = accelerate_steps;
    block->decelerate_after = accelerate_steps + plateau_steps;
    block->initial_rate = initial_rate;
    block->final_rate = final_rate;
    //用于计算 s_acc_step_rate 或 s_dec_step_rate
    block->acc_middle_rate = (block->initial_rate + block->nominal_rate) / 2;
    block->dec_middle_rate = (block->final_rate + block->nominal_rate) / 2;
    //用于计算 acc_param dec_param 的中间变量
    block->delta_acc_rate = (block->nominal_rate - block->initial_rate);
    block->delta_dec_rate = (block->nominal_rate - block->final_rate);
    block->acc_middle_timer = (uint64_t)block->delta_acc_rate * (F_CPU >> 3) / accel;
    block->dec_middle_timer = (uint64_t)block->delta_dec_rate * (F_CPU >> 3) / accel;
  }

  sys_task_exit_critical();
}

void S_CURVE::generator_reset(volatile block_t *const st_current_block, char &step_loops)
{
  acc_acceleration_time = 0;
  dec_acceleration_time = 0;
  acc_deceleration_time = 0;
  dec_deceleration_time = 0;
  // step_rate to timer interval
  OCR1A_nominal = calc_timer(st_current_block->nominal_rate, step_loops);
  // make a note of the number of step loops required at nominal speed
  step_loops_nominal = step_loops;
  s_acc_step_rate = st_current_block->initial_rate;
  s_dec_step_rate = st_current_block->nominal_rate;
  acc_acceleration_time = calc_timer(s_acc_step_rate, step_loops);
  st_current_block->acc_status = 1;
  stepper_timer_set_period(acc_acceleration_time);
}

void S_CURVE::calculare_timer(volatile block_t *const st_current_block, unsigned long step_events_completed, char &step_loops)
{
  unsigned short timer;

  if ((long)step_events_completed <= st_current_block->accelerate_until)
  {
    if (st_current_block->acc_status < 2)
    {
      MultiU24X32toH16(s_acc_step_rate, acc_acceleration_time, st_current_block->acceleration_rate);
      s_acc_step_rate = (UINT16)((sq((uint64_t)s_acc_step_rate) / (st_current_block->delta_acc_rate << 1)) & 0XFFFF);
      s_acc_step_rate += st_current_block->initial_rate;

      // 加速状态转换，如果到达中点速度切换下一个状态
      if (s_acc_step_rate >= st_current_block->acc_middle_rate)   //中点速度修复
      {
        s_acc_step_rate = st_current_block->acc_middle_rate;
        st_current_block->acc_status = 2;
        timer = calc_timer(s_acc_step_rate, step_loops);
        stepper_timer_set_period(timer);
        // 减加速时间补偿
        //dec_acceleration_time = timer - (st_current_block->acc_middle_timer - acc_acceleration_time);
        dec_acceleration_time = timer;
        return;
      }

      // step_rate to timer interval
      timer = calc_timer(s_acc_step_rate, step_loops);
      stepper_timer_set_period(timer);
      acc_acceleration_time += timer;
      return;
    }//end if st_current_block->acc_status < 2

    if (2 == st_current_block->acc_status)
    {
      //减加速段末速度限制
      if (s_acc_step_rate > st_current_block->nominal_rate || dec_acceleration_time > st_current_block->acc_middle_timer)
      {
        s_acc_step_rate = st_current_block->nominal_rate;
        st_current_block->acc_status = 3;
        stepper_timer_set_period(OCR1A_nominal);
        step_loops = step_loops_nominal;
        return;
      }

      // 采用梯形算法，减少运算量
      unsigned short _s_acc_step_rate;
      MultiU24X32toH16(_s_acc_step_rate, dec_acceleration_time, st_current_block->acceleration_rate);
      s_acc_step_rate += _s_acc_step_rate;

      // upper limit
      if (s_acc_step_rate > st_current_block->nominal_rate)
        s_acc_step_rate = st_current_block->nominal_rate;

      // step_rate to timer interval
      timer = calc_timer(s_acc_step_rate, step_loops);
      stepper_timer_set_period(timer);
      dec_acceleration_time += timer;
      return;
    }//end if 2 == st_current_block->acc_status
  }
  else if ((long) step_events_completed >= st_current_block->decelerate_after)
  {
    //PART2.1: 加减速段
    if (st_current_block->acc_status < 5)
    {
      if (st_current_block->acc_status < 4)st_current_block->acc_status = 4;

      MultiU24X32toH16(s_dec_step_rate, (acc_deceleration_time + OCR1A_nominal), st_current_block->acceleration_rate);
      s_dec_step_rate = (UINT16)((sq((uint64_t)s_dec_step_rate) / (st_current_block->delta_dec_rate << 1)) & 0XFFFF);
      s_dec_step_rate = (st_current_block->nominal_rate > s_dec_step_rate  ? (st_current_block->nominal_rate - s_dec_step_rate) : st_current_block-> dec_middle_rate);

      //加速状态转换，如果到达中点速度切换下一个状态
      if (s_dec_step_rate <= st_current_block->dec_middle_rate)
      {
        //中点速度修复
        s_dec_step_rate = st_current_block->dec_middle_rate;
        timer = calc_timer(s_dec_step_rate, step_loops);
        stepper_timer_set_period(timer);
        // 减减速时间补偿
        dec_deceleration_time = timer - (st_current_block->dec_middle_timer - acc_deceleration_time - OCR1A_nominal);
        st_current_block->acc_status = 5;
        return;
      }

      // step_rate to timer interval
      timer = calc_timer(s_dec_step_rate, step_loops);
      stepper_timer_set_period(timer);
      acc_deceleration_time += timer;
      return;
    }// end if st_current_block->acc_status < 5

    //PART2.2: 减减速段
    if (5 == st_current_block->acc_status)
    {
      MultiU24X32toH16(s_dec_step_rate, (st_current_block->dec_middle_timer - dec_deceleration_time), st_current_block->acceleration_rate);
      s_dec_step_rate = (UINT16)((sq(s_dec_step_rate) / (st_current_block->delta_dec_rate << 1)) & 0XFFFF);
      s_dec_step_rate += st_current_block->final_rate;

      //末速度限制
      if (s_dec_step_rate < st_current_block->final_rate ||
          s_dec_step_rate > st_current_block->nominal_rate ||
          (s_dec_step_rate <= st_current_block->nominal_rate && s_dec_step_rate > st_current_block->dec_middle_rate))
      {
        s_dec_step_rate = st_current_block->final_rate;
      }

      // step_rate to timer interval
      timer = calc_timer(s_dec_step_rate, step_loops);
      stepper_timer_set_period(timer);
      dec_deceleration_time += timer;
      return;
    }// end 5 == st_current_block->acc_status
  }
  else
  {
    stepper_timer_set_period(OCR1A_nominal);
    // ensure we're running at the correct step rate, even if we just came off an acceleration
    step_loops = step_loops_nominal;
  }
}

float S_CURVE::max_allowable_speed(float accel, float target_velocity, float distance)
{
  return  sqrt(sq(target_velocity) - accel * distance);
}

S_CURVE s_curve;

#endif // #ifdef S_CURVE_ALGORITHM

}



