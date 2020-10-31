#ifndef CONTROLXYZEB_H
#define CONTROLXYZEB_H
#include "wbtypes.h"
#ifdef __cplusplus
extern "C" {
#endif

  void z_down_60mm_and_xy_to_zero(void);                                                ///< z下降60mm，xy归零
  void xy_to_zero(void);                                                                ///< xy归零
  void z_down_to_bottom(void);                                                          ///< z下降到底部
  void z_down_to_bottom_and_xy_to_zero(void);                                           ///< z下降到底部，xy归零
  void z_check_and_set_bottom(CONST bool isUpDownMinMin, CONST FLOAT zBottomValue);     ///< z检测底部并设置
  void eb_compensate_16mm(bool isColorMix);                                                         ///< eb补偿8mm
  void m84_disable_steppers(void);                                                      ///< m84解锁xyeb
  void g92_set_axis_position(CONST INT axis, CONST FLOAT value);                        ///< g92设置xyzeb位置

#ifdef __cplusplus
} //extern "C" {
#endif

class ControlXYZEB
{
public:
  ControlXYZEB();
  void xyMoveToZero(void);                                           ///< xy归零
  void zDown60MM(void);                                              ///< z下降60mm
  void zDownToBottom(void);                                          ///< z下降到底部
  void zCheckTheBottom(CONST bool isUpDownMinMin, CONST FLOAT zBottomValue);                   ///< z检测底部位置
  void ebCompensate16mm(bool isColorMix);                                        ///< eb补偿8mm，用于断电与暂停打印
  void m84DisableXYEB(void);                                         ///< m84解锁电机
  void g92SetAxisPosition(CONST INT axis, CONST FLOAT value);        ///< g92设置当前位置
private:
  void xyQuickMoveToZero(void);                                      ///< xy快速归零
  void xySlowMoveToZero(void);                                       ///< xy慢速归零
};

#endif // CONTROLXYZEB_H
