#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include "threed_engine.h"
#include <stdbool.h>

namespace sg_grbl
{


#ifdef __cplusplus
  extern "C" {
#endif

    extern volatile unsigned long minsegmenttime;
    extern volatile float max_feedrate[MAX_NUM_AXIS]; // set the max speeds
    extern volatile float axis_steps_per_unit[MAX_NUM_AXIS];
    extern volatile unsigned long max_acceleration_units_per_sq_second[MAX_NUM_AXIS]; // Use M201 to override by software
    extern volatile float minimumfeedrate;
    extern volatile float acceleration;         // Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
    extern volatile float retract_acceleration; //  mm/s^2   filament pull-pack and push-forward  while standing still in the other axis M204 TXXXX
    extern volatile float max_xy_jerk; //speed than can be stopped at once, if i understand correctly.
    extern volatile float max_z_jerk;
    extern volatile float max_e_jerk;
    extern volatile float max_b_jerk;
    extern volatile float mintravelfeedrate;
    extern volatile unsigned long axis_steps_per_sqr_second[MAX_NUM_AXIS];
    extern volatile bool axis_relative_modes[MAX_NUM_AXIS];

    extern const long dropsegments;

    extern volatile int filament_load_unload_temp;
//  extern float extrude_min_temp;
//  extern int heater_0_maxtemp;
    extern volatile int pla_preheat_hotend_temp;//170
    extern volatile int pla_preheat_hpb_temp;


    extern volatile int abs_preheat_hotend_temp;//170
    extern volatile int abs_preheat_hpb_temp;//170

    extern volatile float z_home_retract_mm;

    void reset_acceleration_rates(void);
    void Config_ResetDefault(void);





#ifdef __cplusplus
  } // extern "C" {
#endif

}

#endif//CONFIG_STORE_H
