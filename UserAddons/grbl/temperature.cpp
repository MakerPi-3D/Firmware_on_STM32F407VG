
#include "temperature.h"
#include "temperature_pin.h"
#include "temperature_pid_output_factor.h"
#include "temperature_pid_temp.h"
#include "Configuration.h"
#include "config_model_tables.h"
#include <stdlib.h>
#include <string.h>
#include "ConfigurationStore.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "user_debug.h"
#include <arm_math.h>
#include "Alter.h"
#include "sg_util.h"
#include "globalvariables.h"
#include "sys_function.h"

namespace sg_grbl
{

  #ifdef __cplusplus
  extern "C" {
  #endif
#include "thermistortables.h"
  extern char TempMinErr_cnt;

  //===========================================================================
  //==============================public variables=============================
  //===========================================================================
  static volatile int16_t target_temperature[EXTRUDERS] = { 0 };
  static volatile int16_t target_temperature_bak[EXTRUDERS] = { 0 };
  static volatile int16_t target_temperature_bed = 0;
  static volatile uint32_t current_temperature_raw[EXTRUDERS] = { 0 };
  static volatile float current_temperature[EXTRUDERS] = { 0.0f };
  static volatile uint32_t current_temperature_bed_raw = 0;
  static volatile float current_temperature_bed = 0.0f;
  static volatile float current_temperature_cavity = 0.0f;
  static volatile uint32_t current_temperature_cavity_raw = 0;
  static volatile int16_t target_temperature_cavity = 0;
  static bool is_open_cavity = false;

  static volatile bool is_update_pid_output_factor = false;

  static int current_fanspeed = 0;
  static unsigned char detect_pcb_faulse_count = 10; // 检测电路板异常报警计数
  //===========================================================================
  //=============================private variables============================
  //===========================================================================

  #if EXTRUDERS > 3
# error Unsupported number of extruders
  #elif EXTRUDERS > 2
# define ARRAY_BY_EXTRUDERS(v1, v2, v3) { v1, v2, v3 }
  #elif EXTRUDERS > 1
# define ARRAY_BY_EXTRUDERS(v1, v2, v3) { v1, v2 }
  #else
# define ARRAY_BY_EXTRUDERS(v1, v2, v3) { v1 }
  #endif

  #ifndef SOFT_PWM_SCALE
#define SOFT_PWM_SCALE 0
  #endif

  static const int16_t heater_axis_mintemp[EXTRUDERS] = ARRAY_BY_EXTRUDERS(HEATER_0_MINTEMP, HEATER_1_MINTEMP, HEATER_2_MINTEMP);
  static int16_t heater_axis_maxtemp[EXTRUDERS] = ARRAY_BY_EXTRUDERS(HEATER_0_MAXTEMP, HEATER_1_MAXTEMP, HEATER_2_MAXTEMP);

  static int16_t bed_minttemp_raw = HEATER_BED_RAW_LO_TEMP;
  static int16_t bed_maxttemp_raw = HEATER_BED_RAW_HI_TEMP;

  static volatile bool temp_meas_ready = false;
  static unsigned char soft_pwm_bed;

  static unsigned long  previous_millis_bed_heater;
  static unsigned char soft_pwm[EXTRUDERS];

  // Init min and max temp with extreme values to prevent false errors during startup
  static int16_t minttemp_raw[EXTRUDERS] = ARRAY_BY_EXTRUDERS(HEATER_0_RAW_LO_TEMP, HEATER_1_RAW_LO_TEMP, HEATER_2_RAW_LO_TEMP);
  static int16_t maxttemp_raw[EXTRUDERS] = ARRAY_BY_EXTRUDERS(HEATER_0_RAW_HI_TEMP, HEATER_1_RAW_HI_TEMP, HEATER_2_RAW_HI_TEMP);
  static int16_t maxttemp_raw_temp[EXTRUDERS] = ARRAY_BY_EXTRUDERS(HEATER_0_RAW_HI_TEMP, HEATER_1_RAW_HI_TEMP, HEATER_2_RAW_HI_TEMP);
  static int16_t minttemp[EXTRUDERS] = ARRAY_BY_EXTRUDERS(0, 0, 0);
  static int16_t maxttemp[EXTRUDERS] = ARRAY_BY_EXTRUDERS(16383, 16383, 16383);

  static void *heater_ttbl_map[EXTRUDERS] = ARRAY_BY_EXTRUDERS((void *)HEATER_0_TEMPTABLE, (void *)HEATER_1_TEMPTABLE, (void *)HEATER_2_TEMPTABLE);
  static uint8_t heater_ttbllen_map[EXTRUDERS] = ARRAY_BY_EXTRUDERS(HEATER_0_TEMPTABLE_LEN, HEATER_1_TEMPTABLE_LEN, HEATER_2_TEMPTABLE_LEN);
  static float analog2temp(UINT32 raw, UINT8 e);
  static float analog2tempBed(UINT32 raw);
  static void updateTemperaturesFromRawValues(void);

  static uint8_t temp_error_status = NoError;


  //===========================================================================
  //=============================   functions      ============================
  //===========================================================================


  #ifdef PID_AUTOTUNE
  static unsigned char pid_finish;

  static void PID_autotune_finish()
  {
    pid_finish = 0x80;
    USER_EchoLog("PID Autotune finished! Put the Kp, Ki and Kd constants into Configuration.h");
  }

  static void PID_autotune_bad_extruder_num()
  {
    pid_finish = 0x81;
    USER_EchoLog("PID Autotune failed. Bad extruder number.");
  }

  static void PID_autotune_tmp_too_high()
  {
    pid_finish = 0x82;
    USER_EchoLog("PID Autotune failed! Temperature too high");
  }

  static void PID_autotune_timeout()
  {
    pid_finish = 0x83;
    USER_EchoLog("PID Autotune failed! Timeout");
  }
  #endif

  static void PID_autotune_serial_tmp(int extruder, float input)
  {
    if (extruder < 0)
    {
      USER_EchoLogStr("ok B:%f", input);
      USER_EchoLogStr(" @:%d", soft_pwm_bed);
    }
    else
    {
      USER_EchoLogStr("ok T:%f", input);
      USER_EchoLogStr(" @:%d", soft_pwm[extruder]);
    }

    USER_EchoLogStr("\r\n");
  }

  static void PID_autotune_serial_pid(long bias, long d, float max, float min, long t_high, long t_low, int cycles)
  {
    USER_EchoLogStr("cycles:%d; bias: %ld", cycles, bias);
    USER_EchoLogStr(" d: %ld", d);
    USER_EchoLogStr(" min: %f", min);
    USER_EchoLogStr(" max: %f\r\n", max);

    if (cycles > 2)
    {
      float Ku = (4.0f * d) / ((PI * (max - min)) / 2.0f); // float
      float Tu = ((float)(t_low + t_high) / 1000.0f); // float
      USER_EchoLogStr(" Ku: %f", Ku);
      USER_EchoLogStr(" Tu: %f", Tu);

      if (t_sys_data_current.enable_v5_extruder)
      {
        if (K5 == t_sys_data_current.model_id)
        {
          float Kp = 0.33f * Ku; // float
          float Ki = Kp / Tu; // float
          float Kd = (Kp * Tu) / 3; // float
          USER_EchoLogStr(" K5 ");
          USER_EchoLogStr(" Kp: %f", Kp);
          USER_EchoLogStr(" Ki: %f", Ki);
          USER_EchoLogStr(" Kd: %f\r\n", Kd);
          USER_EchoLog("M301 P%f I%f D%f", Kp, Ki, Kd);
        }
        else if (M3145T == t_sys_data_current.model_id)
        {
          float Kp = 0.33f * Ku; // float
          float Ki = Kp / Tu; // float
          float Kd = (Kp * Tu) / 8; // float
          USER_EchoLogStr(" K5 ");
          USER_EchoLogStr(" Kp: %f", Kp);
          USER_EchoLogStr(" Ki: %f", Ki);
          USER_EchoLogStr(" Kd: %f\r\n", Kd);
          USER_EchoLog("M301 P%f I%f D%f", Kp, Ki, Kd);
        }
        else
        {
          float Kp = 0.1f * Ku; // float
          float Ki = Kp / Tu; // float
          float Kd = (Kp * Tu) / 8; // float
          USER_EchoLogStr(" K5 ");
          USER_EchoLogStr(" Kp: %f", Kp);
          USER_EchoLogStr(" Ki: %f", Ki);
          USER_EchoLogStr(" Kd: %f\r\n", Kd);
          USER_EchoLog("M301 P%f I%f D%f", Kp, Ki, Kd);
        }
      }
      else
      {
        float Kp = 0.6f * Ku; // float
        float Ki = (2 * Kp) / Tu; // float
        float Kd = (Kp * Tu) / 8; // float
        USER_EchoLogStr(" Clasic PID ");
        USER_EchoLogStr(" Kp: %f", Kp);
        USER_EchoLogStr(" Ki: %f", Ki);
        USER_EchoLogStr(" Kd: %f\r\n", Kd);
        Kp = 0.33f * Ku; // float
        Ki = Kp / Tu; // float
        Kd = (Kp * Tu) / 3; // float
        USER_EchoLogStr(" Some overshoot ");
        USER_EchoLogStr(" Kp: %f", Kp);
        USER_EchoLogStr(" Ki: %f", Ki);
        USER_EchoLogStr(" Kd: %f\r\n", Kd);
        Kp = 0.2f * Ku; // float
        Ki = (2 * Kp) / Tu; // float
        Kd = (Kp * Tu) / 3; // float
        USER_EchoLogStr(" No overshoot ");
        USER_EchoLogStr(" Kp: %f", Kp);
        USER_EchoLogStr(" Ki: %f", Ki);
        USER_EchoLogStr(" Kd: %f\r\n", Kd);
      }
    }
  }

  //M303 Exx Cxx
  //对于喷嘴0,设定温度150,在5个周期后自动调节PID参数,并串口输出PID
  //需要手动记录PID
  void temperature_PID_autotune(float temp, int extruder, int ncycles)
  {
    float input = 0.0;
    int cycles = 0;
    bool heating = true;
    unsigned long temp_millis = sys_task_get_tick_count();
    unsigned long t1 = temp_millis;
    unsigned long t2 = temp_millis;
    long t_high = 0;
    long t_low = 0;
    long bias, d;
    #ifdef PID_AUTOTUNE
    float Ku, Tu;
    float Kp, Ki, Kd;
    #endif
    float max = 0, min = 10000;
    #ifdef PID_AUTOTUNE
    pid_finish = 0;
    #endif

    if ((extruder > EXTRUDERS) || (extruder < 0))
    {
      #ifdef PID_AUTOTUNE
      PID_autotune_bad_extruder_num();
      #endif
      return;
    }

    USER_EchoLog("PID Autotune start");
    temperature_disable_heater(); // switch off all heaters.

    if (extruder < 0)
    {
      soft_pwm_bed = (MAX_BED_POWER) / 2;
      bias = d = (MAX_BED_POWER) / 2;
    }
    else
    {
      soft_pwm[extruder] = (PID_MAX) / 2; // float
      bias = d = (PID_MAX) / 2; // float
    }

    for (;;)
    {
      if (temp_meas_ready == true)  // temp sample ready
      {
        updateTemperaturesFromRawValues();
        input = (extruder < 0) ? temperature_get_bed_current() : temperature_get_extruder_current(extruder);
        max = sg_util::max_p(max, input);
        min = sg_util::min_p(min, input);

        if ((heating == true) && (input > temp))
        {
          if ((sys_task_get_tick_count() - t2) > 5000)
          {
            heating = false;

            if (extruder < 0)
            {
              soft_pwm_bed = (bias - d) >> 1;
            }
            else
            {
              soft_pwm[extruder] = (bias - d) >> 1;
            }

            t1 = sys_task_get_tick_count();
            t_high = t1 - t2;
            max = temp;
          }
        }

        if ((heating == false) && (input < temp))
        {
          if ((sys_task_get_tick_count() - t1) > 5000)
          {
            heating = true;
            t2 = sys_task_get_tick_count();
            t_low = t2 - t1;

            if (cycles > 0)
            {
              bias += (d * (t_high - t_low)) / (t_low + t_high);
              bias = sg_util::constrain_p(bias, (long)20, (long)((extruder < 0) ? (MAX_BED_POWER) : (PID_MAX)) - (long)20);

              if (bias > (((extruder < 0) ? (MAX_BED_POWER) : (PID_MAX)) / 2)) // float
              {
                d = (((extruder < 0) ? (MAX_BED_POWER) : (PID_MAX)) - 1) - bias; // float
              }
              else
              {
                d = bias;
              }

              PID_autotune_serial_pid(bias, d, max, min, t_high, t_low, cycles);
            }

            if (extruder < 0)
            {
              soft_pwm_bed = (bias + d) >> 1;
            }
            else
            {
              soft_pwm[extruder] = (bias + d) >> 1;
            }

            ++cycles;
            min = temp;
          }
        }
      }

      if (input > (temp + 50)) // float
      {
        #ifdef PID_AUTOTUNE
        PID_autotune_tmp_too_high();
        #endif
        return;
      }

      if ((sys_task_get_tick_count() - temp_millis) > 2000)
      {
        PID_autotune_serial_tmp(extruder, input);
        temp_millis = sys_task_get_tick_count();
      }

      if (((sys_task_get_tick_count() - t1) + (sys_task_get_tick_count() - t2)) > (10L * 60L * 1000L * 2L))
      {
        #ifdef PID_AUTOTUNE
        PID_autotune_timeout();
        #endif
        return;
      }

      if (cycles > ncycles)
      {
        #ifdef PID_AUTOTUNE
        PID_autotune_finish();
        #endif
        return;
      }
    }
  }

  static inline void temp_extruders_init(void)
  {
    // Finish init of mult extruder arrays
    for (INT16 e = 0; e < EXTRUDERS; ++e)
    {
      // populate with the first value
      maxttemp[e] = maxttemp[0];
      minttemp[e] = heater_axis_mintemp[e];

      while (analog2temp(minttemp_raw[e], e) < heater_axis_mintemp[e])
      {
        if (minttemp_raw[e] < maxttemp_raw[e])
        {
          minttemp_raw[e] += OVERSAMPLENR;
        }
        else
        {
          minttemp_raw[e] -= OVERSAMPLENR;
        }
      }

      maxttemp[e] = heater_axis_maxtemp[e];

      while (analog2temp(maxttemp_raw[e], e) > heater_axis_maxtemp[e])
      {
        if (minttemp_raw[e] < maxttemp_raw[e])
        {
          maxttemp_raw[e] -= OVERSAMPLENR;
        }
        else
        {
          maxttemp_raw[e] += OVERSAMPLENR;
        }
      }
    }
  }

  void temperature_set_heater_maxtemp(int axis, int value)
  {
    if ((axis < EXTRUDERS) && (axis >= 0))
    {
      heater_axis_maxtemp[axis] = value;
      maxttemp_raw[axis] = maxttemp_raw_temp[axis];
      temp_extruders_init();
    }
  }

  int temperature_get_heater_maxtemp(int axis)
  {
    if ((axis < EXTRUDERS) && (axis >= 0))
    {
      return heater_axis_maxtemp[axis];
    }
    else
    {
      return -1;
    }
  }

  int temperature_get_current_ext_fanspeed(void)
  {
    return current_fanspeed;
  }

  int temperature_get_error_status(void)
  {
    return temp_error_status;
  }

  void temperature_set_error_status(char Value)
  {
    temp_error_status = Value;
  }
#define TEMP_LIST_SIZE 20

  typedef struct
  {
    int8_t level;//加热状态  1:加温  2:恒温  3:降温
    float temp_curr;//当前温度
    float temp_diff;//温度差
  } Temp_List;

  uint8_t tempListIndex = 0;
  Temp_List temp_list0[TEMP_LIST_SIZE];

  void clr_tempList()
  {
    tempListIndex = 0;

    //  tempCheckTrigger=false;
    for (int i = 0; i < TEMP_LIST_SIZE; ++i)
    {
      temp_list0[i].temp_curr = 0;
      temp_list0[i].level = 0.0f;
      temp_list0[i].temp_diff = 0.0f;
    }
  }

  //10个数，去掉最高和最低，求平均
  static float calc_ave(const float *const p, const int size)
  {
    float sum = 0;
    float max = p[0];
    float min = p[0];

    for (int i = 0; i < size; ++i)
    {
      sum += p[i];

      if (max < p[i])
      {
        max = p[i];
      }

      if (min > p[i])
      {
        min = p[i];
      }
    }

    //  SERIAL_ECHOPGM("1 ");
    sum -= max; //去除最大值
    sum -= min; //去除最大值
    //  SERIAL_ECHOPGM("2 ");
    return sum / (size - 2);
  }

  //20个数，比较前十个与后十个的平均值之差
  static float calc_diff(const float *p, const int size)
  {
    return calc_ave(p + (size / 2), size / 2) - calc_ave(p, size / 2);
  }

  /*
  迟滞比较

               |<-------^
        反向触发 |        | 正向触发
             A___v_______>|____B
                tr       tf
    reverse_trigger   forward_trigger

  */

  //suzhiwei 20160409: 迟滞比较器 hysteresis comparator
  bool hysteresis(bool &trigger, const int current, const int  min, const int max)
  {
    //靠近目标，正向触发
    if ((!trigger) && (current < min))
    {
      trigger = true;
    }

    //远离目标，反向触发
    if (trigger && (current > max))
    {
      trigger = false;
    }

    return trigger;
  }

  void resetTrigger(bool &trigger)
  {
    trigger = false;
  }

  void temp_list_check(Temp_List p_temp_list[])
  {
    static char error1_cnt = 0;
    const int level = p_temp_list[0].level; //温度变化状态取首个状态为参考
    int temp_log_index = 0;
    float temp_log[TEMP_LIST_SIZE] = {0};

    //提取最大的有效温度记录值（温度变化状态必须相同）
    while ((temp_log_index < TEMP_LIST_SIZE) && (p_temp_list[temp_log_index].level == level))
    {
      temp_log[temp_log_index] = p_temp_list[temp_log_index].temp_curr;
      ++temp_log_index;
    }

    //如果处于相同阶段的温度记录值<规定的有效记录数, 则不判断
    if (temp_log_index < 11)
    {
      return;
    }

    if (1 == level) //升温 2：恒温 3：降温
    {
      float compare_temp_diff = 1.5f;

      //计算level_max*1s 时间内后半段与前半段的温度（过滤max和min）均差，小于2则认为加热失败
      if (0U != t_sys_data_current.enable_color_mixing)
      {
        if ((M2048 == t_sys_data_current.model_id) || (M2030 == t_sys_data_current.model_id) || (K5 == t_sys_data_current.model_id))
        {
          compare_temp_diff = 1.0f;
        }
      }

      float diff_temp = calc_diff(temp_log, temp_log_index);

      if (((diff_temp * TEMP_LIST_SIZE) / temp_log_index) < compare_temp_diff)
      {
        ++error1_cnt;

        //连续2次，触发错误提醒
        if (error1_cnt >= 2)
        {
          //          Stop();// 20s*3=180s=1min
          error1_cnt = 0;
          #if LASER_MODE

          if (!t_sys_data_current.IsLaser)
          #endif
          {
            temperature_disable_heater();
            temp_error_status = HeatFailError;
          }
        }
      }
      else
      {
        error1_cnt = 0;
      }
    }
  }


  //获取temp_list
  void temp_list_get()
  {
    static char tempLevel = 0;
    static int target_temperature_old = 0;
    float temp_diff = (float)target_temperature[0] - current_temperature[0]; //与目标温度差距

    //目标温度小于50度 或 温度差2度 则为非重要检测，复位并返回
    if ((target_temperature[0] < 50) || (abs(temp_diff) < 2))
    {
      clr_tempList();
      return;
    }

    //如果改变了目标温度则复位检测
    if (target_temperature_old != target_temperature[0])
    {
      target_temperature_old = target_temperature[0];
      clr_tempList();
      return;
    }

    //根据error是否在>20、-10~20、<-10，将level赋值1~3：升温1 恒温（恒温阶段去掉温差小于2的部分）2 降温3,
    tempLevel = (temp_diff > 20) ? 1 : ((temp_diff > - 10) ? 2 : 3);

    if (tempLevel > 2) //只检测升温1 恒温2
    {
      //降温则复位检测
      clr_tempList();
      return;
    }

    if ((tempListIndex >= 1) && (tempListIndex < 10) && (tempLevel != temp_list0[tempListIndex - 1].level))
    {
      //如果温度状态改变，则复位检测
      clr_tempList();
      return;
    }

    temp_list0[tempListIndex].level =  tempLevel;
    temp_list0[tempListIndex].temp_diff = temp_diff;
    temp_list0[tempListIndex].temp_curr = current_temperature[0];
    ++tempListIndex;//每秒下一个数组元素

    if (tempListIndex >= TEMP_LIST_SIZE)
    {
      temp_list_check(temp_list0);//每TEMP_LIST_SIZE秒检查一次
      //检测完后复位检测
      clr_tempList();
      return;
    }
  }

  #ifdef CHECK_EXTRUDER_THERMISTOR_FALLS_OFF
  //#include "user_debug.h"
  // 检测喷嘴热敏电阻是否脱离
  // 上电加热30S，如果第10S到第30S加热温度低于20度，判断热敏电阻脱离
  static int detect_time_count = 0;
  static unsigned char detect_onece = 1; // 上电只检测一次

  static void detect_extruder_thermistor_falls_off(const float &pid_input, const float &pid_error)
  {
    static unsigned long detect_get_peroid = 0;
    static float extruder_temp_first = 0.0f;

    // 如果目标温度与当前温度小于50，以下检测逻辑不成立，退出
    if (!detect_onece)
    {
      return;
    }

    if (10 == detect_time_count)
    {
      extruder_temp_first = pid_input;
    }

    if (detect_get_peroid < sys_task_get_tick_count())
    {
      detect_get_peroid = sys_task_get_tick_count() + 1000;
      ++detect_time_count;
      //      USER_ErrLog("current_temp:%f;save_temp:%f", pid_input, extruder_temp_first);
    }

    if (15 == detect_time_count)
    {
      detect_time_count = 0;

      // 混色机器加热可能无法达到20度，波动范围为15到20度之间，取最小值
      // 单色机器加热可以超过20度，但测试数据不多，无法确定
      // V5喷嘴机器与混色机器类似，需要测试
      // 注意：该测试只针对30W、40W、45W功率加热棒，其他情况需要测试
      if ((pid_input - extruder_temp_first) < 4)
      {
        temp_error_status = ThermistorFallsOffError;
      }

      detect_onece = 0;
    }
  }
  #endif

  void cal_pid_output(const INT16 &extruder, const float &pid_input, float &pid_output)
  {
    float pct_of_extruder_tmp_power = 1.0f; // 单色喷嘴温度功率百分比
    float pid_error = target_temperature[extruder] - pid_input;

    if (target_temperature_bak[extruder] != target_temperature[extruder])
    {
      target_temperature_bak[extruder] = target_temperature[extruder];
      temp_reset_pid_output_factor_param();
    }

    // 当前与目标温度之差小于0，重置温度系数参数
    if (pid_error < 0)
    {
      temp_reset_pid_output_factor_param();
    }

    #ifdef CHECK_EXTRUDER_THERMISTOR_FALLS_OFF

    if (1 == t_sys.is_detect_extruder_thermistor)
    {
      if (detect_onece && (pid_error <= (t_sys_data_current.enable_color_mixing ? 45 : 50)))
      {
        detect_time_count = 0;
      }
    }

    #endif

    if (pid_error > (t_sys_data_current.enable_color_mixing ? 45 : 50))
    {
      #ifdef CHECK_EXTRUDER_THERMISTOR_FALLS_OFF

      if (1 == t_sys.is_detect_extruder_thermistor)
      {
        detect_extruder_thermistor_falls_off(pid_input, pid_error);
      }

      #endif
      temp_accumulated_temp_per_second(pid_input);
      pid_output = BANG_MAX;
      temp_pid_extruder_set_reset(extruder, true);
    }
    else if (pid_error > (t_sys_data_current.enable_color_mixing ? 25 : 32))
    {
      float factor = t_sys_data_current.pid_output_factor;
      pct_of_extruder_tmp_power = 0.85f;

      if (temp_cal_pid_output_factor(factor)) // 计算pid_output输出系数
      {
        t_sys_data_current.pid_output_factor = factor;
        is_update_pid_output_factor = true;
      }

      // V5喷嘴一直开启扇热风扇，可能会影响加热速率，触发加热失败，M41G出现这种情况
      if (1 == t_sys_data_current.enable_v5_extruder)
      {
        pct_of_extruder_tmp_power = 1.0f / t_sys_data_current.pid_output_factor; // float
      }

      pid_output = (current_fanspeed) ? (BANG_MAX) : (BANG_MAX * pct_of_extruder_tmp_power * t_sys_data_current.pid_output_factor); // float
      temp_pid_extruder_set_reset(extruder, true);
    }
    else if (((0 == t_sys_data_current.enable_v5_extruder) || (K5 == t_sys_data_current.model_id)) && (pid_error > PID_FUNCTIONAL_RANGE))
    {
      pct_of_extruder_tmp_power = 0.75f;

      if (t_sys_data_current.enable_color_mixing) // 混色喷嘴温度功率百分比
      {
        pct_of_extruder_tmp_power = 0.75f;
      }

      // V5喷嘴一直开启扇热风扇，可能会影响加热速率，触发加热失败，M41G出现这种情况
      if (t_sys_data_current.enable_v5_extruder) // V5喷嘴温度功率百分比
      {
        pct_of_extruder_tmp_power = 1.0f / t_sys_data_current.pid_output_factor; // float
      }

      pid_output = (current_fanspeed) ? (BANG_MAX) : (BANG_MAX * t_sys_data_current.pid_output_factor * pct_of_extruder_tmp_power); // float

      // 混色噴嘴冬天加熱不上，功率為30w，取最大加熱功率
      if ((t_sys_data_current.enable_color_mixing) && (t_sys_data_current.pid_output_factor == temp_get_pid_output_max_factor()))
      {
        pid_output = BANG_MAX;
      }

      temp_pid_extruder_set_reset(extruder, true);
    }
    /*平滑加热曲线[end]  author:yangh@soongon.com date:20150324*/
    else if ((pid_error < (-PID_FUNCTIONAL_RANGE - ((t_sys_data_current.enable_v5_extruder && (K5 != t_sys_data_current.model_id)) ? 5 : 0))) || (target_temperature[extruder] == 0))
    {
      pid_output = 0;
      temp_pid_extruder_set_reset(extruder, true);
    }
    else
    {
      temp_pid_extruder_calc_pid_output(extruder, pid_input, pid_error, pid_output);
    }
  }


  // 当前温度高于目标温度时，控制风扇，避免温度偏离大
  static void manage_fan_to_high_temp(const INT16 &extruder, const float &pid_input, const int &HEATER_MAXTEMP)
  {
    static UINT8 fanSpeedSave = 0; // 保存风扇速度
    static UINT8 isResetFanSpeed = 0; // 是否自动设置风扇速度状态

    if (pid_input >= (HEATER_MAXTEMP + 5))  //温度超过最大温度，开风扇
    {
      fanSpeedSave = current_fanspeed;
      isResetFanSpeed = 1;
      current_fanspeed = 255;
    }
    else
    {
      if (isResetFanSpeed)
      {
        current_fanspeed = fanSpeedSave;
        isResetFanSpeed = 0;
      }
    }

    if ((target_temperature[extruder] > 0) && ((target_temperature[extruder] - current_temperature[extruder]) > 15)) // float
    {
      if ((K5 == t_sys_data_current.model_id) && (14 == t_sys_data_current.custom_model_id))
      {
        return;
      }

      if (current_fanspeed > 100)
      {
        fanSpeedSave = current_fanspeed;
        isResetFanSpeed = 1;
        current_fanspeed = 100;  //目标温度不为0，温度小于目标温度15度，风扇又开太大，则把风扇关小，防止加热失败
      }
    }
    else if ((target_temperature[extruder] > 0) && ((target_temperature[extruder] - current_temperature[extruder]) > 8)) // float
    {
      if ((K5 == t_sys_data_current.model_id) && (14 == t_sys_data_current.custom_model_id))
      {
        return;
      }

      if (current_fanspeed > 150)
      {
        fanSpeedSave = current_fanspeed;
        isResetFanSpeed = 1;
        current_fanspeed = 150;  //目标温度不为0，温度小于目标温度10度，风扇又开太大，则把风扇关小，防止加热失败
      }
    }
    else
    {
      if (isResetFanSpeed)
      {
        current_fanspeed = fanSpeedSave;
        isResetFanSpeed = 0;
      }
    }
  }

  static void manage_extruders_heater(void)
  {
    float pid_input = 0.0f;
    float pid_output = 0.0f;
    int heater_maxtemp = 0;
    static unsigned long temp_list_get_peroid = 0;

    for (INT16 e = 0; e < EXTRUDERS; ++e)
    {
      pid_input = current_temperature[e];
      heater_maxtemp = heater_axis_maxtemp[e];

      if (t_sys_data_current.enable_color_mixing)
      {
        heater_maxtemp -= 10;
      }

      // 当前温度高于目标温度时，控制风扇，避免温度偏离大
      if (PICTURE_IS_JAPANESE != t_sys_data_current.pic_id)
      {
        manage_fan_to_high_temp(e, pid_input, heater_maxtemp);
      }

      // 计算pid_output
      cal_pid_output(e, pid_input, pid_output);
      // Check if temperature is within the correct range
      #if LASER_MODE

      if (!t_sys_data_current.IsLaser)
      {
        // Check if temperature is within the correct range
        if ((current_temperature[e] > minttemp[e]) && (current_temperature[e] < maxttemp[e]))
        {
          soft_pwm[e] = (unsigned char)((int)pid_output >> 1);
        }
        else
        {
          soft_pwm[e] = 0;
        }
      }
      else
      #endif
      {
        // Check if temperature is within the correct range
        if (current_temperature[e] < maxttemp[e])
        {
          soft_pwm[e] = (unsigned char)((int)pid_output >> 1);
        }
        else
        {
          soft_pwm[e] = 0;
        }
      }

      if (temp_list_get_peroid <= sys_task_get_tick_count())
      {
        temp_list_get();
        temp_list_get_peroid = sys_task_get_tick_count() + 1000; //每1s获取数据
      }
    } // End extruder for loop
  }

  static void manage_bed_heater(void)
  {
    #if defined(BED_MAXTEMP) && (TEMP_SENSOR_BED != 0)

    // Check if temperature is within the correct range
    if ((current_temperature_bed > BED_MINTEMP) && (current_temperature_bed < BED_MAXTEMP))
    {
      if (current_temperature_bed >= target_temperature_bed)
      {
        soft_pwm_bed = 0;
      }
      else
      {
        soft_pwm_bed = MAX_BED_POWER >> 1;
      }
    }
    else
    {
      soft_pwm_bed = 0;
      temperature_set_bed_heater_status(false);
    }

    #endif // defined(BED_MAXTEMP) && (TEMP_SENSOR_BED != 0)
  }

  static void manage_cavity_heater(void)
  {
    if (current_temperature_cavity >= target_temperature_cavity)
    {
      temperature_set_cavity_pin_status(false);
    }
    else
    {
      temperature_set_cavity_pin_status(true);
    }
  }

  void temperature_manage_heater(const int ext_fanspeed)
  {
    if (temp_meas_ready != true)  //better readability
    {
      return;
    }

    current_fanspeed = ext_fanspeed;
    updateTemperaturesFromRawValues();

    if (current_temperature[0] > (heater_axis_maxtemp[0] + 10)) //当前温度大于最大温度就发出警报，并且不能关闭警报
    {
      if (detect_pcb_faulse_count == 0)
      {
        temp_error_status = DETECT_PCB_FAULSE;
      }

      detect_pcb_faulse_count--;
    }

    // 喷嘴加热控制
    manage_extruders_heater();

    if ((sys_task_get_tick_count() - previous_millis_bed_heater) < BED_CHECK_INTERVAL)
    {
      return;
    }

    previous_millis_bed_heater = sys_task_get_tick_count();
    // 热床加热控制
    manage_bed_heater();
    sys_os_delay(50);

    // 腔体温度控制
    if (is_open_cavity)
    {
      manage_cavity_heater();
    }
  }


  // Derived from RepRap FiveD extruder::getTemperature()
  // For hot end temperature measurement.
  static float analog2temp(UINT32 raw, UINT8 e)
  {
    if (e >= EXTRUDERS)
    {
      temperature_disable_heater();
      return 0.0;
    }

    if (heater_ttbl_map[e] != NULL)
    {
      float celsius = 0;
      UINT8 i;
      short(*tt)[][2] = (short(*)[][2])(heater_ttbl_map[e]);

      for (i = 1; i < heater_ttbllen_map[e]; ++i)
      {
        if ((*tt)[i][0] > raw)
        {
          celsius = (*tt)[i - 1][1] + // float
                    (((raw - (*tt)[i - 1][0]) * // float
                      (float)((*tt)[i][1] - (*tt)[i - 1][1])) / // float
                     (float)((*tt)[i][0] - (*tt)[i - 1][0])); // float
          break;
        }
      }

      // Overflow: Set to last value in the table
      if (i == heater_ttbllen_map[e])
      {
        celsius = (*tt)[i - 1][1];
      }

      return celsius;
    }

    return ((raw * ((5.0f * 100.0f) / 1024.0f)) / OVERSAMPLENR); // float
  }

  // Derived from RepRap FiveD extruder::getTemperature()
  // For bed temperature measurement.
  static float analog2tempBed(UINT32 raw)
  {
    float celsius = 0;
    UINT8 i;

    for (i = 1; i < BEDTEMPTABLE_LEN; ++i)
    {
      if (BEDTEMPTABLE[i][0] > raw)
      {
        celsius  = BEDTEMPTABLE[i - 1][1] + // float
                   (((raw - BEDTEMPTABLE[i - 1][0]) * // float
                     (float)(BEDTEMPTABLE[i][1] - BEDTEMPTABLE[i - 1][1])) / // float
                    (float)(BEDTEMPTABLE[i][0] - BEDTEMPTABLE[i - 1][0])); // float
        break;
      }
    }

    // Overflow: Set to last value in the table
    if (i == BEDTEMPTABLE_LEN)
    {
      celsius = BEDTEMPTABLE[i - 1][1];
    }

    return celsius;
  }

  /* Called to get the raw values into the the actual temperatures. The raw values are created in interrupt context,
      and this function is called from normal context as it is too slow to run in interrupts and will block the stepper routine otherwise */
  static void updateTemperaturesFromRawValues()
  {
    for (UINT8 e = 0; e < EXTRUDERS; ++e)
    {
      // 定制机型9、12用热电偶，660度对应3.3v
      if (1 == t_sys_data_current.enable_type_of_thermistor)
      {
        current_temperature[e] = ((current_temperature_raw[e] * 660.0f) / 1024.0f) / OVERSAMPLENR; // float
      }
      else
      {
        current_temperature[e] = analog2temp(current_temperature_raw[e], e);
      }
    }

    current_temperature_bed = analog2tempBed(current_temperature_bed_raw);

    if (is_open_cavity)
    {
      current_temperature_cavity  = analog2tempBed(current_temperature_cavity_raw);
    }

    sys_task_enter_critical();
    temp_meas_ready = false;
    sys_task_exit_critical();
  }

  void temperature_init()
  {
    temp_extruders_init();
    #ifdef BED_MAXTEMP

    while (analog2tempBed(bed_maxttemp_raw) > BED_MAXTEMP)
    {
      if (bed_minttemp_raw < bed_maxttemp_raw)
      {
        bed_maxttemp_raw -= OVERSAMPLENR;
      }
      else
      {
        bed_maxttemp_raw += OVERSAMPLENR;
      }
    }

    #endif //BED_MAXTEMP
    temp_pid_init();
    // Interleave temperature interrupt with millies interrupt
    temperature_timer_start();
    is_open_cavity = t_sys_data_current.enable_cavity_temp;
    temperature_cavity_pin_init(is_open_cavity);
  }

  void temperature_disable_heater()
  {
    target_temperature_bed = 0.0f;

    if (is_open_cavity)
    {
      target_temperature_cavity = 0.0f;
    }

    soft_pwm_bed = 0;
    temperature_set_bed_heater_status(false);

    for (INT16 i = 0; i < EXTRUDERS; ++i)
    {
      target_temperature[i] = 0.0f;
      soft_pwm[i] = 0;
      temperature_set_extruders_heater_status(false, i);
    }
  }

  void temp_status_switch(unsigned long &raw_temp_0_value, unsigned long &raw_temp_bed_value, unsigned long &raw_temp_cavity_value, unsigned char &temp_count)
  {
    static unsigned char temp_state = 10;

    switch (temp_state)
    {
    case 0: // Prepare TEMP_0
      temperature_adc_status_switch(0);
      temp_state = 1;
      break;

    case 1: // Measure TEMP_0
      raw_temp_0_value += temperature_adc_get_value(0);
      temp_state = 2;
      break;

    case 2: // Prepare TEMP_BED
      temperature_adc_status_switch(1);
      temp_state = 3;
      break;

    case 3: // Measure TEMP_BED
      raw_temp_bed_value += temperature_adc_get_value(1);
      temp_state = 4;
      break;

    case 4: // Prepare TEMP_1
      if (is_open_cavity)
      {
        temperature_adc_status_switch(2);
      }

      temp_state = 5;
      break;

    case 5: // Prepare TEMP_1
      if (is_open_cavity)
      {
        raw_temp_cavity_value += temperature_adc_get_value(2);
      }

      temp_state = 6;
      break;

    case 6: // Prepare TEMP_2
      temp_state = 7;
      break;

    case 7: // Measure TEMP_2
      temp_state = 8;
      break;

    case 8:
      temp_state = 9;
      break;

    case 9:
      temp_state = 0;
      ++temp_count;
      break;

    case 10:
      temp_state = 0;
      break;

    default:
      break;
    }
  }

  static void temperature_extruder_temp_check_error(int extruder)
  {
    if (((minttemp_raw[extruder] > maxttemp_raw[extruder]) && (current_temperature_raw[extruder] <= maxttemp_raw[extruder])) ||
        ((minttemp_raw[extruder] <= maxttemp_raw[extruder]) && (current_temperature_raw[extruder] >= maxttemp_raw[extruder])))
    {
      temperature_disable_heater();
      temp_error_status = MaxTempError;
    }

    if (((minttemp_raw[extruder] > maxttemp_raw[extruder]) && (current_temperature_raw[extruder] >= minttemp_raw[extruder])) ||
        ((minttemp_raw[extruder] <= maxttemp_raw[extruder]) && (current_temperature_raw[extruder] <= minttemp_raw[extruder])))
    {
      if (Check_TempMinErr() && !t_sys_data_current.IsLaser)
      {
        temperature_disable_heater();
        temp_error_status = MinTempError;
      }
    }
    else
    {
      TempMinErr_cnt = 0;
    }
  }

  static void temperature_bed_temp_check_error(void)
  {
    #if defined(BED_MAXTEMP) && (TEMP_SENSOR_BED != 0)

    if (((bed_minttemp_raw > bed_maxttemp_raw) && (current_temperature_bed_raw <= bed_maxttemp_raw)) ||
        ((bed_minttemp_raw <= bed_maxttemp_raw) && (current_temperature_bed_raw >= bed_maxttemp_raw)))
    {
      target_temperature_bed = 0;
      temperature_set_bed_heater_status(false);
      temperature_disable_heater();
      temp_error_status = MaxTempBedError;
    }

    #endif // defined(BED_MAXTEMP) && (TEMP_SENSOR_BED != 0)
  }

  void tp_software_pwm(void)
  {
    static unsigned char pwm_count = (1 << SOFT_PWM_SCALE);
    static unsigned char soft_pwm_0;
    static unsigned char soft_pwm_b;
    #if LASER_MODE

    if (!t_sys_data_current.IsLaser)
    #endif
    {
      if (pwm_count == 0)
      {
        soft_pwm_0 = soft_pwm[0];

        if (soft_pwm_0 > 0)
          temperature_set_extruders_heater_status(true, 0);

        soft_pwm_b = soft_pwm_bed;

        if (soft_pwm_b > 0)
          temperature_set_bed_heater_status(true);
      }

      if (soft_pwm_0 <= pwm_count)
        temperature_set_extruders_heater_status(false, 0);

      if (soft_pwm_b <= pwm_count)
        temperature_set_bed_heater_status(false);

      pwm_count += (1 << SOFT_PWM_SCALE);
      pwm_count &= 0x7f;
    }

    #if LASER_MODE
    else
    {
      if (target_temperature[0] > current_temperature[0])
        temperature_set_extruders_heater_status(true, 0);
      else
        temperature_set_extruders_heater_status(false, 0);
    }

    #endif
  }

  bool temperature_update(void)
  {
    //these variables are only accesible from the ISR, but static, so they don't lose their value
    static unsigned char temp_count = 0;
    static unsigned long raw_temp_0_value = 0;
    static unsigned long raw_temp_bed_value = 0;
    static unsigned long raw_temp_cavity_value = 0;
    tp_software_pwm();
    temp_status_switch(raw_temp_0_value, raw_temp_bed_value, raw_temp_cavity_value, temp_count);

    if (temp_count >= OVERSAMPLENR)  // 10 * 16 * 1/(16000000/64/256)  = 164ms.For stduino 10/2 * 16 * 1/1000 = 80ms
    {
      if (!temp_meas_ready)   //Only update the raw values if they have been read. Else we could be updating them during reading.
      {
        current_temperature_raw[0] = (INT16)raw_temp_0_value;
        current_temperature_bed_raw = (INT16)raw_temp_bed_value;

        if (is_open_cavity)
        {
          current_temperature_cavity_raw = (INT16)raw_temp_cavity_value;
        }
      }

      temp_meas_ready = true;
      temp_count = 0;
      raw_temp_0_value = 0;
      raw_temp_bed_value = 0;

      if (is_open_cavity)
      {
        raw_temp_cavity_value = 0;
      }

      temperature_extruder_temp_check_error(0);
      /* No bed MINTEMP error? */
      temperature_bed_temp_check_error();
      return true;
    }
    else
    {
      return false;
    }
  }

  float temperature_get_extruder_current(int extruder)
  {
    return current_temperature[extruder];
  }

  float temperature_get_bed_current(void)
  {
    return current_temperature_bed;
  }

  float temperature_get_extruder_target(int extruder)
  {
    return target_temperature[extruder];
  }

  float temperature_get_bed_target(void)
  {
    return target_temperature_bed;
  }

  void temperature_set_extruder_target(const float celsius, int extruder)
  {
    target_temperature[extruder] = celsius;
    #if LASER_MODE

    if (t_sys_data_current.IsLaser && (target_temperature[extruder] > 60))
      target_temperature[extruder] = 60;

    #endif
  }

  void temperature_set_bed_target(const float celsius)
  {
    target_temperature_bed = celsius;
  }

  float temperature_get_cavity_target(void)
  {
    return target_temperature_cavity;
  }

  void temperature_set_cavity_target(const float celsius)
  {
    if (is_open_cavity)
    {
      target_temperature_cavity = celsius;
    }
  }

  float temperature_get_cavity_current(void)
  {
    return current_temperature_cavity;
  }

  bool temperature_is_extruder_heating(int extruder)
  {
    return target_temperature[extruder] > current_temperature[extruder];
  }

  bool temperature_is_bed_heating(void)
  {
    return target_temperature_bed > current_temperature_bed;
  }

  bool temperature_is_extruder_cooling(int extruder)
  {
    return target_temperature[extruder] < current_temperature[extruder];
  }

  bool temperature_is_bed_cooling(void)
  {
    return target_temperature_bed < current_temperature_bed;
  }

  int temperature_get_extruder_heater_power(int heater)
  {
    return soft_pwm[heater];
  }

  int temperature_get_bed_heater_power(void)
  {
    return soft_pwm_bed;
  }

  bool temperature_update_pid_output_factor(void)
  {
    bool factor = is_update_pid_output_factor;
    is_update_pid_output_factor = false;
    return factor;
  }
  #ifdef __cplusplus
}
  #endif

}

