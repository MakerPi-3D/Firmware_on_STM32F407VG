#include "gcode_global_params.h"
#include "gcodebufferhandle.h"
#include "user_interface.h"
#include "planner.h"
#include "temperature.h"
#include "stepper.h"
#include "user_debug.h"


namespace gcode
{
  volatile int fan_speed = 0;                                                       /*!< 风扇速度 */
  volatile int16_t active_extruder = 0;                                             /*!< 运动喷头id */
  volatile int16_t tmp_extruder = 0;                                                /*!< 加热喷头id */
  volatile float feed_rate = 1500.0f;                                               /*!< 进料速度 */
  volatile int feed_multiply=100; //100->1 200->2                                   /*!< 进料速度百分比 */
  volatile int extruder_multiply=100; //100->1 200->2                               /*!< 移动速度百分比 */
  volatile float add_homing[XYZ_NUM_AXIS] = {0.0f, 0.0f, 0.0f};                     /*!< xyz零点偏移 */
  volatile float current_position[MAX_NUM_AXIS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};  /*!< 当前坐标 */
  volatile float destination[MAX_NUM_AXIS] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};       /*!< 目标坐标 */
  volatile uint8_t is_serial_cmd = 0;                                               /*!< 是否为串口指令 */
  volatile bool relative_mode = false;                                              /*!< 相对模式开关 Determines Absolute or Relative Coordinates */
  volatile uint32_t print_file_pos = 0U;                                            /*!< 打印文件位置 */
  volatile uint32_t stepper_inactive_time = DEFAULT_STEPPER_DEACTIVE_TIME*1000L;    /*!< 电机解锁延时 */
  volatile uint32_t printing_material_length = 0U;                                  /*!< 打印耗材长度 */
  volatile uint32_t previous_xTaskGetTickCount_cmd = 0;                             /*!< 指令延时时间 */
  volatile bool is_confirm_load_filament = false;                                   /*!< 中途换料确定键 */
  volatile uint8_t m600_filament_change_status = 0;                                 /*!< 中途换料状态 */
  volatile bool m600_is_midway_change_material = false;                             /*!< 是否处于中途换料状态 */


  static void kill(void)
  {
    sys_task_enter_critical();
    sg_grbl::temperature_disable_heater();

    for(int32_t i = 0; i < motion_3d.axis_num; ++i)
    {
      sg_grbl::st_enable_axis(i, false);
    }
    sys_task_exit_critical();
    USER_ErrLog("Printer halted. kill() called!");
    for(;;)
    {
      /* Intentionally left empty */
    } // Wait for reset
  }

  void manage_inactivity(void)
  {
    static uint32_t max_inactive_time = 0;
    if( (sys_task_get_tick_count() - previous_xTaskGetTickCount_cmd) >  max_inactive_time )
    {
      if(0UL != max_inactive_time)
      {
        kill();
      }
    }
    if(0UL != stepper_inactive_time)
    {
      if( (sys_task_get_tick_count() - previous_xTaskGetTickCount_cmd) >  stepper_inactive_time )
      {
        if(!sg_grbl::planner_blocks_queued())
        {
          sg_grbl::st_enable_axis(X_AXIS, false);
          sg_grbl::st_enable_axis(Y_AXIS, false);
#if 0
          if(t_sys_data_current.pic_id != 2)//非日本机器才松开z轴
            disable_z();//Z? -yanghai 20140605
#endif
          sg_grbl::st_enable_axis(E_AXIS, false);
          sg_grbl::st_enable_axis(B_AXIS, false);
        }
      }
    }
  }

  void set_fan_speed(const int value)
  {
    if(fan_speed != value)
    {
      fan_speed = value;
      sys_time5_write_fan_pwm(static_cast<uint16_t>(fan_speed));
    }
  }

  float get_current_position(const volatile uint8_t axis)
  {
    float result = 0.0f;
    if(axis < motion_3d.axis_num)
    {
      result = current_position[axis];
    }
    return result;
  }


  void set_current_position(const volatile uint8_t axis, const volatile float value)
  {
    if(axis < motion_3d.axis_num)
    {
      current_position[axis] = value;
    }
  }

  void plan_set_current_position(void)
  {
    sg_grbl::planner_set_position(current_position);
  }


  float get_destination_position(const volatile uint8_t axis)
  {
    float result = 0.0f;
    if(axis < motion_3d.axis_num)
    {
      result = destination[axis];
    }
    return result;
  }

  void set_destination_position(const volatile uint8_t axis,const volatile float value)
  {
    if(axis < motion_3d.axis_num)
    {
      destination[axis] = value;
    }
  }

  void plan_set_destination_position(void)
  {
    sg_grbl::planner_set_position(destination);
  }

  void process_buffer_line(const volatile float (&position)[MAX_NUM_AXIS], const volatile float _feed_rate, const volatile int _feed_multiply)
  {
    running_status.bed_temp=sg_grbl::temperature_get_bed_target();
    running_status.extruder0_temp=sg_grbl::temperature_get_extruder_target(0);
    running_status.fan_speed = fan_speed;
    running_status.extruder = active_extruder;
    running_status.feed_rate = _feed_rate;
    running_status.feed_multiply = _feed_multiply;
    running_status.extruder_multiply = extruder_multiply;
    running_status.sd_position = print_file_pos;
    running_status.is_serial = is_serial_cmd;//一直是0
    for(int i = 0; i < MAX_NUM_AXIS; i++)
    {
      running_status.axis_position[i] = position[i];
    }
    sg_grbl::planner_buffer_line(&running_status);
  }

  void process_buffer_line_normal(const volatile float (&position)[MAX_NUM_AXIS], const volatile float _feed_rate)
  {
    process_buffer_line(position, _feed_rate, 100);
    sg_grbl::st_synchronize();// wait for finish step buffer
  }

  void process_buffer_line_normal_4_curr(const volatile float _feed_rate)
  {
    process_buffer_line_normal(current_position, _feed_rate);
  }

  void process_buffer_line_normal_4_dest(const volatile float _feed_rate)
  {
    process_buffer_line_normal(destination, _feed_rate);
  }

  void plan_st_synchronize(void)
  {
    for(int32_t i = 0; i < motion_3d.axis_num; ++i)
    {
      if(i < XYZ_NUM_AXIS)
      {
        // 获取st pos值会异常，限定值范围，避免异常
        if(motion_3d_model.xyz_move_max_pos[i] < sg_grbl::st_get_position_length(i))
        {
          current_position[i] = motion_3d_model.xyz_move_max_pos[i];
          destination[i] = motion_3d_model.xyz_move_max_pos[i];
        }
        else if(0.0F > sg_grbl::st_get_position_length(i))
        {
          current_position[i] = 0.0F;
          destination[i] = 0.0F;
        }
        else
        {
          current_position[i] = sg_grbl::st_get_position_length(i);
          destination[i] = sg_grbl::st_get_position_length(i);
        }
      }
      else
      {
        current_position[i] = sg_grbl::st_get_position_length(i);
        destination[i] = sg_grbl::st_get_position_length(i);
      }
    }
    sg_grbl::planner_set_position(current_position);
  }

}






