#include "temperature_pid_output_factor.h"
#include "threed_engine.h"
#include "Configuration.h"
#include "sys_function.h"

namespace sg_grbl
{

  #ifdef __cplusplus
  extern "C" {
  #endif

  static const float PID_OUTPUT_MAX_FACTOR = 1.0f / 0.85f;  /*!< 定义pid_output最大温度输出系数 */
  static const float MAX_PID_INPUT_DIFF = 3.0f;             /*!< 定义最大温度差值 */
  static const float EXTRUDER_ACCUNULATE_INTERVAL = 10.0f;  /*!< 喷嘴每秒温度差累加，间隔10s开始累加 */
  static unsigned long pid_input_get_peroid = 0;            /*!< 温度输入获取间隔 */
  static unsigned long pid_input_time_count = 0;            /*!< 温度输入计数（秒） */
  static float pid_input_pre = 0.0f;                        /*!< 上一个温度输入 */
  static float pid_input_diff_count = 0.0f;                 /*!< 温度输入差值累计 */
  static bool has_max_pid_input_diff = false;               /*!< 是否拥有最大温度输入差值。温度输入差值大于3，参数为真 */
  static float max_pid_input_diff = 0.0f;                   /*!< has_max_pid_input_diff为真，温度输入差值大于0 */

  // 当功率小时，喷嘴温度加热慢，可能出现加热失败；当功率大时，喷嘴加热太快，可能触发最大温度报警
  // 温度控制步骤：
  // 1、当温度pid_input每秒差值小于3时，获取温度差值累计pid_input_diff_count，及对应秒数增加pid_input_time_count，
  //     得到每秒温度差值均值pid_input_diff_count/pid_input_time_count
  // 2、当温度pid_input每秒差值大于3时，获取温度差值最大值max_pid_input_diff
  // 3、求pid_output缩放系数pid_output_factor
  void temp_accumulated_temp_per_second(const float &pid_input)
  {
    if (pid_input_get_peroid <= sys_task_get_tick_count())
    {
      if (pid_input_pre <= 0)
      {
        pid_input_diff_count = 0; // 重置温度输入差值累计
        //t_sys.pid_output_factor = 1; // 重置温度输出系数——201765固件不刷新文件时，会出现加热失败；原因是这个系数没有重置
      }
      else
      {
        // 温度输入差值
        float pid_input_diff = pid_input - pid_input_pre;

        if (pid_input_diff < MAX_PID_INPUT_DIFF)
        {
          if (pid_input_time_count > EXTRUDER_ACCUNULATE_INTERVAL)
          {
            pid_input_diff_count += pid_input_diff; // 温度差值累计
          }

          // 出现pid_input_diff小于0的情况，中间没有加热，隔一段时间加热，pid_input_pre记录之前的值，导致出现负数
          // 计算温度系数时，会累加该错误，导致系数偏低
          if (pid_input_diff > 0)
          {
            ++pid_input_time_count; // 秒数累加
          }
        }
        else
        {
          if (pid_input_diff > 0)
          {
            if (pid_input_time_count > EXTRUDER_ACCUNULATE_INTERVAL)
            {
              pid_input_diff_count += pid_input_diff; // 温度差值累计
            }

            ++pid_input_time_count; // 秒数累加
            // pid_input每秒差值大于3，存储温度差值最大值max_pid_input_diff
            has_max_pid_input_diff = true;

            if (max_pid_input_diff < pid_input_diff)
            {
              max_pid_input_diff = pid_input_diff;
            }
          }
        }
      }

      pid_input_pre = pid_input; // 保存当前温度PWM
      pid_input_get_peroid = sys_task_get_tick_count() + 1000; //每1s获取数据
    }
  }

  void temp_reset_pid_output_factor_param(void)
  {
    // 重置温度控制参数信息
    pid_input_get_peroid = 0;
    pid_input_time_count = 0;
    pid_input_pre = 0.0f;
    pid_input_diff_count = 0.0f;
    has_max_pid_input_diff = false;
    max_pid_input_diff = 0.0f;
  }

  bool temp_cal_pid_output_factor(float &factor)
  {
    // 50W加热棒温度差值最大为每秒6度，最少取样值为10秒，即60度的加热量，才进行pid_output系数的计算（减少由于取样少，导致计算偏差大）
    if (pid_input_time_count < (10 + EXTRUDER_ACCUNULATE_INTERVAL))
    {
      temp_reset_pid_output_factor_param(); // 采样数量不符合计算要求，重置参数
      return false;
    }

    if ((pid_input_time_count > 0) && (pid_input_diff_count > 0))
    {
      factor = MAX_PID_INPUT_DIFF / (pid_input_diff_count / ((float)pid_input_time_count - EXTRUDER_ACCUNULATE_INTERVAL));  // float

      // 修正pid_output系数
      if (factor >= PID_OUTPUT_MAX_FACTOR)
      {
        factor = PID_OUTPUT_MAX_FACTOR;
      }
    }

    // 存在has_max_pid_input_diff，更新pid_output系数
    if (has_max_pid_input_diff && (max_pid_input_diff > 0))
    {
      factor = MAX_PID_INPUT_DIFF / max_pid_input_diff; // float
    }

    // 采样计算完成，重置参数
    temp_reset_pid_output_factor_param();
    return true;
  }

  float temp_get_pid_output_max_factor(void)
  {
    return PID_OUTPUT_MAX_FACTOR;
  }

  #ifdef __cplusplus
} // extern "C" {
  #endif
}






