//#include <stm32f4xx.h>
#include "ref_data_task.h"
#include "config_model_tables.h"
#include "watch_dog.h"
#include "temperature.h"
#include "machinecustom.h"
#include "PrintControl.h"
#include "midwaychangematerial.h"
#include "RespondGUI.h"
#include "interface.h"
#include "controlfunction.h"
#include "globalvariables.h"
#include "infrared_bed_level_adjust.h"
#include "infrared_z_zero_adjust.h"
#include "user_fatfs.h"
#include "USBFileTransfer.h"
#include "Alter.h"
#include "user_interface.h"
#include "mechanical_bed_level_adjust.h"
#include "view_pic_display.h"
#include "poweroffrecovery.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif

//  uint8_t pcWriteBuffer[500];

//  //打印各个任务历史运行中剩余的最小栈空间量
//  void PrintMinStackSize(void)
//  {
//    extern osThreadId InitTaskHandle;
//    extern osThreadId RefDataTaskHandle;
//    extern osThreadId ReadUdiskTaskHandle;
//    extern osThreadId PrintTaskHandle;
//    extern osThreadId GUITaskHandle;
//    extern osThreadId RespondGUITaskHandle;
//    extern USBH_HandleTypeDef hUsbHostFS;
//    static ULONG NextPrintMinStackSize=0;
//    if(NextPrintMinStackSize<sys_task_get_tick_count())
//    {
//      sys_task_enter_critical();
//      INT MinStackSize;
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(InitTaskHandle);    //返回的是字，不是字节
//      printf("InitTask MinStackSize:%d\r\n",MinStackSize);
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(RefDataTaskHandle);          //该函数放到RefDataTask_OS中，传入NULL查询的是本任务
//      printf("RefDataTask MinStackSize:%d\r\n",MinStackSize);
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(ReadUdiskTaskHandle);
//      printf("ReadUdiskTask MinStackSize:%d\r\n",MinStackSize);
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(PrintTaskHandle);
//      printf("PrintTask MinStackSize:%d\r\n",MinStackSize);
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(GUITaskHandle);
//      printf("GUITask MinStackSize:%d\r\n",MinStackSize);
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(RespondGUITaskHandle);
//      printf("RespondGUITask MinStackSize:%d\r\n",MinStackSize);
//      MinStackSize=(INT)uxTaskGetStackHighWaterMark(hUsbHostFS.thread);
//      printf("UdiskTask MinStackSize:%d\r\n\r\n",MinStackSize);
//      NextPrintMinStackSize=sys_task_get_tick_count()+1000;//5*60*1000UL;  //5min
//      sys_task_exit_critical();
//    }
////			PrintMinStackSize();
////    printf("LED Toggle\n");
////    vTaskList((PCHAR)&pcWriteBuffer);
////    printf("Name      State Priority   Stack Num\r\n");
////    printf("%s\r\n", pcWriteBuffer);
//  }

  void UpdateGUIInfo(void)
  {
    // RefreshGUI
    if(IsPrint())
    {
      t_gui.printed_time_sec = printControl.getTime();
    }
    t_gui.print_percent = printFileControl.getPercent();
		if(t_gui.print_percent > 99)
			t_gui.print_percent = 99;
  }

//电路板问题，20170502
//有些电路板有问题，导致系统上电就会自动加热，并且加热功率为最大，如果不断电则后果严重

#if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF
  // 检测打印状态下，热敏电阻状态
  void check_print_extruder_thermistor_status(void)
  {
    // 打印状态且已经加热完成，温度突然下降10度，判断热敏电阻脱落，需要测试
    static INT extruder_tmp_save_status = 0;
    static INT extruder_tmp_save = 0;
    static INT extruder_tmp_target_save = 0;
    static ULONG extruder_tmp_save_peroid = 0;
    static INT extruder_tmp_save_time_count = 0;

    // 只有在打印状态，且执行M109指令完成才进行以下操作
    if(IsPrint() && (1U == t_gui_p.m109_heating_complete))
    {
      // 目标温度与当前温度差值小于3，开启检测热敏电阻状态
      if((0 == extruder_tmp_save_status) && (abs((INT)(sg_grbl::temperature_get_extruder_target(0) - sg_grbl::temperature_get_extruder_current(0))) < 3))
      {
        extruder_tmp_save_status = 1;
        extruder_tmp_save = sg_grbl::temperature_get_extruder_current(0);
        extruder_tmp_target_save = sg_grbl::temperature_get_extruder_target(0);
      }
      // 打印过程中更新目标温度，检测状态重新设置
      if(extruder_tmp_target_save != sg_grbl::temperature_get_extruder_target(0))
      {
        extruder_tmp_save_status = 0;
        extruder_tmp_target_save = sg_grbl::temperature_get_extruder_target(0);
        return;
      }
    }
    else
    {
      extruder_tmp_save_status = 0;
    }

    if(extruder_tmp_save_status)
    {
      // 秒数累加
      if ( extruder_tmp_save_peroid < sys_task_get_tick_count() )
      {
        extruder_tmp_save_peroid = sys_task_get_tick_count() + 1000;
        ++extruder_tmp_save_time_count;
      }

      // 5秒内记录的温度比当前温度大15度，判断热敏电阻脱落导致此温度差
      if(((extruder_tmp_save - sg_grbl::temperature_get_extruder_current(0)) >= 22) && (extruder_tmp_save_time_count == 5))
      {
        PopWarningInfo(ThermistorFallsWarning);
        extruder_tmp_save_time_count = 0;
      }
      else
      {
        if(extruder_tmp_save_time_count > 5)
        {
          extruder_tmp_save_time_count = 0;
        }
      }
    }
  }
#endif // #if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF

  /*
   * 当温度超过最大温度时就发生警报，因温度最大只能是HEATER_0_MAXTEMP-25
   * 已经注释的代码不稳定，经常乱报警，所以方案不可行
   * 2017/8/22
  */
  void temperature_error_pop(void)
  {
    if(DETECT_PCB_FAULSE == sg_grbl::temperature_get_error_status())
    {
      display_picture(95);//警告界面
//      gui_send_sem_open_beep();
      respond_gui_send_sem(OpenBeep);
    }
    else if(MaxTempBedError == sg_grbl::temperature_get_error_status())
    {
      PopWarningInfo(MaxTempBedWarning);
    }
    else if(MinTempError == sg_grbl::temperature_get_error_status())
    {
      PopWarningInfo(MinTempWarning);
    }
    else if(MaxTempError == sg_grbl::temperature_get_error_status())
    {
      PopWarningInfo(MaxTempWarning);
    }
    else if(HeatFailError == sg_grbl::temperature_get_error_status())
    {
      PopWarningInfo(HeatFailWarning);
    }
    else if(ThermistorFallsOffError == sg_grbl::temperature_get_error_status())
    {
      PopWarningInfo(ThermistorFallsWarning);
    }
#if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF
    if(1 == t_sys.is_detect_extruder_thermistor)
    {
      check_print_extruder_thermistor_status();
    }
#endif // #if CHECK_EXTRUDER_THERMISTOR_FALLS_OFF
  }

  void manage_synchronize()
  {
    sg_grbl::temperature_manage_heater(gcode::fan_speed);   // 加热控制
//    gcode::set_fan_speed(temperature_get_current_ext_fanspeed());
    gcode::manage_inactivity();           // gcode延时控制
    temperature_error_pop();       // 检测是否电路板有问题，导致喷嘴一直加热
    if(sg_grbl::temperature_update_pid_output_factor())
    {
      SavePidOutputFactorValue();
    }
  }

  static void v5_extruder_fan_control(const bool isPrint)
  {
    // 是否切换高温标志，大于230度为高温
    static bool isHighTemp = false;
    // V5喷嘴类型1：代表E电机风扇与喉管风扇并联在一起
    // V5喷嘴类型2：代表喷嘴风扇与喉管风扇并联在一起
    if((1 == t_sys_data_current.enable_v5_extruder) || (2 == t_sys_data_current.enable_v5_extruder))
    {
      // V5喷嘴功能打开，混色风扇一直开启
      gpio_color_mix_control(true);

      // 如果喷嘴目标温度小于230度，一直打开喉管风扇，类型1与类型2一致
      if(sg_grbl::temperature_get_extruder_target(0) < 230)
      {
        if(isHighTemp)
        {
          sys_task_enter_critical();
          // 还原默认温度配置
          sg_grbl::temperature_init();
          sys_task_exit_critical();
          isHighTemp = false;
        }
//        if(2 == t_sys_data_current.enable_v5_extruder && isPrint)
//        {
//          gcode::set_fan_speed(255); // V5喷嘴部分机器，喷嘴风扇与扇热风扇一起
//        }
//        else if((1 == t_sys_data_current.enable_v5_extruder) && isPrint )
//        {
//          gpio_e_motor_fan_control(true);
//        }
      }
      else
      {
        // 非打印状态不切换高温模式
        if(!isPrint)
        {
          return;
        }
        // 高温状态下，喉管风扇关闭
        if(2 == t_sys_data_current.enable_v5_extruder)
        {
          gcode::set_fan_speed(gcode::fan_speed); // V5喷嘴部分机器，喷嘴风扇与扇热风扇一起
        }
        else if(1 == t_sys_data_current.enable_v5_extruder)
        {
          if(sg_grbl::temperature_get_extruder_target(0) > 280)
          {
            if(PICTURE_IS_JAPANESE!=t_sys_data_current.pic_id)
            {
              gpio_e_motor_fan_control(false);
            }

          }
          else
          {
            return;
          }
        }
        if(!isHighTemp)
        {
          // 重置高温pid值
          sys_send_gcode_cmd("M301 P8.001658 I0.216086 D98.767128 isInternal");
          isHighTemp = true;
        }
      }
    }
  }

  void UpdatePrintStatus(void)
  {
    RefChangeFilamentStatus();
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    manage_synchronize();
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    // 停止打印乱跑，过滤乱跑指令
    printControl.processStop();
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    v5_extruder_fan_control(IsPrint());
  }

  void UpdateNoPrintStatus(void)
  {
    (void)sys_os_delay(50);  //延时以让低优先级的任务执行
    printControl.processPause();
    (void)sys_os_delay(50);  //延时以让低优先级的任务执行
    printControl.processResume();

    (void)sys_os_delay(50);  //延时以让低优先级的任务执行
    IsTranFileErr(); //传输文件是否错误，比如传输中突然拔线，采用的是超时检测
#ifdef CAL_Z_ZERO_OFFSET
    (void)sys_os_delay(50);  //延时以让低优先级的任务执行
    infrared_z_zero_adjust_interface();
    (void)sys_os_delay(50);  //延时以让低优先级的任务执行
    if(!t_sys_data_current.IsMechanismLevel)
      infrared_bed_level_adjust_interface();
    else
      mechanical_bed_level_adjust_interface();
#endif
    (void)sys_os_delay(50);  //延时以让低优先级的任务执行
    v5_extruder_fan_control(false);
  }

  void ref_data_task_loop(void)
  {
    iwdg_refresh();
    UpdateUdiskStatus();
    (void)sys_os_delay(task_schedule_delay_time);  //延时以让低优先级的任务执行
    UpdatePrintStatus();                      // 更新打印状态
    if((!IsPrint()))  // 串口打印和U盘打印情况下不执行下面函数
    {
      UpdateNoPrintStatus();                  // 更新GUI相关数据
    }
//    if(!serialPrintStatus())                  // 串口打印没打开时，更新界面gui数据
    UpdateGUIInfo();
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行
    control_function_process();
    (void)sys_os_delay(task_schedule_delay_time);                        // 延时以让低优先级的任务执行

    if((0U != t_sys_data_current.enable_powerOff_recovery) && (0U == t_power_off.is_power_off)) // 断电开启，非断电状态
    {
      (void)sys_os_delay(task_schedule_delay_time);
      poweroff_set_data();

      if((0 != IsPrint()))
      {
        (void)sys_os_delay(task_schedule_delay_time);
        poweroff_sync_data();
      }
    }
  }

#ifdef __cplusplus
} // extern "C" {
#endif







