#ifndef TEMPERATURE_PIN_H
#define TEMPERATURE_PIN_H

#include "stm32f4xx_hal.h"
#include "pins.h"
#include "threed_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

  extern TIM_HandleTypeDef htim6;
  extern ADC_HandleTypeDef hadc1;

  // 温度定时器开启
  __inline void temperature_timer_start(void)
  {
    HAL_TIM_Base_Start_IT(&htim6);
  }

  // 温度定时器关闭
  __inline void temperature_timer_stop(void)
  {
    HAL_TIM_Base_Stop_IT(&htim6);
  }

  // 设置热床状态
  __inline void temperature_set_bed_heater_status(bool on)
  {
#if defined(TEMP_BED_PIN) && (TEMP_BED_PIN_ON > -1)
#if defined(HEATER_BED_PIN) && (HEATER_BED_PIN_ON > -1)
    digitalWrite(HEATER_BED_PIN,on);
#endif
#endif
  }

  // 设置喷嘴状态
  __inline void temperature_set_extruders_heater_status(bool on, int extruder)
  {
    if(0 == extruder)
    {
#if defined(TEMP_0_PIN) && (TEMP_0_PIN_ON > -1)
#if defined(HEATER_0_PIN) && (HEATER_0_PIN_ON > -1)
      digitalWrite(HEATER_0_PIN,on);
#endif
#endif
    }
    else if(1 == extruder)
    {
#if defined(TEMP_1_PIN) && (TEMP_1_PIN_ON > -1) && EXTRUDERS > 1
#if defined(HEATER_1_PIN) && (HEATER_1_PIN_ON > -1)
      WRITE(HEATER_1_PIN,on);
#endif
#endif
    }
    else if(2 == extruder)
    {
#if defined(TEMP_2_PIN) && (TEMP_2_PIN_ON > -1) && EXTRUDERS > 2
#if defined(HEATER_2_PIN) && (HEATER_2_PIN_ON > -1)
      WRITE(HEATER_2_PIN,on);
#endif
#endif
    }
  }

  // 温度获取，切换adc通道
  __inline void temperature_adc_status_switch(int id)
  {
    ADC_ChannelConfTypeDef sConfig;
    if(0 == id)
    {
#if defined(TEMP_0_PIN) && (TEMP_0_PIN_ON > -1)
      sConfig.Channel = TEMP_0_PIN_ADC_CHANNEL;
      sConfig.Rank = 1U;
      sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
      (void)HAL_ADC_ConfigChannel(&hadc1, &sConfig);
      (void)HAL_ADC_Start(&hadc1);
#endif
    }
    else if(1 == id)
    {
#if defined(TEMP_BED_PIN) && (TEMP_BED_PIN_ON > -1)
      sConfig.Channel = TEMP_BED_PIN_ADC_CHANNEL;
      sConfig.Rank = 1U;
      sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
      (void)HAL_ADC_ConfigChannel(&hadc1, &sConfig);
      (void)HAL_ADC_Start(&hadc1);
#endif
    }
    else if(2 == id)
    {
#if defined(TEMP_CAVITY_PIN) && (TEMP_CAVITY_PIN_ON > -1)
      sConfig.Channel = TEMP_CAVITY_PIN_ADC_CHANNEL;
      sConfig.Rank = 1U;
      sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
      (void)HAL_ADC_ConfigChannel(&hadc1, &sConfig);
      (void)HAL_ADC_Start(&hadc1);
#endif
    }
  }

  // 温度获取ADC值
  __inline uint32_t temperature_adc_get_value(int id)
  {
    uint32_t result = 0U;
    if(0 == id)
    {
#if defined(TEMP_0_PIN) && (TEMP_0_PIN_ON > -1)
      result = HAL_ADC_GetValue(&hadc1);
#endif
    }
    else if(1 == id)
    {
#if defined(TEMP_BED_PIN) && (TEMP_BED_PIN_ON > -1)
      result = HAL_ADC_GetValue(&hadc1);
#endif
    }
    else if(2 == id)
    {
#if defined(TEMP_CAVITY_PIN) && (TEMP_CAVITY_PIN_ON > -1)
      result = HAL_ADC_GetValue(&hadc1);
#endif
    }
    else
    {
//    return 0;
    }
    return result;
  }

  // 初始化腔体控制IO
  __inline void temperature_cavity_pin_init(bool is_open_cavity)
  {
    if(is_open_cavity)
    {
      // 引脚初始化
      GPIO_InitTypeDef GPIO_InitStruct;
      __GPIOB_CLK_ENABLE();

      // B相
      GPIO_InitStruct.Pin = HEATER_CAVITY_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
      HAL_GPIO_Init(HEATER_CAVITY_PIN_GPIO, &GPIO_InitStruct);
      HAL_GPIO_WritePin(HEATER_CAVITY_PIN_GPIO, HEATER_CAVITY_PIN, GPIO_PIN_SET);
    }
  }

  // 设置腔体状态
  __inline void temperature_set_cavity_pin_status(bool is_on)
  {
    digitalWrite(HEATER_CAVITY_PIN, (!is_on));
  }


#ifdef __cplusplus
} //extern "C" {
#endif

#endif // TEMPERATURE_PIN_H

