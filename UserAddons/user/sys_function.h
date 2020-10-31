#ifndef SYS_FUNCTION_H
#define SYS_FUNCTION_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "globalvariables_ccmram.h"

#ifdef __cplusplus
extern "C" {
#endif
  extern osMessageQId GcodeCommandHandle;

  __inline void sys_os_delay(const int tick_value)
  {
    osDelay(tick_value);
  }

  __inline void sys_task_enter_critical(void)
  {
    HAL_NVIC_SetPriority(TIM4_IRQn, 4, 0);
    taskENTER_CRITICAL();
  }

  __inline void sys_task_exit_critical(void)
  {
    taskEXIT_CRITICAL();
    HAL_NVIC_SetPriority(TIM4_IRQn, 5, 0);
  }

  __inline unsigned long sys_task_get_tick_count(void)
  {
    return xTaskGetTickCount();
  }

  __inline void sys_send_gcode_cmd_delay(bool isDelay)
  {
    osMessagePut(GcodeCommandHandle, (uint32_t)command_buffer_head, 10U);
    if(isDelay)
    {
      sys_os_delay(50);
    }
    command_buffer_head = (command_buffer_head+1)%BUFSIZE;
    for(int i = 0; i < CMD_BUF_SIZE; i++)
    {
      command_buffer[command_buffer_head][i] = 0;
    }
  }

  __inline void sys_send_gcode_cmd(const char* gcode_cmd)
  {
    for(int i = 0; i < CMD_BUF_SIZE; i++)
    {
      command_buffer[command_buffer_head][i] = gcode_cmd[i];
    }
    sys_send_gcode_cmd_delay(true);
  }

#ifdef __cplusplus
} //extern "C"
#endif

#endif // SYS_FUNCTION_H

