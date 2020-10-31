#ifndef STEPPER_PIN_H
#define STEPPER_PIN_H

#include "stm32f4xx_hal.h"
#include "pins.h"
#include "threed_engine.h"

// coarse Endstop Settings
#define ENDSTOPPULLUPS // Comment this out (using // at the start of the line) to disable the endstop pullup resistors
#ifdef ENDSTOPPULLUPS
#define ENDSTOPPULLUP_XMAX
#define ENDSTOPPULLUP_YMAX
#define ENDSTOPPULLUP_ZMAX
#define ENDSTOPPULLUP_XMIN
#define ENDSTOPPULLUP_YMIN
#define ENDSTOPPULLUP_ZMIN
#endif

// The pullups are needed if you directly connect a mechanical endswitch between the signal and ground pins.
#define X_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Y_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Z_MIN_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define X_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Y_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.
#define Z_MAX_ENDSTOP_INVERTING 1 // set to true to invert the logic of the endstop.

// For Inverting Stepper Enable Pins (Active Low) use 0, Non Inverting (Active High) use 1
#define X_ENABLE_ON 0
#define Y_ENABLE_ON 0
#define Z_ENABLE_ON 0
#define E_ENABLE_ON 0
#define B_ENABLE_ON 0 // For all extruders

#define X_ENABLE_OFF 1
#define Y_ENABLE_OFF 1
#define Z_ENABLE_OFF 1
#define E_ENABLE_OFF 1
#define B_ENABLE_OFF 1 // For all extruders

// Disables axis when it's not being used.
#define DISABLE_X false
#define DISABLE_Y false
#define DISABLE_Z false
#define DISABLE_E false
#define DISABLE_B false// For all extruders
#define DISABLE_INACTIVE_EXTRUDER true //disable only inactive extruders and keep active extruder enabled

//By default pololu step drivers require an active high signal. However, some high power drivers require an active low signal as step.
#define INVERT_X_STEP_PIN false
#define INVERT_Y_STEP_PIN false
#define INVERT_Z_STEP_PIN false
#define INVERT_E_STEP_PIN false
#define INVERT_B_STEP_PIN false//COLOR_MIXING

#ifdef __cplusplus
extern "C" {
#endif

  extern TIM_HandleTypeDef htim4;

  __inline void st_delay_us(volatile unsigned short time)
  {
    volatile unsigned short i=0;
    while(time--)
    {
      i=10;  //自己定义
      while(i--);// __NOP();
    }
  }

  // 李工新电路板，增加PB3引脚控制电机工作
  // PB3使能，电机可正常工作
  __inline void st_motor_start_up_pin_init(void)
  {
    // PB3引脚，启动电机
    GPIO_InitTypeDef GPIO_InitStruct;
    __GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = STEP_START_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(STEP_START_PIN_GPIO, &GPIO_InitStruct);
  }

  // 使能pb3，电机可正常工作;
  __inline void st_set_motor_start_up_pin_status(const volatile bool on)
  {
    digitalWrite(STEP_START_PIN, (on?GPIO_PIN_SET:GPIO_PIN_RESET));
  }

  // 使能轴
  __inline void st_axis_enable(const volatile int axis, const volatile bool isEnable)
  {
    if(X_AXIS == axis)
    {
#if defined(X_ENABLE_PIN) && (X_ENABLE_PIN_ON > -1)
      if(!DISABLE_X)
      {
        digitalWrite(X_ENABLE_PIN, (isEnable?X_ENABLE_ON:X_ENABLE_OFF));
      }
#endif // #if defined(X_ENABLE_PIN) && (X_ENABLE_PIN_ON > -1)
    }
    else if(Y_AXIS == axis)
    {
#if defined(Y_ENABLE_PIN) && (Y_ENABLE_PIN_ON > -1)
      if(!DISABLE_Y)
      {
        digitalWrite(Y_ENABLE_PIN, (isEnable?Y_ENABLE_ON:Y_ENABLE_OFF));
      }
#endif // #if defined(Y_ENABLE_PIN) && (Y_ENABLE_PIN_ON > -1)
    }
    else if(Z_AXIS == axis)
    {
#if defined(Z_ENABLE_PIN) && (Z_ENABLE_PIN_ON > -1)
      if(!DISABLE_Z)
      {
        digitalWrite(Z_ENABLE_PIN, (isEnable?Z_ENABLE_ON:Z_ENABLE_OFF));
      }
#endif // #if defined(Z_ENABLE_PIN) && (Z_ENABLE_PIN_ON > -1)
    }
    else if(E_AXIS == axis)
    {
#if defined(E0_ENABLE_PIN) && (E0_ENABLE_PIN_ON > -1)
      if(!DISABLE_E)
      {
        digitalWrite(E0_ENABLE_PIN, (isEnable?E_ENABLE_ON:E_ENABLE_OFF));
      }
#endif // #if defined(E0_ENABLE_PIN) && (E0_ENABLE_PIN_ON > -1)
    }
    else if(B_AXIS == axis)
    {
#if defined(E1_ENABLE_PIN) && (E1_ENABLE_PIN_ON > -1)
      if(!DISABLE_B)
      {
        digitalWrite(E1_ENABLE_PIN, (isEnable?B_ENABLE_ON:B_ENABLE_OFF));
      }
#endif // #if defined(E1_ENABLE_PIN) && (E1_ENABLE_PIN_ON > -1)
    }
  }

  // 写脉冲
  __inline void st_axis_write_step(const volatile int axis, const volatile bool value)
  {
    if(X_AXIS == axis)
    {
#if defined(X_STEP_PIN) && (X_STEP_PIN_ON > -1)
      digitalWrite(X_STEP_PIN, value);
#endif // #if defined(X_STEP_PIN) && (X_STEP_PIN_ON > -1)
    }
    else if(Y_AXIS == axis)
    {
#if defined(Y_STEP_PIN) && (Y_STEP_PIN_ON > -1)
      digitalWrite(Y_STEP_PIN, value);
#endif // #if defined(Y_STEP_PIN) && (Y_STEP_PIN_ON > -1)
    }
    else if(Z_AXIS == axis)
    {
#if defined(Z_STEP_PIN) && (Z_STEP_PIN_ON > -1)
      digitalWrite(Z_STEP_PIN, value);
#endif // #if defined(Z_STEP_PIN) && (Z_STEP_PIN_ON > -1)
    }
    else if(E_AXIS == axis)
    {
#if defined(E0_STEP_PIN) && (E0_STEP_PIN_ON > -1)
      digitalWrite(E0_STEP_PIN, value);
#endif // #if defined(E0_STEP_PIN) && (E0_STEP_PIN_ON > -1)
    }
    else if(B_AXIS == axis)
    {
#if defined(E1_STEP_PIN) && (E1_STEP_PIN_ON > -1)
      digitalWrite(E1_STEP_PIN, value);
#endif // #if defined(E1_STEP_PIN) && (E1_STEP_PIN_ON > -1)
    }
  }

  // 设置方向
  __inline void st_axis_write_dir(const volatile int axis, const volatile bool value)
  {
    if(X_AXIS == axis)
    {
#if defined(X_DIR_PIN) && (X_DIR_PIN_ON > -1)
      digitalWrite(X_DIR_PIN, value);
#endif // #if defined(X_DIR_PIN) && (X_DIR_PIN_ON > -1)
    }
    else if(Y_AXIS == axis)
    {
#if defined(Y_DIR_PIN) && (Y_DIR_PIN_ON > -1)
      digitalWrite(Y_DIR_PIN, value);
#endif // #if defined(Y_DIR_PIN) && (Y_DIR_PIN_ON > -1)
    }
    else if(Z_AXIS == axis)
    {
#if defined(Z_DIR_PIN) && (Z_DIR_PIN_ON > -1)
      digitalWrite(Z_DIR_PIN, value);
#endif // #if defined(Z_DIR_PIN) && (Z_DIR_PIN_ON > -1)
    }
    else if(E_AXIS == axis)
    {
#if defined(E0_DIR_PIN) && (E0_DIR_PIN_ON > -1)
      digitalWrite(E0_DIR_PIN, value);
#endif // #if defined(E0_DIR_PIN) && (E0_DIR_PIN_ON > -1)
    }
    else if(B_AXIS == axis)
    {
#if defined(E1_DIR_PIN) && (E1_DIR_PIN_ON > -1)
      digitalWrite(E1_DIR_PIN, value);
#endif // #if defined(E1_DIR_PIN) && (E1_DIR_PIN_ON > -1)
    }
  }

#ifdef ENDSTOPPULLUP_XMAX

  // 最大限位上拉
  __inline void st_axis_xyz_write_max(void)
  {
#if defined(X_MAX_PIN) && (X_MAX_PIN_ON > -1)
    digitalWrite(X_MAX_PIN, GPIO_PIN_SET);
#endif // #if defined(X_MAX_PIN) && (X_MAX_PIN_ON > -1)

#if defined(Y_MAX_PIN) && (Y_MAX_PIN_ON > -1)
    digitalWrite(Y_MAX_PIN, GPIO_PIN_SET);
#endif // #if defined(Y_MAX_PIN) && (Y_MAX_PIN_ON > -1)

#if defined(Z_MAX_PIN) && (Z_MAX_PIN_ON > -1)
    digitalWrite(Z_MAX_PIN, GPIO_PIN_SET);
#endif // #if defined(Z_MAX_PIN) && (Z_MAX_PIN_ON > -1)
  }

  // 读取最大限位状态
  __inline bool st_axis_xyz_read_max(int axis)
  {
    bool result = false;
    if(X_AXIS == axis)
    {
#if defined(X_MAX_PIN) && (X_MAX_PIN_ON > -1)
      result = (int)digitalRead(X_MAX_PIN) != X_MAX_ENDSTOP_INVERTING;
      if(result)
      {
        st_delay_us(10);//防抖
        result = (int)digitalRead(X_MAX_PIN) != X_MAX_ENDSTOP_INVERTING;
      }
#endif // #if defined(X_MAX_PIN) && (X_MAX_PIN_ON > -1)
    }
    else if(Y_AXIS == axis)
    {
#if defined(Y_MAX_PIN) && (Y_MAX_PIN_ON > -1)
      result = (int)digitalRead(Y_MAX_PIN) != Y_MAX_ENDSTOP_INVERTING;
      if(result)
      {
        st_delay_us(10);//防抖
        result = (int)digitalRead(Y_MAX_PIN) != Y_MAX_ENDSTOP_INVERTING;
      }

#endif // #if defined(Y_MAX_PIN) && (Y_MAX_PIN_ON > -1)
    }
    else if(Z_AXIS == axis)
    {
#if defined(Z_MAX_PIN) && (Z_MAX_PIN_ON > -1)
      result = (int)digitalRead(Z_MAX_PIN) != Z_MAX_ENDSTOP_INVERTING;
      if(result)
      {
        st_delay_us(10);//防抖
        result = (int)digitalRead(Z_MAX_PIN) != Z_MAX_ENDSTOP_INVERTING;
      }
#endif // #if defined(Z_MAX_PIN) && (Z_MAX_PIN_ON > -1)
    }
    return result;
  }

#endif // #ifdef ENDSTOPPULLUP_XMAX

#ifdef ENDSTOPPULLUP_ZMIN

  // 最小限位上拉
  __inline void st_axis_xyz_write_min(void)
  {
#if defined(X_MIN_PIN) && (X_MIN_PIN_ON > -1)
    digitalWrite(X_MIN_PIN, GPIO_PIN_SET);
#endif // #if defined(X_MIN_PIN) && (X_MIN_PIN_ON > -1)

#if defined(Y_MIN_PIN) && (Y_MIN_PIN_ON > -1)
    digitalWrite(Y_MIN_PIN, GPIO_PIN_SET);
#endif // #if defined(Y_MIN_PIN) && (Y_MIN_PIN_ON > -1)

#if defined(Z_MIN_PIN) && (Z_MIN_PIN_ON > -1)
    digitalWrite(Z_MIN_PIN, GPIO_PIN_SET);
#endif // #if defined(Z_MIN_PIN) && (Z_MIN_PIN_ON > -1)
  }

  // 读取最小限位状态
  __inline bool st_axis_xyz_read_min(const volatile int axis)
  {
    bool result = false;
    if(X_AXIS == axis)
    {
#if defined(X_MIN_PIN) && (X_MIN_PIN_ON > -1)
      result =(int)digitalRead(X_MIN_PIN) != X_MIN_ENDSTOP_INVERTING;
      if(result)
      {
        st_delay_us(10);//防抖
        result =(int)digitalRead(X_MIN_PIN) != X_MIN_ENDSTOP_INVERTING;
      }
#endif // #if defined(X_MIN_PIN) && (X_MIN_PIN_ON > -1)
    }
    else if(Y_AXIS == axis)
    {
#if defined(Y_MIN_PIN) && (Y_MIN_PIN_ON > -1)
      result = (int)digitalRead(Y_MIN_PIN) != Y_MIN_ENDSTOP_INVERTING;
      if(result)
      {
        st_delay_us(10);//防抖
        result = (int)digitalRead(Y_MIN_PIN) != Y_MIN_ENDSTOP_INVERTING;
      }
#endif // #if defined(Y_MIN_PIN) && (Y_MIN_PIN_ON > -1)
    }
    else if(Z_AXIS == axis)
    {
#if defined(Z_MIN_PIN) && (Z_MIN_PIN_ON > -1)
      result = (int)digitalRead(Z_MIN_PIN) != Z_MIN_ENDSTOP_INVERTING;
      if(result)
      {
        st_delay_us(10);//防抖
        result = (int)digitalRead(Z_MIN_PIN) != Z_MIN_ENDSTOP_INVERTING;
      }
#endif // #if defined(Z_MIN_PIN) && (Z_MIN_PIN_ON > -1)
    }
    return result;
  }

  __inline bool st_read_other_z_min_endstop(void)
  {
    bool result = false;
#if defined(E1_STEP_PIN) && (E1_STEP_PIN_ON > -1)
    result = (!digitalRead(E1_STEP_PIN));
    if(result)
    {
      st_delay_us(10);//防抖
      result = (!digitalRead(E1_STEP_PIN));
    }
#endif
    return result;
  }

#endif // #ifdef ENDSTOPPULLUP_ZMIN

  // 启动运动定时器
  __inline void stepper_timer_start(void)
  {
    HAL_TIM_Base_Start_IT(&htim4);
  }

  // 停止运动定时器
  __inline void stepper_timer_stop(void)
  {
    HAL_TIM_Base_Stop_IT(&htim4);
  }

  // 设置运动定时器
  __inline void stepper_timer_set_period(const volatile uint32_t period_value)
  {
    htim4.Instance->ARR = period_value;
    htim4.Init.Period = period_value;
  }
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // STEPPER_PIN_H

