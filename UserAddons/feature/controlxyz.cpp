#include "controlxyz.h"
#include "planner.h"
#include "watch_dog.h"
#include <string.h>
#include "threed_engine.h"
#include "config_motion_3d.h"
#include "user_interface.h"
#include "Alter.h"
#include "sysconfig_data.h"
#include "globalvariables.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif


  // 平台相对当前位置下降60mm，XY归零
  void z_down_60mm_and_xy_to_zero(void)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.zDown60MM();
    controlXYZEB.xyMoveToZero();
  }

  void z_down_to_bottom_and_xy_to_zero(void)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.zDownToBottom();
    controlXYZEB.xyMoveToZero();
  }

  void xy_to_zero(void)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.xyMoveToZero();
  }

  void z_down_to_bottom(void)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.zDownToBottom();
  }

  void z_check_and_set_bottom(CONST bool isUpDownMinMin, CONST FLOAT zBottomValue)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.zCheckTheBottom(isUpDownMinMin, zBottomValue);
  }

  void m84_disable_steppers(void)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.m84DisableXYEB();
  }

  void g92_set_axis_position(CONST INT axis, CONST FLOAT value)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.g92SetAxisPosition(axis, value);
  }

  void eb_compensate_16mm(bool isColorMix)
  {
    ControlXYZEB controlXYZEB;
    controlXYZEB.ebCompensate16mm(isColorMix);
  }

  static void send_cmd_delay(void)
  {
    while(sg_grbl::planner_moves_planned()>0)
    {
      iwdg_refresh(); // ref_data_task任务执行循环延时，需要喂狗，避免触发看门狗
      sys_os_delay(100);  // 延时
    }
    sys_os_delay(50);  // 延时
  }

#ifdef __cplusplus
} //extern "C" {
#endif

ControlXYZEB::ControlXYZEB()
{

}

// 平台相对当前位置下降60mm，发gcode指令
void ControlXYZEB::zDown60MM(void)
{
#if LASER_MODE
  if(t_sys_data_current.IsLaser)
    return;
#endif
  // 开启绝对模式
  sys_send_gcode_cmd("G91 isInternal"); // 开启绝对模式
  sys_send_gcode_cmd("M121 isInternal");// 开启限位检测
  sys_send_gcode_cmd("G1 F600 Z+60 I0 D1 H0 isInternal"); // Z增加60mm
  sys_send_gcode_cmd("M120 isInternal"); // 关闭限位检测
  sys_send_gcode_cmd("G90 isInternal");     // 关闭绝对模式
  send_cmd_delay();
}

// XY快速归零
void ControlXYZEB::xyQuickMoveToZero(void)
{
  // XY快速归零
  if((1 == motion_3d_model.xyz_home_dir[0]) && (1 == motion_3d_model.xyz_home_dir[1]))
  {
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal"); // 设置当前位置
    sys_send_gcode_cmd("G1 F2400 X999 Y999 K1 D1 J0 H0 isInternal");  // X Y轴归零
    send_cmd_delay();
    // X单轴归零
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");   // 设置X当前位置
    sys_send_gcode_cmd("G1 F2400 X999 Y0 H0 D1 J0 isInternal");  // X轴归零
    send_cmd_delay();
    // Y单轴归零
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");  // 设置Y当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y999 H0 D1 J0 isInternal"); // Y轴归零
    send_cmd_delay();
  }
  else if((1 == motion_3d_model.xyz_home_dir[0]) && (-1 == motion_3d_model.xyz_home_dir[1]))
  {
    sys_send_gcode_cmd("G92 X0 Y999 A1 isInternal");  // 设置当前位置
    sys_send_gcode_cmd("G1 F2400 X999 Y0 K1 D1 J0 H0 isInternal");  // X Y轴归零
    send_cmd_delay();
    // X单轴归零
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal"); // 设置X当前位置
    sys_send_gcode_cmd("G1 F2400 X999 Y0 H0 D1 J0 isInternal");   // X轴归零
    send_cmd_delay();
    // Y单轴归零
    sys_send_gcode_cmd("G92 X0 Y999 A1 isInternal");  // 设置Y当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y0 H0 D1 J0 isInternal");  // Y轴归零
    send_cmd_delay();
  }
  else if((-1 == motion_3d_model.xyz_home_dir[0]) && (1 == motion_3d_model.xyz_home_dir[1]))
  {
    sys_send_gcode_cmd("G92 X999 Y0 A1 isInternal"); // 设置当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y999 K1 D1 J0 H0 isInternal");  // X Y轴归零
    send_cmd_delay();
    // X单轴归零
    sys_send_gcode_cmd("G92 X999 Y0 A1 isInternal");   // 设置X当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y0 H0 D1 J0 isInternal");   // X轴归零
    send_cmd_delay();
    // Y单轴归零
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");  // 设置Y当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y999 H0 D1 J0 isInternal");    // Y轴归零
    send_cmd_delay();
  }
  else
  {
    sys_send_gcode_cmd("G92 X999 Y999 A1 isInternal");  // 设置当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y0 K1 D1 J0 H0 isInternal"); // X Y轴归零
    send_cmd_delay();
    // X单轴归零
    sys_send_gcode_cmd("G92 X999 Y0 A1 isInternal");  // 设置X当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y0 H0 D1 J0 isInternal");  // X轴归零
    send_cmd_delay();
    // Y单轴归零
    sys_send_gcode_cmd("G92 X0 Y999 A1 isInternal");   // 设置Y当前位置
    sys_send_gcode_cmd("G1 F2400 X0 Y0 H0 D1 J0 isInternal");  // Y轴归零
    send_cmd_delay();
  }
}

//void test(CONST void *v ) {}

// XY慢速归零
void ControlXYZEB::xySlowMoveToZero(void)
{
  static CHAR gcodeG92XYCommandBuf[50] = {0};
//  test(motion_3d_model.xyz_home_pos);
  if((1 == motion_3d_model.xyz_home_dir[0]) && (1 == motion_3d_model.xyz_home_dir[1]))
  {
    sys_send_gcode_cmd("G92 X999 Y999 A1 isInternal");  // 设置XY当前位置
    sys_send_gcode_cmd("G1 F2400 X994 Y994 H0 D1 J0 isInternal"); // 移动XY到5的位置
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X0 Y999 A1 isInternal"); // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X999 Y999 H0 D1 J0 isInternal");// X慢速归零
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X999 Y0 A1 isInternal");  // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X999 Y999 H0 D1 J0 isInternal");  // Y慢速归零
    memset(gcodeG92XYCommandBuf, 0, sizeof(gcodeG92XYCommandBuf));
    (void)snprintf(gcodeG92XYCommandBuf, sizeof(gcodeG92XYCommandBuf), "G92 X%f Y%f A1 isInternal", motion_3d_model.xyz_home_pos[0], motion_3d_model.xyz_home_pos[1]);
    sys_send_gcode_cmd(gcodeG92XYCommandBuf);// Y轴归零
    send_cmd_delay();
  }
  else if((1 == motion_3d_model.xyz_home_dir[0]) && (-1 == motion_3d_model.xyz_home_dir[1]))
  {
    sys_send_gcode_cmd("G92 X999 Y0 A1 isInternal");  // 设置XY当前位置
    sys_send_gcode_cmd("G1 F2400 X994 Y5 H0 D1 J0 isInternal"); // 移动XY到5的位置
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");   // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X999 Y0 H0 D1 J0 isInternal");   // X慢速归零
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X999 Y999 A1 isInternal");    // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X999 Y0 H0 D1 J0 isInternal");// Y慢速归零
    memset(gcodeG92XYCommandBuf, 0, sizeof(gcodeG92XYCommandBuf));
    (void)snprintf(gcodeG92XYCommandBuf, sizeof(gcodeG92XYCommandBuf), "G92 X%f Y0 A1 isInternal", motion_3d_model.xyz_home_pos[0]);
    sys_send_gcode_cmd(gcodeG92XYCommandBuf);  // Y轴归零
    send_cmd_delay();
  }
  else if((-1 == motion_3d_model.xyz_home_dir[0]) && (1 == motion_3d_model.xyz_home_dir[1]))
  {
    sys_send_gcode_cmd("G92 X0 Y999 A1 isInternal"); // 设置XY当前位置
    sys_send_gcode_cmd("G1 F2400 X5 Y994 H0 D1 J0 isInternal"); // 移动XY到5的位置
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X999 Y999 A1 isInternal"); // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X0 Y999 H0 D1 J0 isInternal"); // X慢速归零
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");  // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X0 Y999 H0 D1 J0 isInternal");  // Y慢速归零
    memset(gcodeG92XYCommandBuf, 0, sizeof(gcodeG92XYCommandBuf));
    (void)snprintf(gcodeG92XYCommandBuf, sizeof(gcodeG92XYCommandBuf), "G92 X0 Y%f A1 isInternal", motion_3d_model.xyz_home_pos[1]);
    sys_send_gcode_cmd(gcodeG92XYCommandBuf); // Y轴归零
    send_cmd_delay();
  }
  else
  {
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");  // 设置XY当前位置
    sys_send_gcode_cmd("G1 F2400 X5 Y5 H0 D1 J0 isInternal"); // 移动XY到5的位置
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X999 Y0 A1 isInternal"); // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X0 Y0 H0 D1 J0 isInternal");   // X慢速归零
    send_cmd_delay();
    sys_send_gcode_cmd("G92 X0 Y999 A1 isInternal");  // 设置XY位置
    sys_send_gcode_cmd("G1 F300 X0 Y0 H0 D1 J0 isInternal");  // Y慢速归零
    sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");  // 设置XY位置
    send_cmd_delay();
  }
}

// 平台相对当前位置下降60mm，发gcode指令
void ControlXYZEB::xyMoveToZero(void)
{
  sys_send_gcode_cmd("M121 isInternal"); // 开启限位检测
  xyQuickMoveToZero();                                                                     // 快速归零
  xySlowMoveToZero();                                                                      // 慢速归零
  sys_send_gcode_cmd("M120 isInternal");  // 关闭限位检测
}

void ControlXYZEB::zDownToBottom(void)
{
  sys_send_gcode_cmd("M121 isInternal"); // 打开限位检测
  sys_send_gcode_cmd("G1 F200 Z999 I0 D1 J0 H0 isInternal"); // 平台下降命令
  sys_send_gcode_cmd("M120 isInternal");// 关闭限位检测
}

void ControlXYZEB::m84DisableXYEB(void)
{
  sys_send_gcode_cmd("M84 X Y E B isInternal");    // 解锁XYEB
}

void ControlXYZEB::zCheckTheBottom(CONST bool isUpDownMinMin, CONST FLOAT zBottomValue)
{
  static CHAR gcodeG92CommandBuf[50] = {0};
  // 上下共限位
  // 上电过程中，平台下降撞击限位，可能会出现限位压得比较死
  // 需要重新确认限位状态，避免续打z高度异常
  if(isUpDownMinMin)
  {
    send_cmd_delay();
    sys_send_gcode_cmd("M120 isInternal"); // 关闭限位检测
    sys_send_gcode_cmd("G91 isInternal");   // 开启绝对模式
    sys_send_gcode_cmd("G1 F120 Z-5 I0 D1 J0 H0 isInternal"); // 平台向上提升5mm                                                                      // 延时
    sys_send_gcode_cmd("M121 isInternal");  // 开启限位检测
    sys_send_gcode_cmd("G1 F120 Z+10 I0 D1 J0 H0 isInternal");// 平台向下降10mm，直到撞击限位                                                                      // 延时
    sys_send_gcode_cmd("G90 isInternal");       // 关闭绝对模式
    sys_send_gcode_cmd("M120 isInternal");   // 关闭限位检测
    send_cmd_delay();
    memset(gcodeG92CommandBuf, 0, sizeof(gcodeG92CommandBuf));
    (void)snprintf(gcodeG92CommandBuf, sizeof(gcodeG92CommandBuf), "G92 X%f Y%f Z%f A1 isInternal", motion_3d_model.xyz_home_pos[0], motion_3d_model.xyz_home_pos[1], zBottomValue);
    sys_send_gcode_cmd(gcodeG92CommandBuf);  // Y轴归零
    sys_send_gcode_cmd("G91 isInternal");   // 开启绝对模式
    sys_send_gcode_cmd("G1 F120 Z-5 I0 D1 J0 H0 isInternal");  // 平台向上提升5mm
    sys_send_gcode_cmd("G90 isInternal");      // 关闭绝对模式
    send_cmd_delay();
  }
}

void ControlXYZEB::g92SetAxisPosition(CONST INT axis, CONST FLOAT value)
{
  static CHAR gcodeCmdBuf[5][40] = {0};
  memset(gcodeCmdBuf[axis], 0, sizeof(gcodeCmdBuf[axis]));
  (void)snprintf(gcodeCmdBuf[axis], sizeof(gcodeCmdBuf[axis]), "G92 %c%f A1 isInternal", gcode::axis_codes[axis], value); // 设置XYZEB坐标
  sys_send_gcode_cmd(gcodeCmdBuf[axis]);//E位置
}

void ControlXYZEB::ebCompensate16mm(bool isColorMix)
{
  sys_send_gcode_cmd("G92 E0 B0 A1 isInternal");          // 设置eb位置
  if(isColorMix)
  {
    sys_send_gcode_cmd("G1 F150 E16 B16 H0 D1 isInternal");    // eb各自运动8mm
  }
  else
  {
    sys_send_gcode_cmd("G1 F150 E16 B0 H0 D1 isInternal");     // eb各自运动8mm
  }
  sys_send_gcode_cmd("G92 E0 B0 A1 isInternal");           // 重新设置eb位置
}

