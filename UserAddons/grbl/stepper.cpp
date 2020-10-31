
#include "stepper.h"
#include "grbl_acc_dec.h"
#include "stepper_pin.h"
#include "ConfigurationStore.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "user_interface.h"
#include "stm32f4xx_hal.h"
#include "globalvariables.h"
#include "Configuration.h"
#include "config_model_tables.h"

namespace sg_grbl
{

  #ifdef __cplusplus
  extern "C" {
  #endif

  // Variables used by The Stepper Driver Interrupt
  static volatile block_t *volatile st_current_block;                                        /*!<  A pointer to the block currently being traced */
  static volatile bool axis_min_endstop[3] = {false};                              /*!< 最小限位状态 */
  static volatile bool save_axis_min_endstop[3] = {false};                         /*!< 保存最小限位状态 */
  static volatile bool save_axis_max_endstop[3] = {false};                         /*!< 保存最大限位状态 */
  static volatile bool check_endstops = false;                            /*!< 限位检测开关 */
  static volatile signed char count_direction[MAX_NUM_AXIS] = {0};        /*!< 计数方向*/
  static volatile long count_position[MAX_NUM_AXIS] = {0};                         /*!< 记录当前位置 */
  static volatile long endstops_trigsteps[3] = {0};                       /*!< 记录限位位置 */
  static volatile long counter[MAX_NUM_AXIS] = {0};                                /*!< Counter variables for the bresenham line tracer */
  static volatile unsigned long step_events_completed = 0;                         /*!< The number of step events executed in the current block */
  static volatile char step_loops = 0;
  static volatile bool check_endstop_z_hit_max = false;                            /*!< 检测z限位撞击最大限位状态 */
  static volatile bool check_endstop_z_hit_min = false;                            /*!< 检测z限位撞击最小限位状态 */
  static volatile bool endstop_z_hit_min = true;
  static volatile bool endstop_z_hit_max = true;

  //===========================================================================
  //=============================functions private ============================
  //===========================================================================

  // Called when the current block is no longer needed. Discards the block and makes the memory
  // availible for new blocks.
  static __inline void plan_discard_current_block(void)
  {
    if (block_buffer_head != block_buffer_tail)
    {
      block_buffer_tail = (block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1);
    }
  }

  // Gets the current block. Returns NULL if buffer empty
  static __inline volatile block_t *plan_get_current_block(void)
  {
    if (block_buffer_head == block_buffer_tail)
    {
      return (0);
    }

    volatile block_t *const block = &block_buffer[block_buffer_tail];
    block->busy = true;
    return (block);
  }

  static __inline void set_steps(void)
  {
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      counter[i] += st_current_block->steps_axis[i];

      if (counter[i] > 0)
      {
        // K5静音驱动，打印过程中，X或Y突然不使能
        if (K5 == t_sys_data_current.model_id && (X_AXIS == i || Y_AXIS == i))
        {
          st_axis_enable(i, true);
        }

        st_axis_write_step(i, true);

        if (t_sys.is_granulator)
        {
          for (int delay = 200; delay > 0; delay--);

          counter[i] -= st_current_block->step_event_count;
          count_position[i] += count_direction[i];
          st_axis_write_step(i, false);
        }
        else
        {
          #ifndef USE_PULSE_DELAY
          counter[i] -= st_current_block->step_event_count;
          count_position[i] += count_direction[i];
          st_axis_write_step(i, false);
          #else

          if ((X_AXIS == i || Y_AXIS == i) && counter[i] > 0 && t_sys.pulse_delay_time > 0)
            st_delay_us(t_sys.pulse_delay_time);

          counter[i] -= st_current_block->step_event_count;
          count_position[i] += count_direction[i];
          st_axis_write_step(i, false);
          #endif
        }
      }
    }
  }

  static __inline void set_dirs(void)
  {
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      if ((st_current_block->direction_bits & (1 << i)) != 0)
      {
        // -direction
        st_axis_write_dir(i, motion_3d_model.enable_invert_dir[i]);
        count_direction[i] = -1;
      }
      else
      {
        // +direction
        st_axis_write_dir(i, !motion_3d_model.enable_invert_dir[i]);
        count_direction[i] = 1;
      }
    }
  }

  // 已經判斷Z零點位置，平臺向上運動，判斷是否已經碰到Z下限位
  // 是且Z碰觸限位，重置碰到Z下限位標志
  static __inline bool is_hit_z_max(void)
  {
    if (t_sys_data_current.enable_powerOff_recovery && motion_3d.enable_poweroff_up_down_min_min)
    {
      // 已经执行过第一次归零，并且z撞击了下限位
      if ((!motion_3d.updown_g28_first_time) && endstop_z_hit_max)
      {
        // 在xyz_max_pos-2到xyz_max_pos的范围内过滤接触限位开关，认为依然处在下限位状态
        if (st_get_position_length(Z_AXIS) > (motion_3d_model.xyz_max_pos[Z_AXIS] - 2))
          return true;
        else
        {
          endstop_z_hit_max = !st_get_z_max_endstops_status();
          return st_get_z_max_endstops_status(); // 返回限位状态
        }
      }

      if (motion_3d.updown_g28_first_time) endstop_z_hit_max = true;
    }

    return false;
  }

  static __inline bool read_z_min_endstop(void)
  {
    //      if(1 == t_sys_data_current.enable_bed_level && !autoBedLevelingAdjust.isCalBedPlatform)
    //      {//舵机校准时，使用PA6作为限位开关检测引脚，因热敏电阻会干扰，这里先注释20170608
    //        z_min_endstop = !HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6);
    //      }
    bool result = false;

    if ((1U == t_sys_data_current.enable_bed_level) && (0U == motion_3d.is_cal_bed_platform)) //调平台时探针的状态可以代替限位
    {
      result = st_axis_xyz_read_min(Z_AXIS) || st_read_other_z_min_endstop(); //读取探针的状态
    }
    else if (motion_3d.is_open_infrared_z_min_check)
    {
      if (!t_sys_data_current.IsMechanismLevel)
        result = (st_read_other_z_min_endstop() && st_read_other_z_min_endstop());
      else
      {
        result = !digitalRead(TEMP_0_PIN);

        if (result)
        {
          st_delay_us(10);//防抖
          result = !digitalRead(TEMP_0_PIN);
        }
      }
    }
    else
    {
      result = st_axis_xyz_read_min(Z_AXIS);
    }

    return result;
  }

  static __inline bool is_hit_z_min(void)
  {
    if (t_sys_data_current.enable_powerOff_recovery && motion_3d.enable_poweroff_up_down_min_min)
    {
      // 已经执行过第一次归零，并且z撞击了上限位
      if ((!motion_3d.updown_g28_first_time) && endstop_z_hit_min)
      {
        // 在0到2的范围内过滤接触限位开关，认为依然处在上限位状态
        if (st_get_position_length(Z_AXIS) < 2)
          return true;
        else
        {
          endstop_z_hit_min = !read_z_min_endstop();
          return read_z_min_endstop(); // 返回限位状态
        }
      }

      if (motion_3d.updown_g28_first_time) endstop_z_hit_min = true;
    }

    return false;
  }

  static __inline void change_z_endstops_status(const volatile bool direction)
  {
    if ((t_sys_data_current.enable_powerOff_recovery) && (motion_3d.enable_poweroff_up_down_min_min) && (!motion_3d.updown_g28_first_time))
    {
      if (!direction)
      {
        endstop_z_hit_min = true;
        endstop_z_hit_max = false;
      }
      else
      {
        endstop_z_hit_min = false;
        endstop_z_hit_max = true;
      }
    }
  }

  static __inline void set_dir_check_endstop_xyz(const volatile int axis)
  {
    #ifndef NEWCOREXY

    if ((st_current_block->direction_bits & (1 << axis)) != 0)
    #else
    if ((X_AXIS == axis && ((st_current_block->direction_bits & (1 << ((int)X_AXIS))) != 0) && ((st_current_block->direction_bits & (1 << Y_AXIS)) == 0)) ||
        (Y_AXIS == axis && ((st_current_block->direction_bits & (1 << Y_AXIS)) != 0)) ||
        (Z_AXIS == axis && ((st_current_block->direction_bits & (1 << axis)) != 0)))
    #endif
    {
      // -direction
      // z处在下限位状态，不检测限位状态
      if ((Z_AXIS == axis) && (is_hit_z_max() || (t_sys_data_current.IsMechanismLevel && !motion_3d.is_open_infrared_z_min_check)))
        return;

      bool min_endstop = false;

      if (Z_AXIS == axis)
      {
        min_endstop = read_z_min_endstop();
      }
      else
      {
        min_endstop = st_axis_xyz_read_min(axis);
      }

      if (motion_3d_model.xyz_home_dir[axis] == 1)
        min_endstop = false;

      if (min_endstop && save_axis_min_endstop[axis] && (st_current_block->steps_axis[axis] > 0))
      {
        endstops_trigsteps[axis] = count_position[axis];
        step_events_completed = (unsigned long)st_current_block->step_event_count;
        axis_min_endstop[axis] = true;

        if (Z_AXIS == axis)
        {
          change_z_endstops_status(false);
          check_endstop_z_hit_min = true;
        }
      }

      save_axis_min_endstop[axis] = min_endstop;
    }

    #ifndef NEWCOREXY
    else
    #else
    else if ((X_AXIS == axis && !(((st_current_block->direction_bits & (1 << ((int)X_AXIS))) != 0) && ((st_current_block->direction_bits & (1 << Y_AXIS)) == 0))) ||
             (Y_AXIS == axis && !((st_current_block->direction_bits & (1 << Y_AXIS)) != 0)) ||
             (Z_AXIS == axis && !((st_current_block->direction_bits & (1 << axis)) != 0)))
    #endif
    {
      // +direction
      // z处在上限位状态（或者限制Z最大）不检测限位状态
      if ((Z_AXIS == axis) && ((motion_3d.disable_z_max_limit) || (is_hit_z_min())))
        return;

      //bool max_endstop = ((Z_AXIS == axis)?st_get_z_max_endstops_status():stepper_axis_xyz_read_max(axis));
      bool max_endstop = false;

      if (Z_AXIS == axis)
      {
        max_endstop = st_get_z_max_endstops_status();
      }
      else
      {
        // 现有机器只有一个限位，dir为1时，home方向为Min-》Max，此时需要设置检测Min限位开关
        if (motion_3d_model.xyz_home_dir[axis] == 1)
        {
          if (Z_AXIS == axis)
          {
            max_endstop = read_z_min_endstop();
          }
          else
          {
            max_endstop = st_axis_xyz_read_min(axis);
          }
        }
        else
          max_endstop = st_axis_xyz_read_max(axis);
      }

      if (max_endstop && save_axis_max_endstop[axis] && (st_current_block->steps_axis[axis] > 0))
      {
        endstops_trigsteps[axis] = count_position[axis];
        step_events_completed = st_current_block->step_event_count;

        if (Z_AXIS == axis)
        {
          change_z_endstops_status(true);
          check_endstop_z_hit_max = true;
        }
      }

      save_axis_max_endstop[axis] = max_endstop;
    }
  }

  static __inline void set_dir_check_endstop(void)
  {
    if (check_endstops)
    {
      set_dir_check_endstop_xyz(X_AXIS);
      set_dir_check_endstop_xyz(Y_AXIS);
      set_dir_check_endstop_xyz(Z_AXIS);
    }
  }

  static __inline void stepper_pin_init(void)
  {
    // 启动电机引脚初始化
    st_motor_start_up_pin_init();
    // 延时200ms后锁定z电机
    (void)sys_os_delay(200);
    // 使能pb3，电机可正常工作
    st_set_motor_start_up_pin_status(true);
    //endstops and pullups
    #ifdef ENDSTOPPULLUP_ZMIN
    st_axis_xyz_write_min();
    #endif
    #ifdef ENDSTOPPULLUP_XMAX
    st_axis_xyz_write_max();
    #endif

    // 使能电机
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      st_axis_enable(i, true);
    }

    //Initialize Step Pins
    st_axis_write_step(X_AXIS, INVERT_X_STEP_PIN);
    st_axis_write_step(Y_AXIS, INVERT_Y_STEP_PIN);
    st_axis_write_step(Z_AXIS, INVERT_Z_STEP_PIN);
    st_axis_write_step(E_AXIS, INVERT_E_STEP_PIN);
    st_axis_write_step(B_AXIS, INVERT_B_STEP_PIN);

    // 解锁电机
    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      st_axis_enable(i, false);
    }
  }

  //===========================================================================
  //=============================functions public  ============================
  //===========================================================================

  bool st_is_xyz_min_endstops_hit(void)
  {
    if ((!axis_min_endstop[X_AXIS]) || (!axis_min_endstop[Y_AXIS]) || (!axis_min_endstop[Z_AXIS]))
      return true;

    return false;
  }

  void st_clear_xyz_min_endstops_hit(void)
  {
    axis_min_endstop[X_AXIS] = false;
    axis_min_endstop[Y_AXIS] = false;
    axis_min_endstop[Z_AXIS] = false;
  }

  void st_process(void)
  {
    // If there is no current block, attempt to pop one from the buffer
    if (st_current_block == 0)
    {
      // Anything in the buffer?
      st_current_block = plan_get_current_block();
      volatile block_t *next_block = &block_buffer[(block_buffer_tail + 1) & (BLOCK_BUFFER_SIZE - 1)];

      if ((st_current_block == 0) && (next_block == 0))
      {
        st_axis_enable(E_AXIS, false);
        st_axis_enable(B_AXIS, false);
      }

      if (st_current_block != 0)
      {
        st_current_block->busy = true;
        trapezoid_or_s_curve_generator_reset(st_current_block, step_loops);

        for (int i = 0; i < motion_3d.axis_num; ++i)
        {
          counter[i] = -(long)(st_current_block->step_event_count >> 1);
        }

        step_events_completed = 0;
      }
      else
      {
        stepper_timer_set_period(2000 - 1); // 1kHz.
      }
    }
    else
    {
      // Set directions TO DO This should be done once during init of trapezoid. Endstops -> interrupt
      //out_bits = st_current_block->direction_bits;
      // Set the direction bits (X_AXIS=A_AXIS and Y_AXIS=B_AXIS for COREXY)
      set_dirs();
      // Set direction en check limit switches
      set_dir_check_endstop();

      for (int8_t i = 0; i < step_loops; ++i)
      {
        // Take multiple steps per interrupt (For high speed moves)
        set_steps();
        step_events_completed += 1;

        if (step_events_completed >= st_current_block->step_event_count) break;
      }

      // Calculare new timer value
      trapezoid_or_s_curve_calculare_timer(st_current_block, step_events_completed, step_loops);

      // If current block is finished, reset pointer
      if (step_events_completed >= st_current_block->step_event_count)
      {
        st_current_block = 0;
        plan_discard_current_block();
      }
    }
  }

  void st_init(void)
  {
    stepper_pin_init();
    stepper_timer_start();
    check_endstops = true; // Start with endstops active. After homing they can be disabled
    // 上电后防止平台往下掉
    st_axis_enable(Z_AXIS, true);
  }

  bool st_check_endstop_z_hit_max(void)
  {
    if (check_endstop_z_hit_max)
    {
      check_endstop_z_hit_max = false;
      return true;
    }
    else
      return false;
  }

  bool st_check_endstop_z_hit_min(void)
  {
    if (check_endstop_z_hit_min)
    {
      check_endstop_z_hit_min = false;
      return true;
    }
    else
      return false;
  }

  void st_enable_endstops(const volatile bool check)
  {
    sys_task_enter_critical();
    check_endstops = check;
    sys_task_exit_critical();
  }

  void st_set_position(const volatile long (&_current_position)[MAX_NUM_AXIS])
  {
    sys_task_enter_critical();

    for (int i = 0; i < motion_3d.axis_num; ++i)
    {
      #ifndef NEWCOREXY
      count_position[i] = _current_position[i];
      #else

      if (X_AXIS == i)
        count_position[X_AXIS] = _current_position[X_AXIS] -  _current_position[Y_AXIS] ;
      else if (Y_AXIS == i)
        count_position[Y_AXIS] = _current_position[Y_AXIS]  ;
      else
        count_position[i] = _current_position[i];

      #endif
    }

    sys_task_exit_critical();
  }

  void st_set_position_axis(const volatile long value, const volatile int axis)
  {
    sys_task_enter_critical();
    count_position[axis] = value;
    sys_task_exit_critical();
  }

  float st_get_endstops_len(const volatile int axis)
  {
    return (float)endstops_trigsteps[axis] / axis_steps_per_unit[axis]; // float
  }

  float st_get_position_length(const volatile int axis)
  {
    return (float)count_position[axis] / axis_steps_per_unit[axis]; // float
  }

  long st_get_position_steps(const volatile int axis)
  {
    return count_position[axis];
  }

  void st_synchronize(void)
  {
    while (block_buffer_head != block_buffer_tail)
    {
      (void)sys_os_delay(50);
    }
  }

  bool st_check_queue_is_empty(void)
  {
    bool result = false;
    result = (((block_buffer_head - block_buffer_tail) + BLOCK_BUFFER_SIZE) & (BLOCK_BUFFER_SIZE - 1)) == 0;
    result = result && (block_buffer_head == block_buffer_tail);
    return result;
  }

  void st_wake_up(void)
  {
    stepper_timer_start();
  }

  void st_quick_stop(void)
  {
    stepper_timer_stop();

    while (block_buffer_head != block_buffer_tail)
      plan_discard_current_block();

    st_current_block = 0;
    stepper_timer_start();
  }

  bool st_get_z_max_endstops_status(void)
  {
    if (t_sys_data_current.enable_powerOff_recovery && motion_3d.enable_poweroff_up_down_min_min)
    {
      // 门检测与断电上下限位冲突，开启门检测只能上下共限位
      if (!motion_3d.enable_check_door_open)
        return (read_z_min_endstop() || st_axis_xyz_read_max(Z_AXIS)); // 新旧插法兼容：1、断电Zmax在Door引脚；2、断电Zmax在Zmax引脚
      else
        return read_z_min_endstop();
    }
    else
    {
      return st_axis_xyz_read_max(Z_AXIS);
    }
  }

  bool st_is_min_endstop(const volatile int axis)
  {
    // 判断是否碰到最小限位
    if (Z_AXIS == axis)
    {
      return read_z_min_endstop();
    }
    else
    {
      return st_axis_xyz_read_min(axis);
    }
  }

  void st_enable_axis(const volatile int axis, const volatile bool isEnable)
  {
    st_axis_enable(axis, isEnable);
  }
  #ifdef __cplusplus
} //extern "C" {
  #endif

}

