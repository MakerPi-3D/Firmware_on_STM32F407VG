#include "PrintControl.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "user_debug.h"
#include "gcodebufferhandle.h"
#include "autobedlevelinterface.h"
#include "controlxyz.h"
#include "controlfunction.h"
#include "user_interface.h"
#include "fatfs.h"
#include "planner.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "poweroffrecovery.h"
#include "gcode_global_params.h"
#include "Alter.h"
#include "file_sgcode.h"
#include "file_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "stepper.h"
#include "gcode.h"
#include "interface.h"
#include "RespondGUI.h"
#include  "USBFileTransfer.h"
#include "config_model_tables.h"

#include "temperature.h"
#include "user_debug.h"
#include "view_pic_display.h"
#include "user_fatfs.h"
#include "sysconfig_data.h"
  extern UINT32 old_sd_pos[6];/*20170807堵料修复*/
/////////////////////////////////////////////////////////////////////////////////////
///////////////////////// PrintControl private variables/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#define SD_CARD 1                                  /*!< SD卡存储介质 */
#define UDISK 2                                    /*!< U盘存储介质 */

  static UINT8 medium_id = UDISK;                /*!< 记录打印文件存储介质 */
  static ULONG starttime = 0;              /*!< 打印时间计数 */

  // 打印状态
  static volatile bool is_printing = false;                 /*!< 是否正在打印 */
  static volatile bool is_pause_printing = false;           /*!< 是否正在暂停打印 */
  static volatile bool is_stop_printing = false;            /*!< 是否正在停止打印 */
  static volatile bool is_resume_printing = false;          /*!< 是否正在恢复止打印 */
  static volatile bool is_finish_print = false;             /*!< 是否完成了打印 */
  static volatile bool is_poweroff_recovery_print = false;  /*!< 是否为断电恢复操作 */

  // 暂停打印后，等待电机停止，停止后移开喷嘴
  static INT pause_print_status = 0;               /*!< 暂停打印状态 */
  static bool is_heating_status = false;           /*!< 是否处于打印加热状态 */
  static TickType_t xTimeToCoolDown = 0;           /*!< 冷却超时时间 */
  static INT temp_hotend_to_resume = 0;            /*!< 恢复打印喷嘴温度 */
  static INT temp_bed_to_resume = 0;               /*!< 恢复打印热床温度 */
  static FLOAT OldPosition[5];                     /*!< 恢复打印XYZ位置 */
  static INT OldFeedRate;                          /*!< 恢复打印进料速度 */
  static bool is_pause_to_cool_down = false;       /*!< 是否暂停温度降低 */
  static bool is_pause_to_resume_temp = false;     /*!< 是否暂停恢复温度 */

  // 停止打印
  static bool is_process_stop_print = false;       /*!< 是否执行停止打印 */

  // 暂停打印
  static bool is_process_pause_print_done = false;       /*!< 是否执行暂停打印完成 */

#define FILE_GCODE 0
#define FILE_SGCODE 1
#define FILE_GSD 2
  static UCHAR fileType = FILE_GCODE;      /*!< 打印文件类型 */

/////////////////////////////////////////////////////////////////////////////////////
///////////////////// PrintFileControl private variables/////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

  static CHAR printfilepathname[MK_MAX_LFN];             /*!< 文件名称 */
  static volatile ULONG file_pos = 0;          /*!< 文件位置 */
  static UCHAR read_data_error_count = 0;      /*!< 读U盘文件异常计数 */
  static FIL printfile;                                /*!< 文件对象 */
  static bool is_print_sd_file = false;                /*!< 是否打印sd卡文件 */

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////  private function/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

  bool isProcessPausePrintDone(void)
  {
    return is_process_pause_print_done;
  }

  INT get_pause_extruder_target_temp(void)
  {
    return temp_hotend_to_resume;
  }

  static void reset_print_status(void)
  {
    is_printing = false;
    is_pause_printing = false;
    is_stop_printing = false;
    is_resume_printing = false;
    is_finish_print = false;
  }
/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////  public function/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
  //是否在加热
  INT IsHeating(void)
  {
    if (!t_custom_services.disable_hot_bed)
    {
      if ((INT)sg_grbl::temperature_get_bed_target())
      {
        return 1;
      }
    }
    if ((INT)sg_grbl::temperature_get_extruder_target(0))
    {
      return 1;
    }
    return 0;
  }

  //是否正在打印
  INT IsPrint(void)
  {
    return is_printing;
  }

  INT IsPausePrint(void)
  {
    return is_pause_printing;
  }

  void SetPrintStatus(bool status)
  {
    is_printing = status;
  }

  // 设置暂停打印状态
  void SetPausePrintingStatus(bool status)
  {
    is_pause_printing = status;
  }

  void PowerOffRecStartPrint(void)
  {
    is_poweroff_recovery_print = true;
    printControl.start(false);
  }

  bool IsFinishedPrint(void)
  {
    return is_finish_print;
  }

  bool IsPrintSDFile(void)
  {
    return is_print_sd_file;
  }

  bool IsPauseToCoolDown(void)
  {
    return is_pause_to_cool_down;
  }

  //从SD卡读取gcode命令
  void readGcodeBuf(void)
  {
    printFileControl.getGcodeBuf();
  }

  bool IsProcessStopPrint(void)
  {
    return is_process_stop_print;
  }

#ifdef __cplusplus
} //extern "C"
#endif

PrintControl::PrintControl()
{
}

// 执行暂停打印操作，平台下降60mm，xy归零
void PrintControl::pauseProcess(void)
{
  if(!is_heating_status) // 未开始打印，不用执行暂停操作
  {
    for(INT i = 0; i < 3; ++i)
    {
      if(1 == t_sys_data_current.enable_bed_level)
      {
        OldPosition[i] = sg_grbl::st_get_position_length(i); //获取位置
      }
      else
      {
        OldPosition[i] = gcode::get_current_position(i); //获取位置
      }
    }
    OldPosition[E_AXIS] = gcode::get_current_position(E_AXIS); //获取位置
    OldPosition[B_AXIS] = gcode::get_current_position(B_AXIS); //获取位置
    OldFeedRate  = gcode::feed_rate; //进料速度

    z_down_60mm_and_xy_to_zero();
  }

}

UINT PrintControl::getTime(void)
{
  return (UINT)(((sys_task_get_tick_count() / 1000) - (starttime / 1000)));// + t_sys.print_time_save);
}

//停止打印
void PrintControl::stop(bool isSerial)
{
  reset_print_status();
  is_stop_printing = true;

  if(!isSerial)
  {
    printFileControl.close();
  }
  gpio_color_mix_control(false); // 关闭混色5v风扇
  SetIsUSBPrintStop(true);
}

void PrintControl::prepareStop(void)
{
  if(is_stop_printing)  // 串口停止打印或正常停止打印
  {
    if(0U == t_gui_p.G28_ENDSTOPS_COMPLETE)
    {
      Zaxis_RunOnce = false;
    }
    stepper_quick_stop(); // 电机快速停止
    if(0 == sg_grbl::planner_moves_planned())                     // 运动队列为空，执行以下操作
    {
      is_stop_printing = false;                   // 设置停止打印状态为false
      is_process_stop_print = true;             // 执行停止打印标志置为true
      return;
    }
  }
}

void PrintControl::processStop(void)
{
  static bool serial_respond = false;
  prepareStop();                                                                   // 停止打印准备操作
  sys_os_delay(100); // 延时让其他任务执行
  if(is_process_stop_print)
  {
    if ((1U == t_gui_p.G28_ENDSTOPS_COMPLETE) || !sg_grbl::temperature_get_extruder_current(0))                                                   // 是否已经归零
    {
      z_down_60mm_and_xy_to_zero();                                                        // 平台下降60mm
    }
    else
    {
      sys_send_gcode_cmd("G28 isInternal"); // XYZ归零
      sys_send_gcode_cmd("G1 F600 Z60 I0 H0 isInternal");  // Z下降60mm
    }
    m84_disable_steppers();
    gcode::resetCmdBuf();                                                                 // 重置指令数组
    t_gui.cura_speed = 0;                                                          // 读取gcode文件获取数值，停止打印则清0
    is_process_stop_print = false;                                                    // 执行停止打印标志置为false
    serial_respond = true;
  }
  if(serial_respond && (0 == sg_grbl::planner_moves_planned()))
  {
    serial_respond = false;
  }
}

//继续打印
void PrintControl::resume(bool isSerial)
{
  reset_print_status();
  is_resume_printing = true;
}

void PrintControl::pause(bool isSerial)
{
  reset_print_status();
  is_pause_printing = true;
  pause_print_status = 0;
  is_pause_to_cool_down = false;
  is_process_pause_print_done = false;
  if(t_gui_p.IsNotHaveMatInPrint && t_sys_data_current.enable_block_detect)///*20170807堵料修复*/
  {
    gcode::resetCmdBuf();
    t_gui.file_size = t_gui.printfile_size - old_sd_pos[5];
//    (void)f_lseek(&printfile, old_sd_pos[5]);
    file_pos = old_sd_pos[5];
    stepper_quick_stop(); // 电机快速停止
    file_read_buf_index = FILE_READ_SIZE;//重新从U盘读gcode文件
  }
}

//开始打印
void PrintControl::start(bool isSerial)
{
  reset_print_status();
  (void)memset(file_read_buf, 0, sizeof(file_read_buf)); //clear buffer
  t_gui_p.m305_is_force_verify = 0;
  t_gui.file_size = 0;
  t_gui.printfile_size = 0;
  file_read_buf_index = FILE_READ_SIZE;

  // 重置联机打印状态
  SetIsUSBPrintFinished(false);
  SetIsUSBPrintStop(false);
  SetIsUSBPrintPause(false);

  gpio_color_mix_control(true);  // 打开混色5v风扇

  if(printFileControl.open())
  {
    stepper_quick_stop(); // 电机快速停止
    if(!is_poweroff_recovery_print && !t_sys_data_current.IsLaser)
    {
      sys_send_gcode_cmd("G92 X0 Y0 A1 isInternal");
    }
    if (t_sys_data_current.enable_powerOff_recovery)
    {
      (void)memcpy(t_power_off.path_file_name, printfilepathname, strlen(printfilepathname));
      t_power_off.path_file_name[strlen(printfilepathname)] = '\0';
      is_poweroff_recovery_print = false; // 完成恢复操作，重置断电恢复操作标志
    }
    is_printing = true;// 发指令，设置打印状态，开始读U盘数据
    task_read_udisk_release();
    sys_os_delay(50);
  }
}

void PrintControl::pauseToCoolDown(FLOAT coolDownFactor)
{
  temp_hotend_to_resume = (INT)sg_grbl::temperature_get_extruder_target(0);  // 保持当前喷嘴温度，用于恢复打印
  temp_bed_to_resume = (INT)sg_grbl::temperature_get_bed_target();         // 保持当前热床温度，用于恢复打印

  FLOAT hotendTemp = (sg_grbl::temperature_get_extruder_target(0) * coolDownFactor); // float
  FLOAT bedTemp = (sg_grbl::temperature_get_bed_target() * coolDownFactor); // float

  if(t_gui.target_nozzle_temp != hotendTemp)//防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
  {
    sg_grbl::temperature_set_extruder_target(hotendTemp, 0);
    t_gui.target_nozzle_temp = hotendTemp;
  }

  if((!t_custom_services.disable_hot_bed) && (t_gui.target_hot_bed_temp != bedTemp ))
  {
    sg_grbl::temperature_set_bed_target(bedTemp);
    t_gui.target_hot_bed_temp = bedTemp;
  }

  is_pause_to_cool_down = true; // 断料状态下，换料要用到，即降温了则要先加温才能换料
}

void PrintControl::processPause(void)
{
  if (!is_pause_printing)
  {
    return;
  }

  if (is_pause_printing && (0 == pause_print_status) && (0 == sg_grbl::planner_moves_planned()))
  {
    pause_print_status = 1;
  }
  // 判断当前是否已经加热完成，加热完成跳到下一步
  // 未加热，设置is_heating_status为true，等待加热完成
  if (1 == pause_print_status)
  {
    if(1U == t_gui_p.m109_heating_complete) // 已加热，下一步
    {
      is_heating_status = false; // 处于加热状态
    }
    else
    {
      is_heating_status = true; // 处于加热状态
    }
    pause_print_status = 2;
  }
  else if (2 == pause_print_status)   // 判断当前运动队列是否执行完
  {
    if(is_heating_status)
    {
      is_printing = false; // 设置打印状态为false
      SetIsUSBPrintPause(true);
      pause_print_status = 3;
      return;
    }
    if(0 == sg_grbl::planner_moves_planned()) // 运动队列为空，下一步
    {
      is_printing = false; // 设置打印状态为false
      SetIsUSBPrintPause(true);
      pause_print_status = 3;
    }
  }
  else if (3 == pause_print_status) // 执行暂停打印操作
  {
    pauseProcess();
    if(sg_grbl::st_is_min_endstop(Z_AXIS))
    {
      sg_grbl::st_synchronize();
      gcode::set_current_position(Z_AXIS, motion_3d_model.xyz_max_pos[Z_AXIS]);
      gcode::plan_set_current_position();
      sg_grbl::st_synchronize();
    }
    pause_print_status = 4;
  }
  else if((4 == pause_print_status) && (sg_grbl::planner_moves_planned() == 0))
  {
    pause_print_status = 5;
    is_process_pause_print_done = true;
  }
  else if (5 == pause_print_status) // 设置暂停打印冷却超时时间
  {
    xTimeToCoolDown = sys_task_get_tick_count() + ((1000 * 60) * 3);
    pause_print_status = 6;
  }
  else if (6 == pause_print_status) // 暂停超时，执行温度下降操作
  {
    if(sys_task_get_tick_count() > xTimeToCoolDown)
    {
      pauseToCoolDown(0.5F);
      pause_print_status = 7;
    }
  }
  else if (7 == pause_print_status) // 暂停打印结束
  {
    is_pause_printing = false;
  }
}

bool PrintControl::resumeBackToPrintPos(void)
{
  static INT resume_bak_pos_status = 0;
  if(!is_heating_status)
  {
    if(0 == resume_bak_pos_status)
    {
      xy_to_zero();
      sg_grbl::st_synchronize();
      resume_bak_pos_status = 1;
      return false;
    }
    if((1 == resume_bak_pos_status) && (0 == sg_grbl::planner_moves_planned()))
    {
      eb_compensate_16mm(t_sys_data_current.enable_color_mixing);
      resume_bak_pos_status = 2;
      return false;
    }
    else if((2 == resume_bak_pos_status) && (0 == sg_grbl::planner_moves_planned()))
    {

      static CHAR cmdResume_z_pos[50] = {0};
      static CHAR cmdResume_xy_pos[50] = {0};
      static CHAR cmdResume_eb_pos[50] = {0};
      if((motion_3d_model.xyz_move_max_pos[0] < OldPosition[X_AXIS]) || (motion_3d_model.xyz_move_max_pos[1] < OldPosition[Y_AXIS]))
      {
        (void)snprintf(cmdResume_xy_pos, sizeof(cmdResume_xy_pos), "G1 F2400 X0 Y0 H0 D1 isInternal");
      }
      else
      {
        (void)snprintf(cmdResume_xy_pos, sizeof(cmdResume_xy_pos), "G1 F2400 X%0.4f Y%0.4f H0 D1 isInternal", OldPosition[X_AXIS], OldPosition[Y_AXIS]);
      }
      (void)snprintf(cmdResume_z_pos, sizeof(cmdResume_z_pos), "G1 F%d Z%0.4f I0 D1 H0 isInternal", (OldFeedRate>600?600:OldFeedRate),OldPosition[Z_AXIS]);

      sys_os_delay(50);

      if(1 == t_sys_data_current.enable_bed_level)
      {
        sys_send_gcode_cmd(cmdResume_xy_pos); //XY回到原来位置
        sys_send_gcode_cmd("M120 isInternal");//关闭限位检测
        sys_send_gcode_cmd(cmdResume_z_pos); //z回到原来位置
      }
      else
      {
        sys_send_gcode_cmd("M120 isInternal");//关闭限位检测
        sys_send_gcode_cmd(cmdResume_z_pos);//z回到原来位置
        sys_send_gcode_cmd(cmdResume_xy_pos); //XY回到原来位置
      }
      sys_os_delay(50);
      (void)snprintf(cmdResume_eb_pos, sizeof(cmdResume_eb_pos), "G92 E%0.4f B%0.4f A1 isInternal", OldPosition[E_AXIS],OldPosition[B_AXIS]);
      sys_send_gcode_cmd(cmdResume_eb_pos); //XY回到原来位置
//      g92_set_axis_position((INT)E_AXIS, OldPosition[E_AXIS]);
//      g92_set_axis_position((INT)B_AXIS, OldPosition[B_AXIS]);
      sys_os_delay(50);
      resume_bak_pos_status = 3;
      return false;
    }
    else if((3 == resume_bak_pos_status) && (0 == sg_grbl::planner_moves_planned()))
    {
      resume_bak_pos_status = 0; // 重置变量，避免第二次恢复打印异常
      return true;
    }
    return false;
  }
  else
  {
    return true;
  }
}

void PrintControl::pauseToResumeTemp(void)
{
  if(!is_pause_to_resume_temp)
  {
    if(t_gui.target_nozzle_temp != temp_hotend_to_resume)//防止二次设置,M601命令为了与android屏匹配，执行换料前先加热；20170930
    {
      sg_grbl::temperature_set_extruder_target(temp_hotend_to_resume, 0);
      t_gui.target_nozzle_temp = temp_hotend_to_resume;
    }

    if((!t_custom_services.disable_hot_bed) && (t_gui.target_hot_bed_temp != temp_bed_to_resume ))
    {
      sg_grbl::temperature_set_bed_target(temp_bed_to_resume);
      t_gui.target_hot_bed_temp = temp_bed_to_resume;
    }
    is_pause_to_resume_temp = true;
  }
  sys_os_delay(50);
}

bool PrintControl::isResumeTempDone(void)
{
  pauseToResumeTemp();

  INT degh=(INT)sg_grbl::temperature_get_extruder_current(0);
  INT deghT=(INT)sg_grbl::temperature_get_extruder_target(0);
  if(degh<(deghT-8))
  {
    return false;
  }
  if(!t_custom_services.disable_hot_bed)
  {
//    INT degb=degBed();
//    INT degbT=degTargetBed();
//      if((degh<deghT) || (degb<degbT)) return;//暂停后返回打印，热床不等待，2017/3/14
  }
  else
  {
    if (degh< deghT)
    {
      return false;
    }
  }
  return true;
}

void PrintControl::processResumeFinish(void)
{
  pause_print_status = 0;
  is_heating_status = false;
  is_resume_printing = false;
  is_pause_to_resume_temp = false;
  SetIsUSBPrintPause(false);
  is_printing = true;// 发指令，设置打印状态，开始读U盘数据
  task_read_udisk_release();
  sys_os_delay(50);//延时，避免联机打印无法恢复打印
}

void PrintControl::processResume(void)
{
  if (!is_resume_printing )
  {
    return;
  }
  switch (pause_print_status)
  {
  case 0:
    return;
  case 1:
  case 2:
  case 3:
    break;
  case 4:
  case 5:
  case 6:
    if(!resumeBackToPrintPos())
    {
      return;
    }
    break;
  case 7:
    if(!isResumeTempDone())
    {
      return;
    }
    if(!resumeBackToPrintPos())
    {
      return;
    }
    break;
  } // end switch
  processResumeFinish();
}
/////////////////////////////////////////////////////////////////////
///////////////////////PrintFileControl//////////////////////////////
/////////////////////////////////////////////////////////////////////

PrintFileControl::PrintFileControl()
{

}

void PrintFileControl::getFileName(void)
{
  if (is_poweroff_recovery_print) // 断电续打恢复打印，设置打印文件名
  {
    if (t_power_off.is_file_from_sd)   //断电续打文件在SD卡中
    {
      medium_id = SD_CARD;
    }
    (void)strcpy(printfilepathname, t_power_off.path_file_name);
  }
  else // 正常打印获取文件名
  {
    if (UDISK == medium_id) // U盘文件
    {
      if (t_gui_p.IsRootDir)
      {
        (void)snprintf(printfilepathname, sizeof(printfilepathname), "%s", CurrentPath);
      }
      else
      {
        (void)snprintf(printfilepathname, sizeof(printfilepathname), "%s/", CurrentPath);
      }
      (void)strcat(printfilepathname, SettingInfoToSYS.PrintFileName);
    }
    else if (SD_CARD == medium_id) // SD卡文件
    {
      (void)snprintf(printfilepathname, sizeof(printfilepathname), "%s", SDPath);
      (void)strcat(printfilepathname, SDFileName);
    }
  }
}

bool PrintFileControl::open(void)
{
  bool file_open_status = true;
  getFileName();  // 获取要打印文件名

  if (SD_CARD == medium_id)
  {
  }
  if (f_open(&printfile, printfilepathname, FA_READ) == FR_OK)
  {
    t_gui.printfile_size = f_size(&printfile);
    // 断电续打重设文件打印位置
    if (is_poweroff_recovery_print && (t_power_off.sd_pos != 0))
    {
      t_gui.file_size = t_gui.printfile_size - t_power_off.sd_pos;
      (void)f_lseek(&printfile, t_power_off.sd_pos);
      file_pos = t_power_off.sd_pos; // 打开文件重置文件位置
    }
    else
    {
      t_gui.file_size = t_gui.printfile_size;
      file_pos = 0; // 打开文件重置文件位置
    }
    poweroff_start_print_init(file_pos);
    //提取BMP图片
    if(strstr(printfilepathname,".sgcode"))
    {
      readBMPFile(printfilepathname);
    }
    gcode::resetCmdBuf();

    if(strstr(printfilepathname,".gsd"))
    {
      fileType = FILE_GSD;
    }

    // 重置gui打印状态
    t_gui.printed_time_sec = 0;
    t_gui.print_percent = 0;
    starttime=sys_task_get_tick_count();
    (void)f_close(&printfile);
  }
  else
  {
    file_open_status = false;
  }

  if (SD_CARD == medium_id)
  {
  }
  return file_open_status;
}

void PrintFileControl::close(void)
{
  if (SD_CARD == medium_id)
  {
    is_print_sd_file = false;
    t_power_off.is_file_from_sd = 0;
    medium_id = UDISK;
//    (void)f_close(&printfile);
    (void)f_unlink(printfilepathname); //打印完删除上传的文件
  }
  else //从U盘读取的时候，不能添加，因为USB的控制是一个单独的任务
  {
//    (void)f_close(&printfile);
  }
  if(0 != t_sys_data_current.enable_powerOff_recovery)
  {
    poweroff_reset_flag();
  }

  //删除BMP图片
  if(strstr(printfilepathname,".sgcode"))
  {
    sgcode_delete_bmp();
  }
}

void PrintFileControl::readBMPFile(CONST PCHAR fileName)
{
  UINT32 RCount;
  MaseHeader header;
  FilesMsg msg;
  FIL *file = new(FIL);
  FIL *file1 = new(FIL);
  FRESULT f_res;

  f_res = f_open(file, fileName, FA_READ);

  if(f_res != FR_OK)
  {
    USER_EchoLogStr("File error!!\t f_res = %d\r\n",f_res);
  }
  else
  {
    // 读取header
    f_read(file,&header,sizeof(MaseHeader),&RCount);

    INT gcodeIndex = 0;
    //定位gcode文件
    for(INT i = 0; i < header.uFileCount; ++i)
    {
      f_lseek(file,sizeof(MaseHeader)+(i*sizeof(FilesMsg)));
      f_read(file,&msg,sizeof(FilesMsg),&RCount);
      if(strstr(msg.szFileName, ".gcode"))
      {
        gcodeIndex = i;
        break;
      }
    }

    f_lseek(file,sizeof(MaseHeader)+(gcodeIndex*sizeof(FilesMsg)));
    f_read(file,&msg,sizeof(FilesMsg),&RCount);
    strcpy(SettingInfoToSYS.PrintFileName,printname);
    t_gui.printfile_size = msg.uFileSize;
    if (is_poweroff_recovery_print && (t_power_off.sd_pos != 0))
    {
      t_gui.file_size = t_gui.printfile_size - t_power_off.sd_pos;
//      (void)f_lseek(&printfile, t_power_off.sd_pos + msg.uFileOfs);
      file_pos = t_power_off.sd_pos + msg.uFileOfs;
    }
    else
    {
      t_gui.file_size = t_gui.printfile_size;
//      (void)f_lseek(&printfile, msg.uFileOfs);
      file_pos = msg.uFileOfs;
    }
    /*
    USER_EchoLogStr("PrintFileName=   %s\n",SettingInfoToSYS.PrintFileName);
    USER_EchoLogStr("msg.szFileName=  %s\n",msg.szFileName);
    USER_EchoLogStr("msg.uFileOfs=    %d\n",msg.uFileOfs);
    USER_EchoLogStr("msg.uFileSize=   %d\n",msg.uFileSize);
    USER_EchoLogStr("print_file_size = %d\n",t_gui.printfile_size);
    USER_EchoLogStr("file_size=       %d\n",t_gui.file_size);
    */

    f_close(file);
  }
  delete file;
  delete file1;
  file = NULL;
  file1 = NULL;
}

//从SD卡读取gcode命令
void PrintFileControl::getGcodeBuf(void)
{
  if (is_printing)
  {
    while (t_gui.file_size)
    {
      if (file_read_buf_index == FILE_READ_SIZE)
      {
//        HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
        //sys_os_delay(100);
        bool isReadSuccess = readDataToBuf();
//        HAL_NVIC_SetPriority(OTG_FS_IRQn, 6, 0);
        if (isReadSuccess)
        {
          break;    //读取文件失败
        }
      }
      else
      {
        if(FILE_GSD == fileType)
        {
          if(file_read_buf[file_read_buf_index]>100)
          {
            file_read_buf[file_read_buf_index] -=100;
          }
          else
          {
            file_read_buf[file_read_buf_index] = '\n';
          }
          if(file_read_buf[file_read_buf_index] == '?')
          {
            file_read_buf[file_read_buf_index] = '\n';
          }
        }
        t_gui.file_position =  t_gui.printfile_size - t_gui.file_size;
        t_gui.file_size--;
        if (0 == t_gui.file_size)
        {
          readFinish();
          break;
        }
        if (gcode::GetGcodeFromBuf()) // 若是混色版本 且 已经设置好机器，需要解密
        {
          break;
        }
      }
    }
  }
}

void PrintFileControl::readFinish(void)
{
  reset_print_status();
  close();                  // 读取结束，关闭文件
  sg_grbl::st_synchronize();         // 执行完环形队列指令再执行下面指令
  is_finish_print = true;   // 设置打印完成标志为true，用于gui页面跳转
  t_gui.used_total_material = (INT)(gcode::printing_material_length + gcode::get_current_position((INT)E_AXIS));
  if (1 == t_sys_data_current.enable_color_mixing)
  {
    sys_send_gcode_cmd("G92 E0 B0 isInternal");
  }
  else
  {
    sys_send_gcode_cmd("G92 E0 isInternal");
  }
  z_down_60mm_and_xy_to_zero();            // 打印完后下降60
  gpio_color_mix_control(false); // 关闭混色5v风扇
  SetIsUSBPrintFinished(true);     // USB联机已完成打印
}

bool PrintFileControl::readDataToBuf(void)
{
  UINT file_br;    /* File R/W count */
  bool result = false;
  if (t_gui.file_size)
  {
    (void)memset(file_read_buf, 0, sizeof(file_read_buf)); //clear buffer
    if (SD_CARD == medium_id)
    {
    }
    if (f_open(&printfile, printfilepathname, FA_READ) == FR_OK)
    {
      f_lseek (&printfile, file_pos);
      if (f_read(&printfile, file_read_buf, (t_gui.file_size <= FILE_READ_SIZE)?t_gui.file_size:sizeof(file_read_buf), &file_br) == FR_OK)
      {
        file_pos += (t_gui.file_size <= FILE_READ_SIZE)?t_gui.file_size:sizeof(file_read_buf);
        read_data_error_count = 0;
      }
      f_close(&printfile);
    }
    else
    {
      ++read_data_error_count;
      if(read_data_error_count == 3)
      {
        PopWarningInfo(FatfsWarning);
        result = true;
      }
    }
    if (SD_CARD == medium_id)
    {
    }
    file_read_buf_index = 0;
  }
  return result;
}

/**
 * [PrintFileControl::getPercent 获取文件打印百分比]
 * @return  [description]
 */
INT PrintFileControl::getPercent(void)
{
  if (t_gui.printfile_size)
  {
    return (INT)((t_gui.printfile_size - t_gui.file_size) / ((t_gui.printfile_size + 99) / 100));
  }
  else
  {
    return 0;
  }
}

/**
 * [PrintFileControl::setSDMedium 设置当前打印介质为SD卡，联机打印需要用到]
 */
void PrintFileControl::setSDMedium(void)
{
  medium_id = SD_CARD;     // 设置当前文件所在介质为sd卡
  is_print_sd_file = true; // 设置打印sd文件标志为true
}

PrintControl printControl;
PrintFileControl printFileControl;
