#ifndef TEMPERATURE_PID_TEMP_H
#define TEMPERATURE_PID_TEMP_H

namespace sg_grbl
{

#ifdef __cplusplus
  extern "C" {
#endif

    extern void temp_pid_extruder_set_kp(const float &p);
    extern void temp_pid_extruder_set_ki(const float &i);
    extern void temp_pid_extruder_set_kd(const float &d);
    extern void temp_pid_extruder_set_kc(const float &c);
    extern void temp_pid_extruder_update(void);
    extern void temp_pid_extruder_set_reset(const int &extruder, const bool &value);
    extern void temp_pid_extruder_calc_pid_output(const int &extruder, const float &pid_input, const float &pid_error, float &pid_output);

    extern void temp_pid_bed_update(void);

    extern void temp_pid_init(void);
#ifdef __cplusplus
  } // extern "C" {
#endif

}

#endif // TEMPERATURE_PID_TEMP_H

