#ifndef CONTROLFUNCTION_H
#define CONTROLFUNCTION_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

  void control_function_init(void);
  void control_function_process(void);
  void buzz(uint16_t value);
  void beep(uint16_t value);

  void TIM2_IRQHandler_process(void); ///< 定时器2，堵料检测执行
  void TIM6_IRQHandler_process(void); ///< 定时器6，温度检测执行
  void TIM4_IRQHandler_process(void); ///< 定时器4，电机执行入口
//void TIM7_IRQHandler_process(void); ///< 定时器7，串口初始化执行

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // CONTROLFUNCTION_H
