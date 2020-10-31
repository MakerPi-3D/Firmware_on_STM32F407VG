
#ifndef stepper_h
#define stepper_h

#include <stdbool.h>
#include "threed_engine.h"

namespace sg_grbl
{

#ifdef __cplusplus
  extern "C" {
#endif

    void st_init(void);                                                 ///< Initialize and start the stepper motor subsystem
    void st_enable_endstops(const volatile bool check);                                ///< Enable/disable endstop checking
    void st_set_position(const volatile long (&_current_position)[MAX_NUM_AXIS]);                ///< Set current position in steps
    void st_set_position_axis(const volatile long value, const volatile int axis);
    void st_synchronize(void);                                          ///< Block until all buffered steps are executed
    void st_quick_stop(void);                                           ///< 电机快速停止
    long st_get_position_steps(const volatile int axis);                               ///< Get current position in steps
    float st_get_endstops_len(const volatile int axis);                                ///< 获取限位位置
    float st_get_position_length(const volatile int axis);                             ///< 获取当前实际位置
    bool st_get_z_max_endstops_status(void);                            ///< 获取Z最大限位状态
    bool st_check_endstop_z_hit_max(void);                              ///< 是否撞击Z限位最大，只检测一次，函数调用完置状态为false
    bool st_check_endstop_z_hit_min(void);                              ///< 是否撞击Z限位最小，只检测一次，函数调用完置状态为false
    bool st_is_xyz_min_endstops_hit(void);                              ///<
    void st_clear_xyz_min_endstops_hit(void);                           ///<
    bool st_is_min_endstop(const volatile int axis);                                   ///<
    bool st_check_queue_is_empty(void);                                 ///< 检测队列是否为空
    void st_enable_axis(const volatile int axis, const volatile bool isEnable);

    // The stepper subsystem goes to sleep when it runs out of things to execute. Call this
    // to notify the subsystem that it is time to go to work.
    void st_wake_up(void);

    // "The Stepper Driver Interrupt" - This timer interrupt is the workhorse.
    // It pops blocks from the block_buffer and executes them by pulsing the stepper pins appropriately.
    void st_process(void);

#ifdef __cplusplus
  } //extern "C" {
#endif

}

#endif
