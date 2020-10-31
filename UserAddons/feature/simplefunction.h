#ifndef SIMPLEFUNCTION_H
#define SIMPLEFUNCTION_H

#include "globalvariables.h"

class SimpleFunction
{
public:
  SimpleFunction();
  void process(void);
private:
  void control_check_door_open(void);
  void control_motor_fan(void);
};

/**
 * 声音控制
 */
class SoundControl
{
public:
  SoundControl();
  void beep(uint16_t time);                 ///< 提示音
  void buzz(uint16_t msticks);              ///< 蜂鸣器
  void beepAlarm(void);                           ///< 报警声
};

extern SimpleFunction simpleFunction;
extern SoundControl soundControl;

#endif // SIMPLEFUNCTION_H

