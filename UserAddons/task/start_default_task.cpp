#include "start_default_task.h"
#include "user_debug.h"
#include "user_fatfs.h"
#include "delay.h"
#include "machinecustom.h"
#include "controlfunction.h"
#include "ConfigurationStore.h"
#include "machine_custom.h"
#include "infrared_z_zero_adjust.h"
#include "UdiskControl.h"
#include "fatfs.h"
#include "config_model_tables.h"
#include "USBFileTransfer.h"
#include "globalvariables.h"
#include "autobedlevelinterface.h"
#include "lcd.h"
#include "touch.h"
#include "view_pic_display.h"
#include "Alter.h"
#include "user_interface.h"
#include "poweroffrecovery.h"
#include "stepper.h"
#include "planner.h"
#include "temperature.h"
#include "machine.h"
#include "interface.h"
#ifdef __cplusplus
extern "C" {
#endif
//  extern osSemaphoreId ReceiveUartCmdHandle;


void start_default_task_init(void)
{
  while (BSP_SD_Init() != MSD_OK)
    sys_delay(10);

  USER_EchoLog("SDCard Insert!");
  //挂载SD卡和U盘
  f_mount_SDCard();
  f_mount_Udisk();
  //一些延时所需的初始化
  delay_init(168);//20170603提前延时函数初始化，适应舵机运动
  globalvariables_init();
  // 加载系统配置文件
  sysconfig_init();
  //打印机的一些参数初始化
  sg_grbl::Config_ResetDefault();
  // 机器型号初始化
  sg_machine::machine_init();
  // 自动调平初始化
  auto_bed_level_adjust_init();
  // 控制功能初始化
  control_function_init();
  machine_custom_init();
  sg_grbl::planner_init();
  sg_grbl::st_init(); //电机初始化
  sg_grbl::temperature_init(); //温度控制初始化
  #ifdef CAL_Z_ZERO_OFFSET
  infrared_adjust_init();
  #endif
  HAL_Delay(100);
  poweroff_data_init();
  //删除SD卡里的gcode文件
  DelectSDFiles();
  SettingInfoToSYS.GUISempValue = -1;

  for (INT i = 0; i < 3; ++i)
    t_gui.move_xyz_pos[i] = motion_3d_model.xyz_home_pos[i];

  if (2 == t_sys_data_current.enable_bed_level  && !t_sys_data_current.IsMechanismLevel)
  {
    #ifdef CAL_Z_ZERO_OFFSET
    gpio_infrared_level_init();
    #endif
  }

  gui_init_lcd_touch();
}



void start_default_task_loop(void)
{
  task_receive_uart_wait();
  //    (void)osSemaphoreWait(ReceiveUartCmdHandle, osWaitForever); // 等待信号量
  (void)sys_os_delay(task_schedule_delay_time); // 延时以让低优先级的任务执行
  SaveUSBFile(); // 存储从电脑端发过来的文件
  (void)sys_os_delay(task_schedule_delay_time); // 延时以让低优先级的任务执行
}


#ifdef __cplusplus
} // extern "C" {
#endif

