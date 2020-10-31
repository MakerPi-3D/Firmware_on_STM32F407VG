#ifndef PLANNER_BLOCK_BUFFER_H
#define PLANNER_BLOCK_BUFFER_H

#include "threed_engine.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

//#define S_CURVE_ALGORITHM // S加减速开关
  // This struct is used when buffering the setup for each linear movement "nominal" values are as specified in
  // the source g-code and may never actually be reached if acceleration management is active.
  typedef struct
  {
    // Fields used by the bresenham algorithm for tracing the line
    long steps_axis[MAX_NUM_AXIS];
    unsigned long step_event_count;           // The number of step events required to complete this block
    long accelerate_until;                    // The index of the step event on which to stop acceleration
    long decelerate_after;                    // The index of the step event on which to start decelerating
    long acceleration_rate;                   // The acceleration rate used for acceleration calculation
    unsigned char direction_bits;             // The direction bit set for this block (refers to *_DIRECTION_BIT in config.h)
    short int active_extruder;                // Selects the active extruder

    // Fields used by the motion planner to manage acceleration
//  float speed_x, speed_y, speed_z, speed_e;        // Nominal mm/sec for each axis
    float nominal_speed;                               // The nominal speed for this block in mm/sec
    float entry_speed;                                 // Entry speed at previous-current junction in mm/sec
    float max_entry_speed;                             // Maximum allowable junction entry speed in mm/sec
    float millimeters;                                 // The total travel of this block in mm
    float acceleration;                                // acceleration mm/sec^2
    unsigned char recalculate_flag;                    // Planner flag to recalculate trapezoids on entry junction
    unsigned char nominal_length_flag;                 // Planner flag for nominal speed always reached
    char busy;

    // Settings for the trapezoid generator
    unsigned long nominal_rate;                        // The nominal step rate for this block in step_events/sec
    unsigned long initial_rate;                        // The jerk-adjusted step rate at start of block
    unsigned long final_rate;                          // The minimal rate at exit
    float acceleration_st;                     // acceleration steps/sec^2

#ifdef S_CURVE_ALGORITHM
    unsigned long acc_middle_rate;
    unsigned long dec_middle_rate;
    unsigned long delta_acc_rate;
    unsigned long delta_dec_rate;

    unsigned long acc_middle_timer;
    unsigned long dec_middle_timer;
    unsigned char acceleration_type;
    char acc_status;
#endif // #ifdef S_CURVE_ALGORITHM
  } block_t;


#ifdef __cplusplus
} // extern "C" {
#endif


#endif // PLANNER_BLOCK_BUFFER_H

