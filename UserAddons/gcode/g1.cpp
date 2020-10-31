#include "gcodebufferhandle.h"
#include "globalvariables_ccmram.h"
#include "planner.h"
#include "stepper.h"
#include "Configuration.h"
#include "sys_function.h"
#include "sg_util.h"
#include "config_model_tables.h"
#include "RespondGUI.h"
#include "PrintControl.h"
#include "ConfigurationStore.h"
#include "gcode_global_params.h"
#include "Alter.h"

namespace gcode
{
  extern void process_command_synchronize(void);

  static bool isStrokeDetection = true;     /*!< 参数J，是否开启行程检测 */
  static bool isCoordTransform = false;     /*!< 参数H，是否进行坐标变换 */
  static bool isInfraredDetection = false;  /*!< 参数I，是否开启红外检测 */
  static bool isNewCorexyXFixed = false;    /*!< 参数K，NEWCOREXY机器归零X先限位异常问题处理 */

  //是否开启行程检测
  static void get_is_stroke_detection(void)
  {
    isStrokeDetection = true;
    if(parseGcodeBufHandle.codeSeen('J'))
    {
      int value = parseGcodeBufHandle.codeValue();
      if(value == 1)
      {
        isStrokeDetection = true;
      }
      else if(value == 0)
      {
        isStrokeDetection = false;
      }
    }
  }

  //是否进行自动调平坐标变换
  static void get_is_coord_transform(void)
  {
    isCoordTransform = false;
    if(parseGcodeBufHandle.codeSeen('H'))
    {
      int value = parseGcodeBufHandle.codeValue();
      if(value == 1)
      {
        isCoordTransform = true;
      }
      else if(value == 0)
      {
        isCoordTransform = false;
      }
    }
  }

  //是否开启红外检测
  static void get_is_infrared_detection(void)
  {
    isInfraredDetection = true;
    if(parseGcodeBufHandle.codeSeen('I'))
    {
      int value = parseGcodeBufHandle.codeValue();
      if(value == 1)
      {
        isInfraredDetection = true;
      }
      else if(value == 0)
      {
        isInfraredDetection = false;
      }
    }
  }

  //是否NEWCOREXY机器归零X先限位异常问题处理
  static void get_is_newCorexy_xFixed(void)
  {
    isNewCorexyXFixed = false;
    if(parseGcodeBufHandle.codeSeen('K'))
    {
      int value = parseGcodeBufHandle.codeValue();
      if(value == 1)
      {
        isNewCorexyXFixed = true;
      }
      else if(value == 0)
      {
        isNewCorexyXFixed = false;
      }
    }
  }

  //设置文件打印位置
  static void get_file_pos(void)
  {
    if(parseGcodeBufHandle.codeSeen('P'))
    {
      print_file_pos=(uint32_t)parseGcodeBufHandle.codeValueLong();
    }
  }

  //获取进料速度
  static void get_feed_rate(void)
  {
    if(parseGcodeBufHandle.codeSeen('F'))
    {
      float next_feedrate = parseGcodeBufHandle.codeValue();
      if(next_feedrate > 0.0F)
      {
        feed_rate = next_feedrate;
      }
    }
    //当移动Z轴大于当前位置5mm以上，则限制其移动速度为 350； 2017512增加速度350；可以在300~350之间取值，大了会导致下降噪声大 add by john
    if(destination[Z_AXIS] > (current_position[Z_AXIS] + 5.0F))
    {
      if(motion_3d_model.xyz_max_pos[2] > 350.0F)//z轴大于350说明是大机器，小机器不用控制速度201716
      {
        feed_rate = 350.0F;
      }
    }
  }


  void get_coordinates(void)
  {
    for(uint8_t i=0U; i < motion_3d.axis_num; ++i)
    {
      bool axis_code_seen = parseGcodeBufHandle.codeSeen(axis_codes[i]);
      float axis_code_value = 0.0F;
      if(axis_code_seen)
      {
        axis_code_value = parseGcodeBufHandle.codeValue();
      }
      bool is_relative_mode = (bool)((sg_grbl::axis_relative_modes[i]) || relative_mode);
#if LASER_MODE
      if(t_sys_data_current.IsLaser && (Z_AXIS == i) && IsPrint())
        axis_code_seen = false;
#endif
      {
        if(axis_code_seen)
        {
          if(i < XYZ_NUM_AXIS)
          {
            destination[i] = axis_code_value + ((float)is_relative_mode*sg_grbl::plan_get_current_save_xyz(i)); // float
          }
          else
          {
            destination[i] = axis_code_value + ((float)is_relative_mode*current_position[i]); // float
          }
        }
        else
        {
          if(i < XYZ_NUM_AXIS && !t_sys_data_current.IsLaser)
          {
            destination[i] = sg_grbl::plan_get_current_save_xyz(i); //Are these else lines really needed?
          }
          else
          {
            destination[i] = current_position[i]; //Are these else lines really needed?
          }

        }
      }
      // 不开启断电续打，且当前模式为相对模式，目标点大于最大点，重置目标点位置
      if((Z_AXIS == i) && (0U != motion_3d.disable_z_max_limit) && (0U != is_relative_mode) && (destination[i] > motion_3d_model.xyz_max_pos[i]))
      {
        destination[i] = motion_3d_model.xyz_max_pos[i];
      }
    }

    get_feed_rate(); //获取进料速度
  }


  static void z_max_limit(volatile float (&target)[MAX_NUM_AXIS], volatile uint8_t &IsPopWarningInterface)
  {
    if(0U != motion_3d.disable_z_max_limit) //没有下限位开关
    {
      if (target[Z_AXIS] > (motion_3d_model.xyz_max_pos[Z_AXIS] + add_homing[Z_AXIS]))// float
      {
        // 个别现象，+8是防止校准值小于默认值太多，引起最大行程警报.
        if(target[Z_AXIS] > (motion_3d_model.xyz_max_pos[Z_AXIS] + add_homing[Z_AXIS] + 8.0F))
        {
          IsPopWarningInterface=6U;
        }
        target[Z_AXIS] = motion_3d_model.xyz_max_pos[Z_AXIS] + add_homing[Z_AXIS];// float
      }
    }
    else if(0 != IsPrint()) //有下限位开关，在打印的时候才报错
    {
      if (target[Z_AXIS] > (motion_3d_model.xyz_max_pos[Z_AXIS] + add_homing[Z_AXIS])) // float
      {
        if(target[Z_AXIS] > (motion_3d_model.xyz_max_pos[Z_AXIS] + add_homing[Z_AXIS] + 8.0F)) // float
        {
          IsPopWarningInterface=6U;
        }
        target[Z_AXIS] = motion_3d_model.xyz_max_pos[Z_AXIS] + add_homing[Z_AXIS]; // float
      }
    }
  }

  static void xyz_limit_pop_warming(const volatile uint8_t IsPopWarningInterface)
  {
    switch(IsPopWarningInterface)
    {
    case 1U:
      PopWarningInfo(XMinLimitWarning);
      break;
    case 2U:
      PopWarningInfo(YMinLimitWarning);
      break;
    case 3U:
      PopWarningInfo(ZMinLimitWarning);
      break;
    case 4U:
      PopWarningInfo(XMaxLimitWarning);
      break;
    case 5U:
      PopWarningInfo(YMaxLimitWarning);
      break;
    case 6U:
      PopWarningInfo(ZMaxLimitWarning);
      break;
    default:
      break;
    }
  }

  void clamp_to_software_endstops(volatile float (&target)[MAX_NUM_AXIS])
  {
    uint8_t IsPopWarningInterface=0U;

    //MinLimit(XYZ)
    for(int i = 0; i < XYZ_NUM_AXIS; i++)
    {
      if (target[i] < (motion_3d_model.xyz_min_pos[i] + add_homing[i]))// float
      {
        IsPopWarningInterface = i+1U;
      }
    }

    //MaxLimit(XY)
    for(int i = 0; i < XYZ_NUM_AXIS - 1; i++)
    {
      if (target[i] > (motion_3d_model.xyz_max_pos[i] + add_homing[i]))// float
      {
        IsPopWarningInterface = i+4U;
      }
    }

    z_max_limit(target, IsPopWarningInterface);

    if(0U != IsPopWarningInterface)
    {
      target[X_AXIS] = current_position[X_AXIS];
      target[Y_AXIS] = current_position[Y_AXIS];
      target[Z_AXIS] = current_position[Z_AXIS];
    }
    xyz_limit_pop_warming(IsPopWarningInterface);
  }

  static void g1_prepare_move(void)
  {
    if(1U != t_sys_data_current.enable_bed_level)
    {
      /* 行程检测 */
      if(isStrokeDetection)
      {
        clamp_to_software_endstops(destination);
      }
    }
    previous_xTaskGetTickCount_cmd = sys_task_get_tick_count();

    //M14R03打印软料时第一层时速度要慢，ADD 20170502
    if(current_position[Z_AXIS] <= 0.3F)
    {
      if((0U != t_sys_data_current.enable_soft_filament) && (M14R03== t_sys_data_current.model_id))
      {
        feed_rate = (1200.0F*5.0F)/10.0F;		//棱型图形速度要低于1200*0.6
      }
    }

    /* Do not use feedmultiply for E or Z only moves*/
    if(sg_util::is_float_data_equivalent(current_position[X_AXIS], destination [X_AXIS]) && sg_util::is_float_data_equivalent(current_position[Y_AXIS], destination [Y_AXIS]))
    {
      process_buffer_line(destination, feed_rate/60.0F, feed_multiply); // float
    }
    else
    {
      /* 现在XY速度不小与20mm/s */
      FLOAT feed_rate_tmp = ((feed_rate*static_cast<FLOAT>(feed_multiply))/60.0F)/100.0F;
#ifdef NEWCOREXY
      if(stepper_axis_xyz_read_min(X_AXIS) && destination[Y_AXIS] > destination[X_AXIS])//X轴压到限位
      {
        current_position[X_AXIS] += 1;
        process_buffer_line(current_position, feed_rate_tmp, feed_multiply);
        process_buffer_line(destination, feed_rate_tmp, feed_multiply);
      }
      else
#endif
      {
        process_buffer_line(destination, feed_rate_tmp, feed_multiply);
        if(isNewCorexyXFixed)
        {
          process_command_synchronize();
        }
      }
    }

    for(uint8_t i=0U; i < motion_3d.axis_num; ++i)
    {
      current_position[i] = destination[i];
    }
  }

  void g1_process(void)
  {
    get_is_stroke_detection(); //是否开启行程检测
    get_is_coord_transform(); //是否进行自动调平坐标变换
    get_is_infrared_detection(); //是否开启红外检测
    get_is_newCorexy_xFixed(); //是否NEWCOREXY机器归零X先限位异常问题处理
#ifdef CAL_Z_ZERO_OFFSET
    // 参数C，关闭红外检测
    if(!isInfraredDetection)
    {
      if(2U == t_sys_data_current.enable_bed_level && 1U != t_sys.is_bed_level_down_to_zero)
      {
        motion_3d.is_open_infrared_z_min_check = false;
      }
    }
#endif

    //poweroffrecoverry//2017516恢复到打印高度后再把断电续打状态标志置0，对应poweroffrecovery.cpp288行对其置1
    if(current_position[E_AXIS] == t_power_off.e_pos)
    {
      t_power_off.is_power_off=0U;
    }

    get_coordinates(); // For X Y Z E F
    get_file_pos(); //设置文件打印位置
    if(isCoordTransform)
    {
      sg_grbl::plan_set_process_auto_bed_level_status(false);
      g1_prepare_move();
      sg_grbl::plan_set_process_auto_bed_level_status(true);
    }
    else
    {
      g1_prepare_move();
    }

#ifdef CAL_Z_ZERO_OFFSET
    if(!isInfraredDetection)
    {
      sg_grbl::st_synchronize();
      if(2U == t_sys_data_current.enable_bed_level && 1U != t_sys.is_bed_level_down_to_zero)
      {
        motion_3d.is_open_infrared_z_min_check = true;
      }
    }
    else
    {
      motion_3d.is_open_infrared_z_min_check = false;
    }
#endif
  }


}








