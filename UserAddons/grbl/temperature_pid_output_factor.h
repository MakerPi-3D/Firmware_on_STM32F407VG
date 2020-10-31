#ifndef TEMPERATURE_PID_OUTPUT_FACTOR_H
#define TEMPERATURE_PID_OUTPUT_FACTOR_H

namespace sg_grbl
{

#ifdef __cplusplus
  extern "C" {
#endif

    void temp_accumulated_temp_per_second(const float &pid_input);
    void temp_reset_pid_output_factor_param(void);
    bool temp_cal_pid_output_factor(float &factor);
    float temp_get_pid_output_max_factor(void);

#ifdef __cplusplus
  } // extern "C" {
#endif

}

#endif // TEMPERATURE_PID_OUTPUT_FACTOR_H





