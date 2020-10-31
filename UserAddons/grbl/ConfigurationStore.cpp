#include <string.h>
#include "ConfigurationStore.h"
#include "user_debug.h"
#include "Configuration.h"
#include "config_model_tables.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "wbtypes.h"
#include "globalvariables.h"

namespace sg_grbl
{

  #ifdef __cplusplus
  extern "C" {
  #endif

  volatile int filament_load_unload_temp = 230;     ///< 进退丝温度
  volatile int pla_preheat_hotend_temp = 200 ;//170
  volatile int pla_preheat_hpb_temp = 70;
  volatile int abs_preheat_hotend_temp = 230 ;//170
  volatile int abs_preheat_hpb_temp = 100 ;//170

  volatile float z_home_retract_mm = 1.0F;
  volatile unsigned long minsegmenttime;
  volatile float max_feedrate[MAX_NUM_AXIS]; // set the max speeds
  volatile float axis_steps_per_unit[MAX_NUM_AXIS];
  volatile unsigned long max_acceleration_units_per_sq_second[MAX_NUM_AXIS]; // Use M201 to override by software
  volatile float minimumfeedrate;
  volatile float acceleration;         // Normal acceleration mm/s^2  THIS IS THE DEFAULT ACCELERATION for all moves. M204 SXXXX
  volatile float retract_acceleration; //  mm/s^2   filament pull-back and push-forward  while standing still in the other axis M204 TXXXX
  volatile float max_xy_jerk; //speed than can be stopped at once, if i understand correctly.
  volatile float max_z_jerk;
  volatile float max_e_jerk;
  volatile float max_b_jerk;
  volatile float mintravelfeedrate;
  volatile unsigned long axis_steps_per_sqr_second[MAX_NUM_AXIS];
  volatile bool axis_relative_modes[MAX_NUM_AXIS];

  const long dropsegments = DROP_SEGMENTS;

  // Calculate the steps/s^2 acceleration rates, based on the mm/s^s
  void reset_acceleration_rates()
  {
    for (INT16 i = 0; i < static_cast<INT16>(motion_3d.axis_num); ++i)
    {
      axis_steps_per_sqr_second[i] = max_acceleration_units_per_sq_second[i] * static_cast<ULONG>(axis_steps_per_unit[i]);
    }
  }

  void Config_ResetDefault()
  {
    const bool axis_relative_modes_buf[MAX_NUM_AXIS] = AXIS_RELATIVE_MODES;
    const float axis_steps_per_unit_buf[MAX_NUM_AXIS] = DEFAULT_AXIS_STEPS_PER_UNIT;
    const float max_feedrate_buf[MAX_NUM_AXIS] = DEFAULT_MAX_FEEDRATE;
    const unsigned long max_acc_units_per_sq_second_buf[MAX_NUM_AXIS] = DEFAULT_MAX_ACCELERATION;
    minsegmenttime = DEFAULT_MINSEGMENTTIME;
    minimumfeedrate = DEFAULT_MINIMUMFEEDRATE;
    acceleration = DEFAULT_ACCELERATION;
    retract_acceleration = DEFAULT_RETRACT_ACCELERATION;
    max_xy_jerk = DEFAULT_XYJERK;
    max_z_jerk = DEFAULT_ZJERK;
    max_e_jerk = DEFAULT_EJERK;
    max_b_jerk = DEFAULT_BJERK;
    mintravelfeedrate = DEFAULT_MINTRAVELFEEDRATE;
    memcpy((bool *)axis_relative_modes, axis_relative_modes_buf, MAX_NUM_AXIS * sizeof(bool));
    z_home_retract_mm = static_cast<float>(Z_HOME_RETRACT_MM);
    pla_preheat_hotend_temp = PLA_PREHEAT_HOTEND_TEMP ;//170
    pla_preheat_hpb_temp = PLA_PREHEAT_HPB_TEMP;
    abs_preheat_hotend_temp = ABS_PREHEAT_HOTEND_TEMP ;//170
    abs_preheat_hpb_temp = ABS_PREHEAT_HPB_TEMP ;//170

    for (int i = 0; i < MAX_NUM_AXIS; ++i)
    {
      axis_steps_per_unit[i] = axis_steps_per_unit_buf[i] * static_cast<float>(motion_3d.step); // float
      max_feedrate[i] = max_feedrate_buf[i];
      max_acceleration_units_per_sq_second[i] = max_acc_units_per_sq_second_buf[i];
    }

    // steps per sq second need to be updated to agree with the units per sq second
    reset_acceleration_rates();
  }

  #ifdef __cplusplus
} //extern "C" {
  #endif

}
