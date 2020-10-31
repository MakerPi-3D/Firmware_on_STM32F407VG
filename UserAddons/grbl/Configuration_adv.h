#ifndef CONFIGURATION_ADV_H
#define CONFIGURATION_ADV_H

//===========================================================================
//=============================Thermal Settings  ============================
//===========================================================================

#define BED_CHECK_INTERVAL 5000 //ms between checks in bang-bang control

#ifdef PIDTEMP
// this adds an experimental additional term to the heating power, proportional to the extrusion speed.
// if Kc is chosen well, the additional required power due to increased melting should be compensated.
//#define PID_ADD_EXTRUSION_RATE
#ifdef PID_ADD_EXTRUSION_RATE
#define  DEFAULT_Kc (1) //heating power=Kc*(e_speed)
#endif
#endif

//  extruder run-out prevention.
//if the machine is idle, and the temperature over MINTEMP, every couple of SECONDS some filament is extrudedNT
#define EXTRUDER_RUNOUT_MINTEMP 190
#define EXTRUDER_RUNOUT_SECONDS 30.
#define EXTRUDER_RUNOUT_ESTEPS 14. //mm filament
#define EXTRUDER_RUNOUT_SPEED 1500.  //extrusion speed
#define EXTRUDER_RUNOUT_EXTRUDE 100


//===========================================================================
//=============================Mechanical Settings===========================
//===========================================================================

#define ENDSTOPS_ONLY_FOR_HOMING // If defined the endstops will only be used for homing

//// Added by ZetaPhoenix 09-15-2012
//#ifdef MANUAL_HOME_POSITIONS  // Use manual limit switch locations
//  #define X_HOME_POS MANUAL_X_HOME_POS
//  #define Y_HOME_POS MANUAL_Y_HOME_POS
//  #define Z_HOME_POS MANUAL_Z_HOME_POS
//#else //Set min/max homing switch positions based upon homing direction and min/max travel limits
//X axis
#define X_HOME_POS 0
//  #if X_HOME_DIR == -1
//    #ifdef BED_CENTER_AT_0_0
//      #define X_HOME_POS X_MAX_LENGTH * -0.5
//    #else
//      #define X_HOME_POS X_MIN_POS
//    #endif //BED_CENTER_AT_0_0
//  #else
//    #ifdef BED_CENTER_AT_0_0
//      #define X_HOME_POS X_MAX_LENGTH * 0.5
//    #else
//      #define X_HOME_POS X_MAX_POS
//    #endif //BED_CENTER_AT_0_0
//  #endif //X_HOME_DIR == -1

//Y axis
#define Y_HOME_POS 0
//  #if Y_HOME_DIR == -1
//    #ifdef BED_CENTER_AT_0_0
//      #define Y_HOME_POS Y_MAX_LENGTH * -0.5
//    #else
//      #define Y_HOME_POS Y_MIN_POS
//    #endif //BED_CENTER_AT_0_0
//  #else
//    #ifdef BED_CENTER_AT_0_0
//      #define Y_HOME_POS Y_MAX_LENGTH * 0.5
//    #else
//      #define Y_HOME_POS Y_MAX_POS
//    #endif //BED_CENTER_AT_0_0
//  #endif //Y_HOME_DIR == -1

// Z axis
#define Z_HOME_POS 0
//  #if Z_HOME_DIR == -1 //BED_CENTER_AT_0_0 not used
//    #define Z_HOME_POS Z_MIN_POS
//  #else
//    #define Z_HOME_POS Z_MAX_POS
//  #endif //Z_HOME_DIR == -1
//#endif //End auto min/max positions

//homing hits the endstop, then retracts by this distance, before it tries to slowly bump again:
#define X_HOME_RETRACT_MM 5
#define Y_HOME_RETRACT_MM 5
#define Z_HOME_RETRACT_MM 1
#define QUICK_HOME  //if this is defined, if both x and y are to be homed, a diagonal move will be performed initially.

#define AXIS_RELATIVE_MODES {false, false, false, false, false}

#define MAX_STEP_FREQUENCY 40000 // Max step frequency for Ultimaker (5000 pps / half step)

//default stepper release if idle
#define DEFAULT_STEPPER_DEACTIVE_TIME 60

#define DEFAULT_MINIMUMFEEDRATE       0.0     // minimum feedrate
#define DEFAULT_MINTRAVELFEEDRATE     0.0

// minimum time in microseconds that a movement needs to take if the buffer is emptied.
#define DEFAULT_MINSEGMENTTIME        20000

// If defined the movements slow down when the look ahead buffer is only half full
#define SLOWDOWN

// Minimum planner junction speed. Sets the default minimum speed the planner plans for at the end
// of the buffer and all stops. This should not be much greater than zero and should only be changed
// if unwanted behavior is observed on a user's machine when running at very slow speeds.
#define MINIMUM_PLANNER_SPEED 0.05f// (mm/sec)

////===========================================================================
////=============================Additional Features===========================
////===========================================================================

// Arc interpretation settings:
#define MM_PER_ARC_SEGMENT 1
#define N_ARC_CORRECTION 25

#define DROP_SEGMENTS 5 //everything with less than this number of steps will be ignored as move and joined with the next movement



//===========================================================================
//=============================  Define Defines  ============================
//===========================================================================
#if TEMP_SENSOR_0 > 0
#define THERMISTORHEATER_0 TEMP_SENSOR_0
#define HEATER_0_USES_THERMISTOR
#endif
#if TEMP_SENSOR_1 > 0
#define THERMISTORHEATER_1 TEMP_SENSOR_1
#define HEATER_1_USES_THERMISTOR
#endif
#if TEMP_SENSOR_2 > 0
#define THERMISTORHEATER_2 TEMP_SENSOR_2
#define HEATER_2_USES_THERMISTOR
#endif
#if TEMP_SENSOR_BED > 0
#define THERMISTORBED TEMP_SENSOR_BED
#define BED_USES_THERMISTOR
#endif
#if TEMP_SENSOR_0 == -1
#define HEATER_0_USES_AD595
#endif
#if TEMP_SENSOR_1 == -1
#define HEATER_1_USES_AD595
#endif
#if TEMP_SENSOR_2 == -1
#define HEATER_2_USES_AD595
#endif
#if TEMP_SENSOR_BED == -1
#define BED_USES_AD595
#endif
#if TEMP_SENSOR_0 == -2
#define HEATER_0_USES_MAX6675
#endif
#if TEMP_SENSOR_0 == 0
#undef HEATER_0_MINTEMP
#undef HEATER_0_MAXTEMP
#endif
#if TEMP_SENSOR_1 == 0
#undef HEATER_1_MINTEMP
#undef HEATER_1_MAXTEMP
#endif
#if TEMP_SENSOR_2 == 0
#undef HEATER_2_MINTEMP
#undef HEATER_2_MAXTEMP
#endif
#if TEMP_SENSOR_BED == 0
#undef BED_MINTEMP
#undef BED_MAXTEMP
#endif

#endif //__CONFIGURATION_ADV_H

