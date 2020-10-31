#include "respond_gui_task.h"
#include "config_model_tables.h"
#include "interface.h"
#include "user_debug.h"
#include "guicontrol.h"
#include "filamentcontrol.h"
#include "infrared_z_zero_adjust.h"
#include "UdiskControl.h"
#include "PrintControl.h"
#include "controlfunction.h"
#include "midwaychangematerial.h"
#include "globalvariables.h"
#include "machinecustom.h"
#include "boardtest.h"
#include "infrared_bed_level_adjust.h"
#include "autobedleveling.h"
#include "RespondGUI.h"
#include "functioncustom.h"
#include "user_interface.h"
#include "Alter.h"
#include "mechanical_bed_level_adjust.h"
#include "poweroffrecovery.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif
  extern UINT8 MinTempWarningPopSet;
  extern void temperature_set_error_status(CHAR Value);
  extern INT temperature_get_error_status(void);

  void ScanGUISemStatus(void)
  {
    switch (SettingInfoToSYS.GUISempValue)
    {
    case BackZeroValue:  //回零
      USER_DbgLog("BackZero ok!");
#ifdef CAL_Z_ZERO_OFFSET
      // M3145K归零接口
      if(2 == t_sys_data_current.enable_bed_level)
      {
//        if(1 == t_sys.is_bed_level_down_to_zero)
        {
          sys_send_gcode_cmd("G28 isInternal");
        }
//        else
//        {
//          infrared_z_zero_adjust_autohome();
//        }
      }
      else
      {
        sys_send_gcode_cmd("G28 isInternal");
      }
#else
      sys_send_gcode_cmd("G28 isInternal");
#endif
      for(INT i = 0; i < 3; ++i)
        t_gui.move_xyz_pos[i] = motion_3d_model.xyz_home_pos[i];
      break;
    case DisableStepValue:  //解锁电机
      guiControl.disableSteppers();
      break;
    case PreHeatPLAValue:  //预热PLA
      guiControl.preHeatPLA();
      break;
    case CoolDownValue:  //冷切
      USER_DbgLog("CoolDown ok!");
      guiControl.coolDown();
      break;
    case FeedFilamentValue:  //进料
      filamentControl.startLoad();
      break;
    case StopFeedFilamentValue:  //退出进料
//      USER_EchoLogStr("DEBUG: StopFeedFilament ok!\r\n");
      filamentControl.cancelProcess();
      break;
    case BackFilamentValue:  //退料
      filamentControl.startUnload();
      break;
    case StopBackFilamentValue:  //退出退料
//      USER_EchoLogStr("DEBUG: StopBackFilament ok!\r\n");
      filamentControl.cancelProcess();
      break;
    case OpenSDCardValue:  //打开SD卡
      USER_DbgLog("OpenSDCard ok!");
      GUIOpenSDCard();
      serial_print[0] = false;
      serial_print[1] = false;
      break;
    case OpenDirValue:  //打开目录
      USER_DbgLog("OpenDir ok!");
      GUIOpenSDDir();
      break;
    case BackLastDirValue:  //返回上层目录
      USER_DbgLog("BackLastDir ok!");
      GUIBackSDLastDir();
      break;
    case NextPageValue:  //下一页
      USER_DbgLog("NextPage ok!");
      GUINextPage();
      break;
    case LastPageValue:  //上一页
      USER_DbgLog("LastPage ok!");
      GUILastPage();
      break;
    case FilePrintValue:  //选中文件确定打印
      USER_DbgLog("FilePrint ok!");
//      MinTempWarningPopSet=0;
      gcode::printing_material_length = 0;
      if(3 == temperature_get_error_status())
      {
        temperature_set_error_status(0);
      }
      t_gui_p.m109_heating_complete = 0U;  //打印时重置为还没有加热完成
      t_sys.print_time_save = 0;
      printControl.start(false);
      IsCompleteHeat(); //解决一开始就跳转到有中途换料的界面的问题
      break;
    case PausePrintValue:  //暂停打印
      USER_DbgLog("PausePrint ok!");
      printControl.pause(false);
      break;
    case ResumePrintValue:  //继续打印
      USER_DbgLog("ResumePrint ok!");
      printControl.resume(false);
      break;
    case StopPrintValue:  //停止打印
      USER_DbgLog("StopPrint ok!");
      printControl.stop(false);
      guiControl.coolDown();
      break;
    case OpenBeep:  //打开蜂鸣器
      USER_DbgLog("OpenBeep ok!");
      if(t_sys.alarm_sound)
      {
        t_gui_p.isBeepAlarm = 1;
        gpio_beep_control(false);
      }
      break;
    case CloseBeep:  //关闭蜂鸣器
      USER_DbgLog("CloseBeep ok!");
      t_gui_p.isBeepAlarm = 0;
      break;
    case SysErrValue:  //系统错误
      ManagWarningInfo();
      break;
    case PrintSetValue_M14:  //M14机型 打印设置
      guiControl.printSetForM14();
      break;
    case PreHeatABSValue:  //预热ABS
      guiControl.preHeatABS();
//    GUI_PreHeatABS();
      break;
    case MoveXYZValue:  //移动光轴确定键
      guiControl.moveXYZ();
//    GUI_MoveXYZ(t_gui.move_x_pos,t_gui.move_y_pos,t_gui.move_z_pos);
      break;
    case ConfirmChangeFilamentValue:  //确认中途换料
      ChangeFilament();
      break;
    case ConfirmLoadFilamentValue:  //中途换料中确认进料
      USER_EchoLogStr("ConfirmLoadFilamentValue ok!\r\n");//串口上传信息到上位机2017.7.6
		  gcode::is_confirm_load_filament = true;
      break;
    case ConfirmChangeModelValue:  //更改机型
      USER_EchoLogStr("ConfirmChangeModelValue ok!\r\n");//串口上传信息到上位机2017.7.6
      SaveSelectedModel();
      break;
    case ConfirmChangePictureValue:  //
      SaveSelectedPicture();
      break;
    case ConfirmChangeFunctionValue:  //更改功能
//      USER_EchoLogStr("ConfirmChangeFunctionValue ok!\r\n");//串口上传信息到上位机2017.7.6
      SaveSelectedFunction();
      break;
    case PrintSetValue_NotM14_Left:  //非M14机型 点击左上方 打印设置
      guiControl.printSetForNotM14Left();
      break;
    case PrintSetValue_NotM14_Right:  //非M14机型 点击右上方 打印设置
      guiControl.printSetForNotM14Right();
      break;
    case ConfirmPowerOffRecover:  //断电续打确认键
      poweroff_ready_to_recover_print();
      break;
    case CancelPowerOffRecover:  //断电续打取消
      if(0 != t_sys_data_current.enable_powerOff_recovery)
      {
        poweroff_delete_file_from_sd();
        poweroff_reset_flag(); //重置标志位
        t_gui_p.isBeepAlarm = 0;
      }
      break;
    case CalculateZMaxPos:  //测量行程
      poweroff_start_cal_z_max_pos();
      break;
    case MatCheckCalibrateValue:  //断料检测模块校准
      break;
    case PauseToResumeNozzleTemp: //断料状态下去换料，先把暂停打印降低的温度恢复
      USER_EchoLogStr("PauseToResumeTemp ok!\r\n");//串口上传信息到上位机2017.7.6
      //PauseToResumeNozTemp();
      PauseToResumeTemp();
      break;
    case StepTestValue:  //电机测试
      boardTest.toggleStepStatus();
      break;
    case FanTestValue:  //风扇测试
      boardTest.toggleFanStatus();
      break;
    case HeatTestValue:  //加热测试
      boardTest.toggleHeatStatus();
      break;
    case RunMaxPos:  //加热测试
      boardTest.runMaxPos();
      break;
    case CalHeatTime:  //加热計時
      boardTest.calHeatTime();
      break;
    case ConfirmChangelogoValue:  //确定logo设置
      SaveSelectedlogo();
      break;
    case CalBedLevel:  //平台校准初始化
      start_calculate_bed_level();
      break;
    case CalBedLevelZAtLF:  //校准左前角
      get_bed_level_position_z_at_lf();
      break;
    case CalBedLevelZAtLB:  //校准左后角
      get_bed_level_position_z_at_lb();
      break;
    case CalBedLevelZAtMiddle:  //校准中间点
      get_bed_level_position_z_at_middle();
      break;
    case CalBedLevelZAtRB:  //校准右后点
      get_bed_level_position_z_at_rb();
      break;
    case CalBedLevelZAtRF:  //校准右前点
      get_bed_level_position_z_at_rf();
      break;
    case CalBedLevelFinish:  //校准完成
      calculate_bed_level_finish();
      break;
    case PreHeatBedValue:
      guiControl.preHeatBed();
      break;
    case PrintSetValue_Cavity:
      guiControl.printSetForCavity();
      break;
#ifdef CAL_Z_ZERO_OFFSET
    case StartCalZZero:
      isCalZero = true;
      break;
    case CancelCalZZero:
      SaveZOffsetZero() ;
      isCalZero = false;
      break;
      // 校准平台接口
    case StartCalBedLevel:
      if(!t_sys_data_current.IsMechanismLevel)
        infrared_bed_level_adjust_start();
      else
        mechanical_bed_level_adjust_start();
      break;
#endif
    default:
      USER_DbgLog("GUISempValue is not effective Value");
    }

    task_gui_wait_release();
  }


  void respond_gui_task_loop(void)
  {
    if(task_is_gui_send_wait_done())  //GUI信号量处理
    {
      ScanGUISemStatus();
      (void)sys_os_delay(80);
    }
  }


#ifdef __cplusplus
} // extern "C" {
#endif



