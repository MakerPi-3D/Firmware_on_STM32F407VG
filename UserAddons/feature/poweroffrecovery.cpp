#include "poweroffrecovery.h"
#include "functioncustom.h"
#include "user_debug.h"
#include "machinecustom.h"
#include "midwaychangematerial.h"
#include "controlxyz.h"
#include "controlfunction.h"
#include "planner.h"
#include "planner_running_status.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "config_model_tables.h"
#include <cstdlib>
#include <cstring>
#include "globalvariables.h"
#include "temperature.h"
#include "stepper.h"

#include "PrintControl.h"
#include "gcode.h"
#include "fatfs.h"

#include <stdint.h>
#include "autobedlevelinterface.h"
#include "user_interface.h"
#include "Alter.h"
#include "gcode_global_params.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  FORCE_INLINE  __attribute__((always_inline)) inline
// PowerOffRecovery
#define POWER_OFF_REC_FILE             "0:/PowerOffData.txt"          /*!< 断电文件 */
#define POWER_OFF_REC_FILE_BAK         "0:/PowerOffData_bak.txt"      /*!< 断电备份文件 */
#define POWER_OFF_REC_FILE_PATH        "D1:"                          /*!< 文件名(含路径) */
#define POWER_OFF_REC_BED_TARGET_TEMP  "D2:"                          /*!< 热床目标温度 */
#define POWER_OFF_REC_NOZ_TARGET_TEMP  "D3:"                          /*!< 喷嘴目标温度 */
#define POWER_OFF_REC_FAN_SPEED        "D4:"                          /*!< 风扇速度 */
#define POWER_OFF_REC_FEED_MULTIPLY    "D5:"                          /*!< 打印速度 */
#define POWER_OFF_REC_FEED_RATE        "D6:"                          /*!< 出料速度 */
#define POWER_OFF_REC_Z_POS            "D7:"                          /*!< Z轴位置 */
#define POWER_OFF_REC_E_POS            "D8:"                          /*!< E轴位置 */
#define POWER_OFF_REC_B_POS            "D9:"                          /*!< B轴位置 */
#define POWER_OFF_REC_SD_POS           "D10:"                         /*!< 文件指针位置 */
#define BLOCKDETECT_FLAG               "D11:"                         /*!< 堵料标志 */
#define POWER_OFF_REC_FLAG             "D12:"                         /*!< 标志 */
#define POWER_OFF_IS_SERIAL_FLAG       "D13:"                         /*!< 串口标志 */
#define POWER_OFF_REC_X_POS            "D14:"                         /*!< X轴位置 */
#define POWER_OFF_REC_Y_POS            "D15:"                         /*!< Y轴位置 */
#define POWER_OFF_REC_TIME             "D16:"                         /*!< 打印时间 */
#define POWER_OFF_ENABLE_COLOR_BUF     "D17:"                         /*!< 混色和单色代码通用 */

  static FIL power_off_file;
  static bool IsStartCalculateZMaxPos = false;                      /*!< 是否开启校正Z高度 */
  static bool isPowerOffRecoverPrint = false;                       /*!< 是否为断电恢复操作 */
  static bool isPowerOffRecoverPrintFinish = false;                 /*!< 是否为断电恢复操作 */
  static bool isStopGetZMax = false;                                /*!< 是否停止校准z高度标志位 */
  static bool isXYHome = false;
  static uint32_t po_sd_position_start = 0UL;           /*!< 设置打印文件开始位置 */
  static bool is_start_save_po_data = false;            /*!< 是否开始保存断电数据 */
  static bool is_po_file_open = false;                  /*!< 断电参数文件是否打开 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////poweroff data 初始化 start/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // 获取混色和单色代码通用标志位
  static void po_get_enable_color_buf(void)
  {
    char* enableColorBufInfoPtr;
    enableColorBufInfoPtr=strstr(&poweroff_data[0],POWER_OFF_ENABLE_COLOR_BUF); //D12:34556
    enableColorBufInfoPtr=enableColorBufInfoPtr+strlen(POWER_OFF_ENABLE_COLOR_BUF);
    t_sys.enable_color_buf =static_cast<uint8_t>(atol(enableColorBufInfoPtr));  //转换成长整型
  }

  // 获取打印时间
  static void po_get_print_time(void)
  {
    char* TimeInfoPtr;
    TimeInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_TIME); //D12:34556
    TimeInfoPtr=TimeInfoPtr+strlen(POWER_OFF_REC_TIME);
    t_sys.print_time_save=static_cast<uint32_t>(atol(TimeInfoPtr));  //转换成长整型
  }

  // 获取Y轴位置
  static void po_get_y_pos(void)
  {
    char* YPosInfoPtr;
    YPosInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_Y_POS); //D7:50.5
    YPosInfoPtr=YPosInfoPtr+strlen(POWER_OFF_REC_Y_POS);
    t_power_off.y_pos = static_cast<float>(atof(YPosInfoPtr));  //转换成符点值
  }

  // 获取X轴位置
  static void po_get_x_pos(void)
  {
    char* XPosInfoPtr;
    XPosInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_X_POS); //D7:50.5
    XPosInfoPtr=XPosInfoPtr+strlen(POWER_OFF_REC_X_POS);
    t_power_off.x_pos=static_cast<float>(atof(XPosInfoPtr));  //转换成符点值
  }

  // 获取串口标志位
  static void po_get_serial_flag(void)
  {
    char* SerialFlagInfoPtr;
    SerialFlagInfoPtr=strstr(&poweroff_data[0],POWER_OFF_IS_SERIAL_FLAG); //D15:1
    if(SerialFlagInfoPtr != NULL)
    {
      SerialFlagInfoPtr=SerialFlagInfoPtr+strlen(POWER_OFF_IS_SERIAL_FLAG);
      t_power_off.serial_flag = static_cast<unsigned char>(*SerialFlagInfoPtr-'0');
    }
  }

  // 获取断电标志位
  static void po_get_flag(void)
  {
    char* FlagInfoPtr;
    FlagInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_FLAG); //D13:1
    if(FlagInfoPtr != NULL)
    {
      FlagInfoPtr=FlagInfoPtr+strlen(POWER_OFF_REC_FLAG);
      t_power_off.flag = static_cast<unsigned char>(*FlagInfoPtr-'0');
    }
  }

  // 获取堵料状态
  static void po_get_block_flag(void)
  {
    char* blockflagInfoPtr;
    blockflagInfoPtr = strstr(&poweroff_data[0],BLOCKDETECT_FLAG); //D13:
    blockflagInfoPtr = blockflagInfoPtr+strlen(BLOCKDETECT_FLAG);
    t_power_off.blockdetectflag = *blockflagInfoPtr - '0';
//    USER_EchoLogStr("\r\n*blockflagInfoPtr = %c\t t_power_off.blockdetectflag = %d\r\n",*blockflagInfoPtr,t_power_off.blockdetectflag);
  }

  // 获取文件位置
  static void po_get_sd_pos(void)
  {
    char* SDPosInfoPtr;
    SDPosInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_SD_POS); //D10:34556
    SDPosInfoPtr=SDPosInfoPtr+strlen(POWER_OFF_REC_SD_POS);
    t_power_off.sd_pos=static_cast<uint32_t>(atol(SDPosInfoPtr));  //转换成长整型
  }

  // 获取B轴位置
  static void po_get_b_pos(void)
  {
    char* BPosInfoPtr;
    BPosInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_B_POS); //D9:50.5
    BPosInfoPtr=BPosInfoPtr+strlen(POWER_OFF_REC_B_POS);
    t_power_off.b_pos=static_cast<float>(atof(BPosInfoPtr));  //转换成符点值
  }

  // 获取E轴位置
  static void po_get_e_pos(void)
  {
    char* EPosInfoPtr;
    EPosInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_E_POS); //D8:50.5
    EPosInfoPtr=EPosInfoPtr+strlen(POWER_OFF_REC_E_POS);
    t_power_off.e_pos=static_cast<float>(atof(EPosInfoPtr));  //转换成符点值
  }

  // 获取Z轴位置
  static void po_get_z_pos(void)
  {
    char* ZPosInfoPtr;
    ZPosInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_Z_POS); //D7:50.5
    ZPosInfoPtr=ZPosInfoPtr+strlen(POWER_OFF_REC_Z_POS);
    t_power_off.z_pos=static_cast<float>(atof(ZPosInfoPtr));  //转换成符点值
  }

  // 进料速度
  static void po_get_feed_rate(void)
  {
    char* FeedRateInfoPtr;
    FeedRateInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_FEED_RATE); //D6:100
    FeedRateInfoPtr=FeedRateInfoPtr+strlen(POWER_OFF_REC_FEED_RATE);
    t_power_off.feed_rate=static_cast<float>(atof(FeedRateInfoPtr));  //转换成整型值
  }

  // 进料速度百分比
  static void po_get_feed_multiply(void)
  {
    char* FeedMultiplyInfoPtr;
    FeedMultiplyInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_FEED_MULTIPLY); //D5:100
    FeedMultiplyInfoPtr=FeedMultiplyInfoPtr+strlen(POWER_OFF_REC_FEED_MULTIPLY);
    t_power_off.feed_multiply=atoi(FeedMultiplyInfoPtr);  //转换成整型值
  }

  // 获取风扇速度
  static void po_get_fan_speed(void)
  {
    char* FanSpeedInfoPtr;
    FanSpeedInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_FAN_SPEED); //D4:150
    FanSpeedInfoPtr=FanSpeedInfoPtr+strlen(POWER_OFF_REC_FAN_SPEED);
    t_power_off.fan_speed=atoi(FanSpeedInfoPtr);  //转换成整型值
  }

  // 获取喷嘴目标温度
  static void po_get_noz_target_temp(void)
  {
    char* NozzleTargetTempInfoPtr;
    NozzleTargetTempInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_NOZ_TARGET_TEMP); //D3:50
    NozzleTargetTempInfoPtr=NozzleTargetTempInfoPtr+strlen(POWER_OFF_REC_NOZ_TARGET_TEMP);
    t_power_off.nozzle_target_temp=atoi(NozzleTargetTempInfoPtr);  //转换成整型值
  }

  // 获取热床目标温度
  static void po_get_bed_target_temp(void)
  {
    char* BedTargetTempInfoPtr;
    BedTargetTempInfoPtr=strstr(&poweroff_data[0],POWER_OFF_REC_BED_TARGET_TEMP); //D2:50
    BedTargetTempInfoPtr=BedTargetTempInfoPtr+strlen(POWER_OFF_REC_BED_TARGET_TEMP);
    t_power_off.bed_target_temp=atoi(BedTargetTempInfoPtr);  //转换成整型值
  }

  // 获取文件路径名
  static void po_get_path_file_name(void)
  {
    char* PathFileNameInfoHeadPtr=strstr(&poweroff_data[0],POWER_OFF_REC_FILE_PATH); //D1：0:/test.gcode
    if(PathFileNameInfoHeadPtr != NULL)
    {
      PathFileNameInfoHeadPtr=PathFileNameInfoHeadPtr+strlen(POWER_OFF_REC_FILE_PATH);
      char* PathFileNameInfoEndPtr=strstr(PathFileNameInfoHeadPtr,".gcode");
      if(NULL != strstr(PathFileNameInfoHeadPtr,".sgcode"))
      {
        PathFileNameInfoEndPtr=strstr(PathFileNameInfoHeadPtr,".sgcode");
      }
      if(PathFileNameInfoEndPtr != NULL)
      {
        UINT length=(UINT)((PathFileNameInfoEndPtr + strlen(".gcode")) - PathFileNameInfoHeadPtr);
        if(NULL != strstr(PathFileNameInfoHeadPtr,".sgcode"))
        {
          length=(UINT)((PathFileNameInfoEndPtr + strlen(".sgcode")) - PathFileNameInfoHeadPtr);
        }
        (void)strncpy(t_power_off.path_file_name,PathFileNameInfoHeadPtr,length);  //复制文件名（含路径）
        t_power_off.path_file_name[length] = '\0';
      }
    }
  }

  // 解析断电保存数据
  static void po_data_explain(void)
  {
    po_get_flag();//获取标志位
    po_get_serial_flag();
    if(0U != t_power_off.serial_flag) // 串口标志位开，断电标志关闭
    {
      t_power_off.flag = 0U;
    }
    if((0U != t_power_off.flag) || (0U != t_power_off.serial_flag))  //需要续打
    {
      po_get_path_file_name();
      po_get_bed_target_temp();
      po_get_noz_target_temp();
      po_get_fan_speed();
      po_get_feed_multiply();
      po_get_feed_rate();
      po_get_z_pos();
      po_get_e_pos();
      po_get_b_pos();
      po_get_sd_pos();
      po_get_block_flag();
      po_get_x_pos();
      po_get_y_pos();
      po_get_print_time();
      po_get_enable_color_buf();
      (void)strcpy(t_power_off.file_name, strrchr(t_power_off.path_file_name,'/')+1); //复制文件名（不含路径）
      if((UINT8)(t_power_off.path_file_name[0]-'0') == 0U)
      {
        t_power_off.is_file_from_sd = 1U;
      }
    }
  }

  // 恢复打印（或者取消打印）时，先将断电文件备份，误操作时可恢复打印
  static void power_off_save_file_bak(void)
  {
    if(f_open(&power_off_file, POWER_OFF_REC_FILE_BAK, FA_CREATE_ALWAYS | FA_WRITE) ==FR_OK) //打开保存数据的文件
    {
      (void)f_lseek(&power_off_file,0U); //重新指向0
      (void)f_write(&power_off_file, poweroff_data, sizeof(poweroff_data), NULL);  //写入空白符清空原来的信息
      (void)f_sync(&power_off_file); //保存好
      (void)f_close(&power_off_file);
    }
    else
    {
      USER_ErrLog("poweroff_save_file_bak open file failed!");
    }
  }

  static void po_read_data_buf(void)
  {
    uint32_t poweroff_data_size = 0;
    memset(&t_power_off, 0, sizeof(t_power_off));
    memset(poweroff_data, 0, sizeof(char)*POWER_OFF_BUF_SIZE); //初始化填入空白符到BUF
    if(f_open(&power_off_file, POWER_OFF_REC_FILE, FA_READ | FA_WRITE) ==FR_OK) //打开保存数据的文件
    {
      (void)f_read(&power_off_file, poweroff_data, POWER_OFF_BUF_SIZE, &poweroff_data_size);
      poweroff_data[poweroff_data_size]=0U;
      (void)f_close(&power_off_file);
    }
    else
    {
      USER_ErrLog("PowerOffFile Open Failed!");
    }

    if(poweroff_data_size > 0) // 存在断电数据，先备份数据
    {
      power_off_save_file_bak();
    }
  }

  // 初始化断电数据
  void poweroff_data_init(void)
  {
    if(0U != t_sys_data_current.enable_powerOff_recovery)
    {
      po_read_data_buf();
      po_data_explain();

      if(f_open(&power_off_file, POWER_OFF_REC_FILE, FA_READ | FA_WRITE) ==FR_OK) //打开保存数据的文件
      {
        is_po_file_open = true;
      }
      else
      {
        USER_ErrLog("%s not exist", POWER_OFF_REC_FILE );
        if(f_open(&power_off_file, POWER_OFF_REC_FILE, FA_CREATE_ALWAYS | FA_WRITE) ==FR_OK) //打开保存数据的文件
        {
          is_po_file_open = true;
        }
        else
        {
          USER_ErrLog("%s create error", POWER_OFF_REC_FILE );
        }
      }
    }
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////poweroff data 初始化 end///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // 保存断电数据
  static FORCE_INLINE void power_off_sync_data(void)
  {
    if(is_po_file_open && is_start_save_po_data)
    {
      (void)f_lseek(&power_off_file,0U); //重新指向0
      (void)f_write(&power_off_file, poweroff_data, sizeof(poweroff_data), NULL);
      (void)f_sync(&power_off_file);
    }
  }

  void poweroff_start_print_init(uint32_t sd_position_start)
  {
    po_sd_position_start = sd_position_start;
    is_start_save_po_data = false;
    for(int i = 0; i < 4; i++)
    {
      reset_run_status(&po_save_running_status[i]);
    }
    memset(&poweroff_data, 0, sizeof(poweroff_data)); // 清空断电数据
  }

  void HAL_GPIO_EXTI_Callback(UINT16 GPIO_Pin)
  {
    if((GPIO_Pin==GPIO_PIN_3) && t_sys_data_current.enable_powerOff_recovery) // PD3
    {
      if(IsPrint())
      {
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3) == GPIO_PIN_RESET)
        {
          if(is_po_file_open)
          {
            (void)f_close(&power_off_file);
            USER_EchoLog("power_off_file close");
          }
          HAL_NVIC_SystemReset(); //复位
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
      }
    }
  }

  // 断电保存数据
  void poweroff_sync_data(void)
  {
    static uint32_t sync_data_timeout = 0UL;

    // 断电恢复时，判断是否已经在打印，避免还没打印就保存数据，把正确数据覆盖
    if(!is_start_save_po_data && (runningStatus[block_buffer_tail].sd_position > po_sd_position_start))
    {
      is_start_save_po_data = true;
    }

    // 1、需要判断是否断电恢复成功
    // 2、HAL_GPIO_EXTI_Callback保存断电数据不可靠，现在每隔一秒保存断电数据
    if(sync_data_timeout < sys_task_get_tick_count())
    {
      power_off_sync_data();
      sync_data_timeout = sys_task_get_tick_count() + 1000U;
    }
  }

  // 设置断电数据缓存
  static FORCE_INLINE void power_off_sync_data_buf(void)
  {
    static uint32_t set_data_timeout = 0UL;
    copy_run_status_to_other(&po_save_running_status[2],&po_save_running_status[3]);
    copy_run_status_to_other(&po_save_running_status[1],&po_save_running_status[2]);
    copy_run_status_to_other(&po_save_running_status[0],&po_save_running_status[1]);
    copy_run_status_to_other(&runningStatus[block_buffer_tail],&po_save_running_status[0]);
    if(set_data_timeout < sys_task_get_tick_count())
    {
      unsigned char save_index = 0U;
      for(int i = 3; i >= 0; --i)
      {
        if(0 != po_save_running_status[i].sd_position)
        {
          save_index = i;
          break;
        }
      }

      memset(&poweroff_data[0], 0, sizeof(poweroff_data));
      (void)snprintf(&poweroff_data[0], sizeof(poweroff_data),\
                     "D1:%s\r\nD2:%d\r\nD3:%d\r\nD4:%d\r\nD5:%d\r\nD6:%f\r\nD7:%f\r\nD8:%f\r\nD9:%f\r\nD10:%d\r\nD11:%d\r\nD12:1\r\nD13:%d\r\nD14:%f\r\nD15:%f\r\nD16:%ld\r\nD17:%d\r\n",\
                     t_power_off.path_file_name, \
                     (int)po_save_running_status[save_index].bed_temp,\
                     (int)po_save_running_status[save_index].extruder0_temp,\
                     po_save_running_status[save_index].fan_speed,\
                     po_save_running_status[save_index].feed_multiply,\
                     po_save_running_status[save_index].feed_rate,\
                     po_save_running_status[save_index].axis_position[Z_AXIS],\
                     po_save_running_status[save_index].axis_position[E_AXIS],\
                     po_save_running_status[save_index].axis_position[B_AXIS],\
                     (int)po_save_running_status[save_index].sd_position,\
                     t_gui_p.IsNotHaveMatInPrint,\
                     (po_save_running_status[save_index].is_serial),\
                     po_save_running_status[save_index].axis_position[X_AXIS],\
                     po_save_running_status[save_index].axis_position[Y_AXIS],\
                     (long)printControl.getTime(), \
                     t_sys.enable_color_buf);//2017516记录三个sd位置
      set_data_timeout = sys_task_get_tick_count() + 10U;
    }
  }

  void poweroff_set_data(void)
  {
    static int _current_block_buffer_tail = -1;
    static bool _is_pause_print = false;
    if((0 != IsPrint())) // 打印状态
    {
      static int bed_target = 0;
      static int hotend_target = 0;
      bool is_sync_data = false;
      if((bed_target != static_cast<int>(sg_grbl::temperature_get_bed_target())) ||
          (hotend_target != static_cast<int>(sg_grbl::temperature_get_extruder_target(0)))) // 目标温度变化
      {
        is_sync_data = true;
        bed_target = static_cast<int>(sg_grbl::temperature_get_bed_target());
        hotend_target = static_cast<int>(sg_grbl::temperature_get_extruder_target(0));
      }
      if(_current_block_buffer_tail != block_buffer_tail) // 當前運動隊列變化，保存數據
      {
        is_sync_data = true;
        _current_block_buffer_tail = block_buffer_tail;
      }
      if(is_sync_data)
      {
        power_off_sync_data_buf();
      }
      _is_pause_print = false;
    }
    else if((0 != IsPausePrint()) || gcode::m600_is_midway_change_material) // 暂停打印或者中途换料，先保存断电数据
    {
      if(!_is_pause_print) // 暫停打印只保存一次
      {
        power_off_sync_data_buf();
        power_off_sync_data();
      }
      _current_block_buffer_tail = -1;
      _is_pause_print = true;
    }
    else
    {
      _current_block_buffer_tail = -1;
      _is_pause_print = false;
    }
  }

  //断电续打取消删除SD卡中的文件
  void poweroff_delete_file_from_sd(void)
  {
    if (0U != t_power_off.is_file_from_sd)   //断电续打文件在SD卡中，删除文件
    {
      (void)f_unlink(t_power_off.path_file_name);
    }
  }



  // 重置断电标志位
  void poweroff_reset_flag(void)
  {
    memset(poweroff_data, 0, sizeof(char)*POWER_OFF_BUF_SIZE); //初始化填入空白符到BUF
    memset(&t_power_off, 0, sizeof(t_power_off));
    (void)f_lseek(&power_off_file,0U); //重新指向0
    (void)f_write(&power_off_file, poweroff_data, sizeof(poweroff_data), NULL);  //写入空白符清空原来的信息
    (void)f_sync(&power_off_file); //保存好
  }


#ifdef __cplusplus
} //extern "C" {
#endif

PowerOffOperation::PowerOffOperation()
{
}

void PowerOffOperation::startCalculateZMaxPos(void)
{
  isStopGetZMax = false;
  if(!t_sys_data_current.IsMechanismLevel)
  {
    sys_send_gcode_cmd("G28 X0 Y0 Z0 O1 isInternal");// XYZ归零
    z_down_to_bottom(); // Z下降到底部
    t_gui_p.G28_ENDSTOPS_COMPLETE = 0U; // 设置为未归零
  }
  IsStartCalculateZMaxPos=true; // 设置开始校准Z高度
}

void PowerOffOperation::stopGetZMaxPos(void)
{
  isStopGetZMax = true;
}

void PowerOffOperation::getZMaxPos(void)
{
  if(isStopGetZMax)
  {
    stepper_quick_stop(); // 电机快速停止
    IsStartCalculateZMaxPos = false;
    isStopGetZMax = false;
    sys_os_delay(50);
  }
  if(0U != t_sys_data_current.enable_powerOff_recovery)
  {
//#if defined(Z_MAX_PIN) && Z_MAX_PIN_ON > -1
    if(IsStartCalculateZMaxPos) // 开始校准Z
    {
      if(t_sys_data_current.IsMechanismLevel)
      {
        t_gui_p.IsFinishedCalculateZPosLimit = 1U;
        IsStartCalculateZMaxPos = false;
      }
      else if((0 == sg_grbl::planner_moves_planned()) && (0U != sg_grbl::st_check_endstop_z_hit_max()) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE)) //已碰到限位开关  //先归零，然后向下碰限位开关
      {
        CONST FLOAT ZMaxPosValue = sg_grbl::st_get_endstops_len(static_cast<INT>(Z_AXIS));
        SaveCalculateZMaxPos(ZMaxPosValue);
        t_gui_p.IsFinishedCalculateZPosLimit = 1U;
        IsStartCalculateZMaxPos = false;
        sys_send_gcode_cmd("G28 isInternal");

        //串口上传信息到上位机2017.7.6
        USER_EchoLogStr("z_max_pos:%.2f\r\n",ZMaxPosValue);
        sys_os_delay(50);
      }
    }
//#endif
  }
}

static void recovery_print_heating(void)
{
  static CHAR gcodeM104OrM190CommandBuf[50] = {0};
  static CHAR gcodeM109CommandBuf[50] = {0};
  (void)memset(gcodeM104OrM190CommandBuf, 0, sizeof(gcodeM104OrM190CommandBuf));
  if (0U == t_custom_services.disable_hot_bed)
  {
    if (t_power_off.bed_target_temp > 50U)
    {
      sys_send_gcode_cmd("M190 S50 isInternal");
      (void)snprintf(gcodeM104OrM190CommandBuf, sizeof(gcodeM104OrM190CommandBuf), "M140 S%d isInternal", t_power_off.bed_target_temp);
    }
    else
    {
      (void)snprintf(gcodeM104OrM190CommandBuf, sizeof(gcodeM104OrM190CommandBuf), "M190 S%d isInternal", t_power_off.bed_target_temp);
    }
    sys_send_gcode_cmd(gcodeM104OrM190CommandBuf);//热床加温命令
  }

  (void)memset(gcodeM109CommandBuf, 0, sizeof(gcodeM109CommandBuf));
  (void)snprintf(gcodeM109CommandBuf, sizeof(gcodeM109CommandBuf), "M109 S%d isInternal", t_power_off.nozzle_target_temp);
  sys_send_gcode_cmd(gcodeM109CommandBuf);//喷嘴加温命令
}

static void recovery_print_set_EB(void)
{
  if(0U != t_power_off.blockdetectflag)//20170927,堵料时断电续打须进丝一段时间
  {
    sys_send_gcode_cmd("G92 E0 isInternal");
    sys_send_gcode_cmd("G1 F150 E150 H0 isInternal");
    t_power_off.blockdetectflag = 0U;
  }

  eb_compensate_16mm(t_sys_data_current.enable_color_mixing);

  g92_set_axis_position(static_cast<INT>(E_AXIS), t_power_off.e_pos);

  if (0U != t_sys_data_current.enable_color_mixing)
  {
    g92_set_axis_position(static_cast<INT>(B_AXIS), t_power_off.b_pos);
  }
}

static void recovery_print_set_Z(void)
{
  // 需要判断是否归零
  if((motion_3d_model.xyz_move_max_pos[0] < t_power_off.x_pos) || (motion_3d_model.xyz_move_max_pos[1] < t_power_off.y_pos))
  {
    sys_send_gcode_cmd("G28 isInternal");
  }
  else
  {
    static CHAR gcodeG1ZCommandBuf[50] = {0};
    (void)memset(gcodeG1ZCommandBuf, 0, sizeof(gcodeG1ZCommandBuf));
    if ((M4141 == t_sys_data_current.model_id) || (M4141S_NEW == t_sys_data_current.model_id) || (M4141S == t_sys_data_current.model_id))
    {
      (void)snprintf(gcodeG1ZCommandBuf, sizeof(gcodeG1ZCommandBuf), "G1 F400 Z%f P%u I0 H0 isInternal", t_power_off.z_pos, t_power_off.sd_pos); //添加上文件位置
    }
    else
    {
      (void)snprintf(gcodeG1ZCommandBuf, sizeof(gcodeG1ZCommandBuf), "G1 F600 Z%f P%u I0 H0 isInternal", t_power_off.z_pos, t_power_off.sd_pos); //添加上文件位置
    }
    sys_send_gcode_cmd(gcodeG1ZCommandBuf);//Z位置

    static CHAR gcodeG1XYCommandBuf[50] = {0};
    (void)memset(gcodeG1XYCommandBuf, 0, sizeof(gcodeG1XYCommandBuf));
    (void)snprintf(gcodeG1XYCommandBuf, sizeof(gcodeG1XYCommandBuf), "G1 F2400 X%f Y%f H0 isInternal", t_power_off.x_pos, t_power_off.y_pos); //添加上文件位置
    sys_send_gcode_cmd(gcodeG1XYCommandBuf);//Z位置
  }
  static CHAR gcodeG1FCommandBuf[50] = {0};
  (void)memset(gcodeG1FCommandBuf, 0, sizeof(gcodeG1FCommandBuf));
  if(t_power_off.feed_rate > 2400.0F)
  {
    t_power_off.feed_rate = 2400.0F; // 限制移动速度最大为40mm/s
  }
  (void)snprintf(gcodeG1FCommandBuf, sizeof(gcodeG1FCommandBuf), "G1 F%f H0 isInternal", t_power_off.feed_rate*60.0f*100.0f/t_power_off.feed_multiply);
  sys_send_gcode_cmd(gcodeG1FCommandBuf);//出料速度
}

static inline void recovery_print_close_to_saved_z(void)
{
  static CHAR gcodeG1CommandBuf[50] = {0};
  (void)memset(gcodeG1CommandBuf, 0, sizeof(gcodeG1CommandBuf));
  float zUpValue = t_sys_data_current.poweroff_rec_z_max_value;
  if(t_sys_data_current.poweroff_rec_z_max_value > (50.0F + t_power_off.z_pos)) // float
  {
    zUpValue = 50.0F + t_power_off.z_pos; // float
  }
  if ((M4141 == t_sys_data_current.model_id) || (M4141S_NEW == t_sys_data_current.model_id) || (M4141S == t_sys_data_current.model_id))
  {
    (void)snprintf(gcodeG1CommandBuf, sizeof(gcodeG1CommandBuf), "G1 F200 Z%.2f I0 H0 isInternal", zUpValue);
  }
  else
  {
    (void)snprintf(gcodeG1CommandBuf, sizeof(gcodeG1CommandBuf), "G1 F600 Z%.2f I0 H0 isInternal", zUpValue);
  }
  sys_send_gcode_cmd(gcodeG1CommandBuf);//设置Z最大位置
}

void PowerOffOperation::recoveryPrintLoop(void)
{
  static UINT8 powerOffRecoverPrintStatus = 0U;
  if ((powerOffRecoverPrintStatus == 0U) && (1U == t_gui_p.m109_heating_complete) && (0 == sg_grbl::planner_moves_planned()))   //加热完成和Z轴向下完成
  {
    t_gui_p.G28_ENDSTOPS_COMPLETE = 1U; //设置为归零
    motion_3d.updown_g28_first_time = 1U; // 设置已经执行了上下共限位归零操作
    z_check_and_set_bottom(motion_3d.enable_poweroff_up_down_min_min, t_sys_data_current.poweroff_rec_z_max_value); // 检测z底部位置
#if LASER_MODE
    if(!t_sys_data_current.IsLaser)
#endif
    {
      static CHAR gcodeM140CommandBuf[50] = {0};
      (void)memset(gcodeM140CommandBuf, 0, sizeof(gcodeM140CommandBuf));
      (void)snprintf(gcodeM140CommandBuf, sizeof(gcodeM140CommandBuf), "M140 S%d isInternal", t_power_off.bed_target_temp);
      sys_send_gcode_cmd(gcodeM140CommandBuf);

      static CHAR gcodeM104CommandBuf[50] = {0};
      (void)memset(gcodeM104CommandBuf, 0, sizeof(gcodeM104CommandBuf));
      (void)snprintf(gcodeM104CommandBuf, sizeof(gcodeM104CommandBuf), "M104 S%d isInternal", t_power_off.nozzle_target_temp);
      sys_send_gcode_cmd(gcodeM104CommandBuf);//设置Z最大位置
    }
    powerOffRecoverPrintStatus = 1U;
    isPowerOffRecoverPrintFinish = false;
  }
  else if ((powerOffRecoverPrintStatus == 1U) && (0 == sg_grbl::planner_moves_planned()))
  {
    recovery_print_close_to_saved_z();
    powerOffRecoverPrintStatus = 2U;
  }
  else if ((powerOffRecoverPrintStatus == 2U) && (0 == sg_grbl::planner_moves_planned()))
  {
#if LASER_MODE
    if(!t_sys_data_current.IsLaser)
#endif
    {
      t_gui_p.m109_heating_complete = 0U;  //设置为未加热
      recovery_print_heating();
    }
    powerOffRecoverPrintStatus = 3U;
  }
  else if ((powerOffRecoverPrintStatus == 3U) && (1U == t_gui_p.m109_heating_complete))     //加热完成和XY轴归零完成和Z轴向下归零完成
  {
#if LASER_MODE
    if(!t_sys_data_current.IsLaser)
#endif
      recovery_print_set_EB();
    powerOffRecoverPrintStatus = 4U;
  }
  else if ((powerOffRecoverPrintStatus == 4U) && (0 == sg_grbl::planner_moves_planned()))     //加热完成和XY轴归零完成和Z轴向下归零完成
  {
    recovery_print_set_Z();

    if(0U != t_power_off.serial_flag)
    {
      sys_send_gcode_cmd("M1005 S6 isInternal");//打开串口打印状态
    }
    powerOffRecoverPrintStatus = 5U;
  }
  else if ((powerOffRecoverPrintStatus == 5U) && (0 == sg_grbl::planner_moves_planned()))     //加热完成和XY轴归零完成和Z轴向下归零完成
  {
    gcode::set_fan_speed(t_power_off.fan_speed);         //风扇速度
    gcode::feed_multiply = t_power_off.feed_multiply; //打印速度
    powerOffRecoverPrintStatus = 0U;
    isPowerOffRecoverPrintFinish = true;
#if LASER_MODE
    if(t_sys_data_current.IsLaser)
      t_gui_p.m109_heating_complete = 0U;  //设置为未加热
#endif
  }
  else
  {
    // TODO
  }
}

void PowerOffOperation::recoveryTemp(void)
{
  static CHAR gcodeM140CommandBuf[50] = {0};
  static CHAR gcodeM104CommandBuf[50] = {0};
  (void)memset(gcodeM140CommandBuf, 0, sizeof(gcodeM140CommandBuf));
  (void)snprintf(gcodeM140CommandBuf, sizeof(gcodeM140CommandBuf), "M140 S%d isInternal", t_power_off.bed_target_temp);
  sys_send_gcode_cmd(gcodeM140CommandBuf);//设置Z最大位置

  (void)memset(gcodeM104CommandBuf, 0, sizeof(gcodeM104CommandBuf));
  (void)snprintf(gcodeM104CommandBuf, sizeof(gcodeM104CommandBuf), "M104 S%d isInternal", t_power_off.nozzle_target_temp);
  sys_send_gcode_cmd(gcodeM104CommandBuf);//设置Z最大位置
}


//打开文件，继续打印
void PowerOffOperation::recoveryPrint(void)
{
  if(0U == t_sys_data_current.enable_powerOff_recovery)
    return;
  static UINT8 isStartPowerOffRecoverPrint = 0U;


  if((0U != isPowerOffRecoverPrint) && (0U == isXYHome) && (0 == sg_grbl::planner_moves_planned()))
  {
    xy_to_zero();
    sg_grbl::st_synchronize();
    isXYHome = true;
  }
  if((0U != isPowerOffRecoverPrint) && (0 != isXYHome) && (0 == sg_grbl::planner_moves_planned())) // 等待平台降到最低处
  {
    isStartPowerOffRecoverPrint = 1U;
    isPowerOffRecoverPrint = 0;
  }

  if (0U != isStartPowerOffRecoverPrint)
  {
    if (t_power_off.sd_pos != 0U)
    {
      recoveryPrintLoop();
    }
    else
    {
      if(0U != t_power_off.serial_flag)
      {
        recoveryTemp();

        if(sg_grbl::planner_moves_planned() > 0)
        {
          return;
        }
      }
      isPowerOffRecoverPrintFinish = true;
    }
    if(0U != isPowerOffRecoverPrintFinish)
    {
      isStartPowerOffRecoverPrint = 0U;
    }
  }
  recoveryPrintFinish();
}

void PowerOffOperation::recoveryPrintFinish(void)
{
  if(isPowerOffRecoverPrintFinish)
  {
    t_gui_p.IsFinishedPowerOffRecoverReady = 1U; // UI界面更新标志位

    if(0U == t_power_off.serial_flag)
    {
      PowerOffRecStartPrint(); //开始从文件读取内容继续去打印
    }
    isPowerOffRecoverPrintFinish = false;
    t_power_off.is_power_off = 0U;
  }
}

//准备继续打印
void PowerOffOperation::readyToRecoveryPrint(void)
{
  stepper_quick_stop(); // 电机快速停止
#if LASER_MODE
  if(t_sys_data_current.IsLaser)
    t_gui_p.m109_heating_complete = 1U;
  else
#endif
    t_gui_p.m109_heating_complete = 0U; //设置为未加热

  if (t_power_off.sd_pos != 0U)
  {
    motion_3d.updown_g28_first_time = 0;
    sys_send_gcode_cmd("G90 isInternal");
    sys_send_gcode_cmd("M82 isInternal");
#if LASER_MODE
    if(!t_sys_data_current.IsLaser)
#endif
      sys_send_gcode_cmd("M109 S180 isInternal");// 先加热到180度，再移动喷嘴防止喷嘴与打印模具粘在一起。
    z_down_to_bottom(); // Z下降到底部 XY归零命令
  }
  else
  {
    motion_3d.updown_g28_first_time = 0U;
    sys_send_gcode_cmd("G90 isInternal");
    sys_send_gcode_cmd("M82 isInternal");
    sys_send_gcode_cmd("G28 isInternal");

  }
  isPowerOffRecoverPrint = 1;
  isXYHome = false;
  t_power_off.is_power_off=1U; //标志为断电状态，防止把当前sdPos写入到poweroff_data中，解决断电多次重新打印现象
}

PowerOffOperation powerOffOperation;

