#ifndef LIGHTCONTROL_H
#define LIGHTCONTROL_H
#include "wbtypes.h"
/**
 * 灯控制
 */
class LightControl
{
public:
  LightControl();
  void process(void);                                   ///< 灯控制执行入口
  void led_switch(void);
private:
  void core_board_led(void);                            ///< 核心板LED灯控制
  void led_light(void);                                 ///< LED灯条控制
  void caution_light_twinkle(UINT DelayTime);   ///< M14R03警示灯闪烁
  void caution_light(void);                             ///< M14R03警示灯控制
};

extern LightControl lightControl;

#endif // LIGHTCONTROL_H
