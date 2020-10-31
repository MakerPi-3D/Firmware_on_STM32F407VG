
#ifndef temperature_h
#define temperature_h

namespace sg_grbl
{

#ifdef __cplusplus
  extern "C" {
#endif

#define NoError 0
#define HeatFailError 1
#define MaxTempError 2
#define MinTempError 3
#define MaxTempBedError 4
#define DETECT_PCB_FAULSE 5
#define ThermistorFallsOffError 6

    void temperature_init(void);            // 初始化加热系统
    void temperature_manage_heater(const int ext_fanspeed);   // 加热控制接口
    void temperature_disable_heater(void);  // 停止加热
    bool temperature_update(void);          // 温度更新，定时器
    int temperature_get_error_status(void); // 获取温度错误状态
    void temperature_set_error_status(char Value);
    bool temperature_update_pid_output_factor(void);
    int temperature_get_current_ext_fanspeed(void);
    void temperature_PID_autotune(float temp, int extruder, int ncycles);

    // 喷嘴温度函数接口
    float temperature_get_extruder_current(int extruder);
    float temperature_get_extruder_target(int extruder);
    void temperature_set_extruder_target(const float celsius, int extruder);
    bool temperature_is_extruder_heating(int extruder);
    bool temperature_is_extruder_cooling(int extruder);
    int temperature_get_extruder_heater_power(int heater);

    // 热床温度函数接口
    float temperature_get_bed_current(void);
    float temperature_get_bed_target(void);
    void temperature_set_bed_target(const float celsius);
    bool temperature_is_bed_heating(void);
    bool temperature_is_bed_cooling(void);
    int temperature_get_bed_heater_power(void);

    // 腔体温度函数接口
    float temperature_get_cavity_target(void);
    float temperature_get_cavity_current(void);
    void temperature_set_cavity_target(const float celsius);

    void temperature_set_heater_maxtemp(int axis, int value);
    int temperature_get_heater_maxtemp(int axis);

#ifdef __cplusplus
  } //extern "C" {
#endif

}

#endif

