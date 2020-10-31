#include "gcode.h"
#include "gcodebufferhandle.h"
#include "stepper.h"
#include "planner.h"
#include "sys_function.h"
#include "gcode_global_params.h"
#include "user_interface.h"
#include "PrintControl.h"
#include "ConfigurationStore.h"
#include "user_debug.h"
#include "interface.h"

namespace gcode
{
  extern void g1_process(void);
  extern void g2_process(void);
  extern void g3_process(void);
  extern void g28_process(void);
  extern void g29_process(void);
  extern void g92_process(void);

  extern void m105_process(void);
  extern void m114_process(void);

  extern void m104_process(void);
  extern void m109_process(void);
  extern void m140_process(void);
  extern void m190_process(void);
  extern void m2004_process(void);

  extern void m106_process(void);
  extern void m107_process(void);

  extern void m200_process(void);
  extern void m201_process(void);
  extern void m203_process(void);
  extern void m204_process(void);
  extern void m205_process(void);

  extern void m220_process(void);
  extern void m221_process(void);

  extern void m301_process(void);
  extern void m302_process(void);
  extern void m303_process(void);

  extern void m600_process(void);
  extern void m601_process(void);
  extern void m602_process(void);

  // M84 解锁电机 （例如：M84 X Y Z E B S1 -->> 代表1S后解锁XYZEB电机）
  extern void m84_process(void);

  extern void m305_process(void);
  extern void m2501_process(void);
  extern void m2502_process(void);
  extern void m2503_process(void);

  extern void m92_process(void);


  static void g_code_process(void)
  {
    switch (static_cast<int32_t>(parseGcodeBufHandle.codeValueLong()))
    {
    case 0: // G0 -> G1
    case 1: // G1
      g1_process();
      break;

    case 2:
      g2_process();
      break;

    case 3:
      g3_process();
      break;

    case 28: //G28 Home all Axis one at a time
      g28_process();
      break;

    case 29://G29 Auto Bed Leveling (Marlin)
      g29_process();
      break;

    case 90: // G90
      relative_mode = false;
      break;

    case 91: // G91
      relative_mode = true;
      break;

    case 92: // G92
      g92_process();
      break;

    default:
      break;
    }
  }

  static void set_extruder_absolute(const bool status)
  {
    sg_grbl::axis_relative_modes[E_AXIS] = status;

    if (1U == t_sys_data_current.enable_color_mixing)
    {
      sg_grbl::axis_relative_modes[B_AXIS] = status;
    }
  }

  static void m_code_process(void)
  {
    switch (static_cast<INT>(parseGcodeBufHandle.codeValueLong()))
    {
    case 104: // M104 Set Extruder Temperature
      m104_process();
      break;

    case 140: // M140 set bed temp
      m140_process();
      break;

    case 105 : // M105 Get Extruder Temperature
      m105_process();
      return;

    case 109:  // M109 - Wait for extruder heater to reach target.
      m109_process();
      break;

    case 190: // M190 - Wait for bed heater to reach target.
      m190_process();
      break;

    case 114://M114  Get Current Position
      m114_process();
      break;

    case 106: //M106 Fan On
      m106_process();
      break;

    case 107: //M107 Fan Off
      m107_process();
      break;

    case 82://M82 Set extruder to absolute mode
      set_extruder_absolute(false);
      break;

    case 83://M83 Set extruder to relative mode
      set_extruder_absolute(true);
      break;

    case 84: // M84 Stop idle hold
      m84_process();
      break;

    case 92: // M92 Set axis_steps_per_unit
      m92_process();
      break;

    case 120: // M120
      sg_grbl::st_synchronize();//等待上一条消息执行完
      sg_grbl::st_enable_endstops(false) ;
      break;

    case 121: // M121
      sg_grbl::st_synchronize();//等待上一条消息执行完
      sg_grbl::st_enable_endstops(true) ;
      break;

    case 200: // M200 D<millimeters> set filament diameter and set E axis units to cubic millimeters (use S0 to set back to millimeters).
      m200_process();
      break;

    case 201: // M201
      m201_process();
      break;

    case 203: // M203 max feedrate mm/sec
      m203_process();
      break;

    case 204: // M204 acclereration S normal moves T filmanent only moves
      m204_process();
      break;

    case 205: //M205 advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk
      m205_process();
      break;

    case 206: // M206 additional homing offset
      for (int8_t i = 0; i < XYZ_NUM_AXIS; ++i)
      {
        if (parseGcodeBufHandle.codeSeen(axis_codes[i]))
        {
          add_homing[i] = parseGcodeBufHandle.codeValue();
        }
      }

      break;

    case 207: // M207 Calibrate z axis by detecting z max length
      //串口上传信息到上位机2017.7.6
      USER_EchoLogStr("Start cal_z_max_pos\r\n");
      respond_gui_send_sem(CalculateZMaxPos);
      break;

    case 220: // M220 S<factor in percent>- set speed factor override percentage
      m220_process();
      break;

    case 221: // M221 S<factor in percent>- set extrude factor override percentage
      m221_process();
      break;

    case 301: // M301
      m301_process();
      break;

    case 302: // allow cold extrudes, or set the minimum extrude temperature
      m302_process();
      break;

    case 303: // M303 PID autotune
      m303_process();
      break;

    case 400: // M400 finish all moves
      sg_grbl::st_synchronize();
      break;

    case 600: //M600//Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
      m600_process();
      break;

    case 601: //M601//Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
      m601_process();
      break;

    case 602://M602
      m602_process();
      break;

    case 117://M117 start Printing

      //串口上传信息到上位机2017.7.6
      //    USER_EchoLogStr("M117 Printing!\r\n");
      //    print_flage = 1;
      if (parseGcodeBufHandle.codeSeen('F'))
      {
        t_gui.cura_speed = parseGcodeBufHandle.codeValue();
      }
      else
      {
        t_gui.cura_speed = 40U;
      }

      break;

    case 305://M305
      m305_process();
      break;

    case 2004:// M2004 设置温度不等待运动完成
      m2004_process();
      break;

    case 2501:
      m2501_process();
      break;

    case 2502:
      m2502_process();
      break;

    case 2503:
      m2503_process();
      break;

    default:
      break;
    }
  }

  void t_code_process(void)
  {
    tmp_extruder = (uint8_t)parseGcodeBufHandle.codeValueLong();

    if (tmp_extruder >= EXTRUDERS)
    {
#if 0
      SERIAL_ECHO_START;
      SERIAL_ECHO("T");
      SERIAL_ECHO(tmp_extruder);
      SERIAL_ECHOLN(MSG_INVALID_EXTRUDER);
#endif
    }
    else
    {
#if EXTRUDERS > 1
      bool make_move = false;
#endif

      if (parseGcodeBufHandle.codeSeen('F'))
      {
#if EXTRUDERS > 1
        make_move = true;
#endif
        float next_feedrate = parseGcodeBufHandle.codeValue();

        if (next_feedrate > 0.0F)
        {
          feed_rate = next_feedrate;
        }
      }

#if EXTRUDERS > 1

      if (tmp_extruder != active_extruder)
      {
        // Save current position to return to after applying extruder offset
        memcpy(destination, current_position, sizeof(destination));
        // Offset extruder (only by XY)
        int16_t i;

        for (i = 0; i < 2; ++i)
        {
          current_position[i] = current_position[i] -
                                extruder_offset[i][active_extruder] +
                                extruder_offset[i][tmp_extruder];
        }

        // Set the new active extruder and position
        active_extruder = tmp_extruder;
        sg_grbl::planner_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

        // Move to the old position if 'F' was in the parameters
        if (make_move)
        {
          prepare_move();
        }
      }

#endif
    }
  }


}


#ifdef __cplusplus
extern "C" {
#endif




extern  void serial_ClearToSend();

void stepper_quick_stop(void)
{
  // 清空运动队列
  sg_grbl::st_quick_stop();
  // 清空指令队列
  sys_clear_gcode_cmd();

  // 二次确认运动队列
  if (sg_grbl::planner_moves_planned() > 1)
  {
    sg_grbl::st_quick_stop();
  }

  // 延时让其他任务执行
  sys_os_delay(100);
  gcode::plan_st_synchronize();
}


bool t_set_targeted_hotend(int16_t code)
{
  bool result = false;
  gcode::tmp_extruder = gcode::active_extruder;

  if (gcode::parseGcodeBufHandle.codeSeen('T'))
  {
    gcode::tmp_extruder = static_cast<int16_t>(gcode::parseGcodeBufHandle.codeValueLong());

    if (gcode::tmp_extruder >= EXTRUDERS)
    {
      USER_ErrLogStart();
      USER_ErrLogStr("M%d Invalid extruder ", code); // M104 M105 M109 M218 M221
      USER_ErrLogStr("%d\n", gcode::tmp_extruder);
      result = true;
    }
  }

  return result;
}


void test(CONST void *v) {}

void process_commands(void)
{
  gcode::is_serial_cmd = 0;

  if (gcode::parseGcodeBufHandle.codeSeen('G'))
  {
    gcode::g_code_process();
    serial_ClearToSend();
  }
  else if (gcode::parseGcodeBufHandle.codeSeen('M'))
  {
    gcode::m_code_process();
    serial_ClearToSend();
  }
  else if (gcode::parseGcodeBufHandle.codeSeen('T'))
  {
    gcode::t_code_process();
    serial_ClearToSend();
  }

  if ((0 != IsPrint())  && (0 == IsPausePrint()))   //若有打印发送信号量从而从SD卡或U盘读取下一条gcode命令
  {
    task_read_udisk_release();
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif





