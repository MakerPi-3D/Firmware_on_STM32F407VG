#ifndef AUTOBEDLEVELING_H
#define AUTOBEDLEVELING_H
#include "threed_engine.h"
#include "autobedlevelinterface.h"
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif

  extern void gui_bed_level_nozzle_heat(void);
  extern void gui_bed_level_auto_home(void);
#if 0
  extern void gui_bed_level_auto_home_finish(void);
  extern void gui_bed_level_cal_z_at_middle(void);
  extern void gui_bed_level_cal_z_at_right_back(void);
#endif
  extern void gui_bed_level_cal_z(void);
  extern void gui_bed_level_cal_z_at_left_front(void);
  extern void gui_bed_level_cal_z_at_left_back(void);
  extern void gui_bed_level_cal_z_at_right_front(void);
  extern void gui_bed_level_cal_z_finish(void);
  extern void DGUS_bed_level_first(void);

#ifdef __cplusplus
} //extern "C" {
#endif

#ifdef ENABLE_AUTO_BED_LEVELING

class AutoBedLevelingAdjust;

class AutoBedLeveling
{
  friend class AutoBedLevelingAdjust;                                                                                    ///< 声明友元类，友元类可以使用当前类的所有成员

public:
  AutoBedLeveling();
  ~AutoBedLeveling();

  void Gcode_M104(FLOAT value);
  void Gcode_M109(FLOAT value);                                                                                        ///< 自动调平，M109设置喷嘴温度
  void Gcode_G28(void);                    ///< 自动调平，G28指令入口
  void Gcode_G29(void);                                                                                                ///< 自动调平，G29指令入口
  void Gcode_G30(void);                                                                                                ///< 自动调平，G30指令入口                                                                            ///< 设置机型平台触点位置
  void setActiveExtruder(UINT8 value);                                                                               ///< 设置喷嘴
  bool isNozzleHeatFinish;

//  bool OK_KEY;                                                                                                         ///< 界面OK键
  bool isSerial;
private:
  void Gcode_G28_move_to_xy_zero(void);                                                                                ///< G28操作，移动xy到零点

  void do_blocking_move_relative(FLOAT offset_x, FLOAT offset_y, FLOAT offset_z);                                      ///< 当前位置偏移xyz
  void do_blocking_move_to_zero(void);                                                                                 ///< 回到零点
  void do_blocking_move_away_from_zero(void);                                                                          ///< 离开零点
  void get_bed_level_position_z(void);                                                                                 ///< 获取平台触点z高度

  // 复制pos
  FLOAT plan_position[MAX_NUM_AXIS];                                                                                   ///< 复制pos数组
  void copy_curr_position(void);                                                                ///< 复制pos
  void mark_position(CONST FLOAT &pos, CONST INT &index);                                                              ///< 更改pos数组index数值                                                                                              ///< XY移动速度

  // plan_buffer_line设置参数
  FLOAT feedrate;                                                                                                      ///< 进料速率
  INT16 active_extruder;                                                                                             ///< 当前喷头ID

//  FLOAT current_save_xyz[XYZ_NUM_AXIS];                                                                                ///< 保存当前XYZ位置，校准前
//  FLOAT destination_save_xyz[XYZ_NUM_AXIS];                                                                            ///< 保存目标XYZ位置，校准前
};
extern AutoBedLeveling autoBedLeveling;

class AutoBedLevelingAdjust
{
public:
  AutoBedLevelingAdjust();
  ~AutoBedLevelingAdjust();

  void setModelBedPlatform(INT modelID);                                                                               ///< 设置机型平台触点位置

  void startCalculateBedLevel(void);                                                                                   ///< 发送开始校准命令
  void PrepareCalculateBedLevel(void);                                                                                 ///< 为开始计算平台矩阵做准备工作
  void get_bed_level_position_z_at_lf(void);                                                                           ///< 获取平台在左前方碰喷嘴的坐标
  void get_bed_level_position_z_at_lb(void);                                                                           ///< 获取平台在左后方碰喷嘴的坐标
  void get_bed_level_position_z_at_middle(void);                                                                       ///< 获取平台在中间碰喷嘴的坐标
  void get_bed_level_position_z_at_rb(void);                                                                           ///< 获取平台在右后方碰喷嘴的坐标
  void get_bed_level_position_z_at_rf(void);                                                                           ///< 获取平台在右前方碰喷嘴的坐标
//  void set_bed_level_equation(void);                                                                                   ///< 设置平台矩阵
  void calculateBedLevelFinish(void);

  void do_blocking_move_to(FLOAT x, FLOAT y, FLOAT z);                                                                 ///< 移动到xyz
  void run_z_probe(void);                                                                                              ///< 获取平台触点，Z方向操作
  void setup_for_endstop_move(void);                                                                                   ///< 开启限位开关状态
  void clean_up_after_endstop_move(void);                                                                              ///< 关闭限位开关状态
  void correcting_position(void);                                                                                      ///< 修正当前位置

  UINT8 bedLevelStatus;
//  bool isCalBedPlatform ;                                                                                              ///< 是否已经校正平台
private:
  void do_blocking_move_to_xy_getZ(FLOAT x, FLOAT y, FLOAT &z);       	                                               ///< 移动到平台触点操作


//  FLOAT z_at_xLeft_yFront;                                                                                             ///< 探头在左前位置的Z高度
//  FLOAT z_at_xRight_yFront;                                                                                            ///< 探头在右前位置的Z高度
//  FLOAT z_at_xLeft_yBack;                                                                                              ///< 探头在左后位置的Z高度
  FLOAT z_raise_before_probing;                                                                                        ///<
  FLOAT z_raise_between_probings;                                                                                      ///<
  FLOAT z_movedown_until_find_bed ;                                                                                    ///<
  FLOAT xy_travel_speed;                                                                                               ///<
  FLOAT x_probe_offset_from_extruder;                                                                                  ///< 探头X方向偏移位置
  FLOAT y_probe_offset_from_extruder;                                                                                  ///< 探头Y方向偏移位置
  FLOAT z_probe_offset_from_extruder;                                                                                  ///< 探头Z方向偏移位置

  FLOAT targetTemperature;
};
extern AutoBedLevelingAdjust autoBedLevelingAdjust;

#endif //#ifdef ENABLE_AUTO_BED_LEVELING
#endif // AUTOBEDLEVELING_H


