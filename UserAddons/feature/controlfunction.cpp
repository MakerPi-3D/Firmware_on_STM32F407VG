#include "controlfunction.h"
#include "filamentcontrol.h"
#include "lightcontrol.h"
#include "simplefunction.h"
#include "boardtest.h"
#include "poweroffrecovery.h"
#include "guicontrol.h"
#include "gcodebufferhandle.h"
#include "temperature.h"
#include "stepper.h"
#include "planner.h"
#include "user_debug.h"
#include "materialcheck.h"
#include "config_model_tables.h"
#include "user_interface.h"
#include "stm32f4xx_hal.h"
#include "uart.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#ifdef __cplusplus
extern "C" {
#endif

  extern INT IsPrint(void);



#ifdef __cplusplus
} // extern "C" {
#endif

void control_function_init(void)
{
  // 堵料检测初始化
  blockDetect.init();

  // 断料检测初始化
  materialCheck.init();

  // 堵料检测开启定时器2
  if((0U != t_sys_data_current.enable_block_detect))
  {
    sys_time2_start();
  }
}

void control_function_process(void)
{
  if((0 == IsPrint()))     // 串口打印和U盘打印情况下不执行下面函数
  {
    filamentControl.process();                 // 进退丝操作入口
    if(0U != motion_3d.enable_board_test)    // 测试固件入口
    {
      boardTest.process();
    }
    powerOffOperation.getZMaxPos();            // 校准z高度入口
  }
  (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  guiControl.refreshGuiInfo();
  if(t_sys_data_current.enable_powerOff_recovery)
  {
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    powerOffOperation.recoveryPrint();           // 断电恢复入口
  }
  (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  simpleFunction.process();
  (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  lightControl.process();                      // 灯控制入口
  if(t_sys_data_current.enable_block_detect)                // 堵料检测入口
  {
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    BlockDetectControl();
  }
  if(t_sys_data_current.enable_material_check)                // 堵料检测入口
  {
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    materialCheck.process();                     // 断料检测入口
  }
  (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
  soundControl.beepAlarm();
  usart_process();
}

void buzz(uint16_t value)
{
  soundControl.buzz(value);
}

void beep(uint16_t value)
{
  soundControl.beep(value);
}

void TIM2_IRQHandler_process(void)
{
  if(0U != t_sys_data_current.enable_block_detect)
  {
    block_detect_change_status();
    blockDetect.ttc_update();
    blockDetect_process();
  }
}

void TIM6_IRQHandler_process(void)
{
  sg_grbl::temperature_update();
}

void TIM4_IRQHandler_process(void)
{
  sg_grbl::st_process();
}



