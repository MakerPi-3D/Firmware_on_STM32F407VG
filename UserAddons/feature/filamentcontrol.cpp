#include "filamentcontrol.h"
#include "user_debug.h"
#include "globalvariables.h"
#include "stepper.h"
#include "temperature.h"
#include "machinecustom.h"
#include "controlfunction.h"
#include "controlxyz.h"
#include "planner.h"
#include "planner_running_status.h"
#include "config_model_tables.h"
#include "math.h"

#include "PrintControl.h"
#include "gcode.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "ConfigurationStore.h"

#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "user_interface.h"
#include "user_debug.h"
#include "guicontrol.h"
#include "view_common.h"
#include "view_commonf.h"
#ifdef __cplusplus
extern "C" {
#endif

  void TransFileReady(void)
  {
    //关闭蜂鸣器
    t_gui_p.isBeepAlarm = 0; //以防刚完成打印又传输文件造成的蜂鸣器鸣叫
    //关闭进丝或退丝，以防在进丝或退丝界面时上传文件
    if (!t_custom_services.disable_hot_bed)
    {
      sg_grbl::temperature_set_bed_target((FLOAT)0);
      t_gui.target_hot_bed_temp = 0;
    }
    filamentControl.resetStatus();
  }

  static bool is_process_load_unload_done = false;

#ifdef __cplusplus
} //extern "C" {
#endif

FilamentControl::FilamentControl()
{
  startLoadFlag = 0;
  startUnloadFlag = 0;
  timeOutFlag = 0;
  timeOutTickCount = 0;
}

bool Lowtemp_load = false;
// 准备进退丝操作
void FilamentControl::prepare(void)
{
  t_gui_p.IsSuccessFilament = 0;
  t_gui_p.IsFinishedFilamentHeat = 0;

  GUI_WNozzleTargetTemp(sg_grbl::filament_load_unload_temp);// 喷嘴预热230度

  //X5机器进丝时EB电机要先转动一点以卡住耗材
  if(K5 == t_sys_data_current.model_id && t_sys_data_current.enable_color_mixing && Lowtemp_load)
  {
    sys_send_gcode_cmd("G92 E0 B0 isInternal");
    sys_send_gcode_cmd("G1 F200 E3 B3 isInternal");
  }

  if (0U == t_gui_p.G28_ENDSTOPS_COMPLETE) // 进退丝，平台未归零，先归零
  {
    sys_send_gcode_cmd("G28 isInternal");// xyz归零
    while(0U == t_gui_p.G28_ENDSTOPS_COMPLETE) // 等待归零完成
    {
      sys_os_delay(50);
    }
  }

  if((sg_grbl::st_get_position_length(X_AXIS) > 0.0F) || (sg_grbl::st_get_position_length(Y_AXIS) > 0.0F)) // XY是否在零点
  {
    xy_to_zero();
    sg_grbl::st_synchronize();
  }

  if ( sg_grbl::st_get_position_length(Z_AXIS) < 50.0F || (t_sys_data_current.IsMechanismLevel ? (!t_gui_p.G28_ENDSTOPS_COMPLETE) : 0))                                               // 获取平台当前实际高度，小于50mm，下降到50mm位置
  {
    sys_send_gcode_cmd("G1 F600 Z50 I0 H0 isInternal"); // z下降到50mm位置
  }
}

UCHAR m83_process = 0;

// 开始进丝
void FilamentControl::startLoad(void)
{
  if(!startLoadFlag) // 避免重复触发
  {
    Lowtemp_load = true;
    prepare();
  }
  m83_process = 1;
  startLoadFlag = 1;
}

// 开始退丝
void FilamentControl::startUnload(void)
{
  if(!startUnloadFlag) // 避免重复触发
  {
    Lowtemp_load = false;
    prepare();
  }
  m83_process = 1;
  startUnloadFlag = 1;
}

// 执行进丝操作
void FilamentControl::processLoad(void)
{
  if ((INT)sg_grbl::temperature_get_extruder_current(0) >= (t_gui.target_nozzle_temp - 3))   //最后几度要等待挺长时间的，不再等待
  {
    if (!timeOutFlag)
    {
      if(K5 == t_sys_data_current.model_id)
      {
        if (1 == t_sys_data_current.enable_color_mixing)
        {
          timeOutTickCount = sys_task_get_tick_count() + (60 * 1000UL); //120s
        }
        else
        {
          timeOutTickCount = sys_task_get_tick_count() + (90 * 1000UL); //120s
        }
      }
      else
      {
        timeOutTickCount = sys_task_get_tick_count() + (120 * 1000UL); //120s
      }
      timeOutFlag = 1;
    }
  }
  if (timeOutFlag && (!is_process_load_unload_done))
  {
    if ((INT)sg_grbl::temperature_get_extruder_current(0) >= motion_3d.extrude_min_temp)
    {
      if (sg_grbl::planner_moves_planned() > 1)   //只有1个有效block的时候，继续发送进丝命令
      {
        return;
      }
      else
      {
        if(m83_process)
        {
          sys_send_gcode_cmd("M83 isInternal"); // 设置喷嘴E、B为绝对模式
          m83_process = 0;
        }

        if(M14S == t_sys_data_current.model_id)
        {
          sys_send_gcode_cmd("G1 F80 E5 B5 isInternal");
        }
        else
        {
          if (1 == t_sys_data_current.enable_color_mixing)
          {
            if(K5 == t_sys_data_current.model_id)
            {
              sys_send_gcode_cmd("G1 F300 E20 B20 isInternal");//2017.4.24更改为140，增大挤出头拉力
//              sys_send_gcode_cmd("G1 F300 E3 B3 isInternal");//2017.4.24更改为140，增大挤出头拉力
//              sys_send_gcode_cmd("G1 F400 E2 B2 isInternal");//2017.4.24更改为140，增大挤出头拉力
//              sys_send_gcode_cmd("G1 F300 E3 B3 isInternal");//2017.4.24更改为140，增大挤出头拉力
//              sys_send_gcode_cmd("G1 F400 E2 B2 isInternal");//2017.4.24更改为140，增大挤出头拉力
            }
            else
            {
              sys_send_gcode_cmd("G1 F140 E15 B15 isInternal");//2017.4.24更改为140，增大挤出头拉力
            }
          }
          else
          {
            sys_send_gcode_cmd("G1 F140 E10 B10 isInternal");//2017.4.24更改为140，增大挤出头拉力
          }
        }
      }
    }
  }
}


// 执行退丝操作
void FilamentControl::processUnload(void)
{
  static INT filament_mm_count = 0;
  timeOutTickCount = sys_task_get_tick_count() + (60 * 1000UL); //60s
  //if((INT)degHotend(0)>=(FilamentTemp-3))  //最后几度要等待挺长时间的，不再等待
  if ((int16_t)sg_grbl::temperature_get_extruder_current(0) >= (t_gui.target_nozzle_temp - 3))   //最后几度要等待挺长时间的，不再等待
  {
    if (!timeOutFlag)
    {
      timeOutFlag = 1;
      if(m83_process)
      {
        sys_send_gcode_cmd("M83 isInternal"); // 设置喷嘴E、B为绝对模式
        m83_process = 0;
      }

      if(M14S == t_sys_data_current.model_id)
      {
        sys_send_gcode_cmd("G92 E0 B0 isInternal");
        sys_send_gcode_cmd("G1 F80 E15 B15 isInternal");
        sys_send_gcode_cmd("G92 E0 B0 isInternal");
        sys_send_gcode_cmd("G1 F80 E-80 B-80 isInternal");
      }
      else
      {
        sys_send_gcode_cmd("G92 E0 B0 isInternal");
        if((K5 == t_sys_data_current.model_id) && t_sys_data_current.enable_color_mixing)
        {
          sys_send_gcode_cmd("G1 F300 E50 B50 isInternal");//2017.4.24更改为140，增大挤出头拉力
//          sys_send_gcode_cmd("G1 F300 E3 B3 isInternal");
//          sys_send_gcode_cmd("G1 F400 E2 B2 isInternal");
//          sys_send_gcode_cmd("G1 F200 E5 B5 isInternal");//2017.4.24更改为140，增大挤出头拉力
//          sys_send_gcode_cmd("G1 F300 E3 B3 isInternal");
//          sys_send_gcode_cmd("G1 F400 E2 B2 isInternal");
//          sys_send_gcode_cmd("G1 F200 E5 B5 isInternal");//2017.4.24更改为140，增大挤出头拉力
//          sys_send_gcode_cmd("G1 F400 E4 B4 isInternal");
        }
        else
        {
          sys_send_gcode_cmd("G1 F200 E50 B50 isInternal");
        }
        // sys_send_gcode_cmd("G92 E0");
        // sys_send_gcode_cmd("G1 F500 E-80");
        if (1 == t_sys_data_current.enable_color_mixing)
        {
          if(K5 == t_sys_data_current.model_id)
          {
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
            sys_send_gcode_cmd("G1 F1800 E-40 B-30 isInternal");//2017425退丝刚开始，快速退丝；B电机离喷嘴近点测试得退5mm以内最合适，E电机离喷嘴较远，退多一点
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
            sys_send_gcode_cmd("G1 F800 E-10 B-10 isInternal");
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
          }
          else
          {
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
            sys_send_gcode_cmd("G1 F8400 E-15 B-5 isInternal");//2017425退丝刚开始，快速退丝；B电机离喷嘴近点测试得退5mm以内最合适，E电机离喷嘴较远，退多一点
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
            sys_send_gcode_cmd("G1 F2500 E-65 B-70 isInternal");
          }
        }else {
          if(K5 == t_sys_data_current.model_id)
          {
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
            sys_send_gcode_cmd("G1 F1800 E-40 B-30 isInternal");//2017425退丝刚开始，快速退丝；B电机离喷嘴近点测试得退5mm以内最合适，E电机离喷嘴较远，退多一点
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
            sys_send_gcode_cmd("G1 F800 E-10 B-10 isInternal");
            sys_send_gcode_cmd("G92 E0 B0 isInternal");
          }
        }
      }
      filament_mm_count=80;
    }
  }
  if (timeOutFlag && (!is_process_load_unload_done))
  {
    //if((INT)degHotend(0)>=(FilamentTemp-30))
    if ((INT)sg_grbl::temperature_get_extruder_current(0) >= motion_3d.extrude_min_temp/*(t_gui.target_nozzle_temp - 30)*/)
    {
      if (sg_grbl::planner_moves_planned() > 1)
      {
        return;
      }
      else     //只有1个有效block的时候，继续发送退丝命令
      {
        if(M14S == t_sys_data_current.model_id)
        {
          sys_send_gcode_cmd("G1 F80 E-5 B-5 isInternal");
        }
//        else if(t_sys_data_current.enable_color_mixing && K5 == t_sys_data_current.model_id)
//					sys_send_gcode_cmd("G1 F1200 E-10 B-10 isInternal");
        else
        {
          sys_send_gcode_cmd("G1 F300 E-10 B-10 isInternal");
        }
        filament_mm_count+= 10;
        if(filament_mm_count>400)
        {
          timeOutTickCount = 0;
        }
      }
    }
  }
}


// 执行进退丝入口
void FilamentControl::process(void)
{
  if ((!startLoadFlag) && (!startUnloadFlag)) // 进退丝标志位都为false，退出
  {
    return;
  }

  if (startLoadFlag)
  {
    processLoad();
  }
  if (startUnloadFlag)
  {
    processUnload();
  }

  //进丝成功或退丝成功
  if (timeOutFlag)
  {
    // 判断进丝或退丝加热是否完成
    t_gui_p.IsFinishedFilamentHeat = 1;
    if (sys_task_get_tick_count() > timeOutTickCount)
    {
      is_process_load_unload_done = true;
    }

    if(is_process_load_unload_done && (sg_grbl::planner_moves_planned() == 0))
    {
      // 退出进退丝操作
      exit(false);
      // 判断进丝或退丝是否完成
      t_gui_p.IsSuccessFilament = 1;
    }
  }
  sys_os_delay(50);
}

// 重置进退丝状态
void FilamentControl::resetStatus(void)
{
  // 设置目标温度为0
  sg_grbl::temperature_set_extruder_target((FLOAT)0, 0);
  t_gui.target_nozzle_temp = 0;

  // 重置进退丝状态变量
  startLoadFlag = 0;
  startUnloadFlag = 0;
  timeOutFlag = 0;
  timeOutTickCount = 0;

  is_process_load_unload_done = false;
}

void FilamentControl::exit(bool isCancel)
{
  // 退出进丝或退丝操作
  sys_send_gcode_cmd("G1 F7200 isInternal"); // 设置速度为7200mm/min
  sys_send_gcode_cmd("M82 isInternal");// 关闭绝对模式
  sys_send_gcode_cmd("G92 E0 B0 isInternal"); // 重置E、B坐标值为0
  // 重置进退丝状态
  resetStatus();
}

void FilamentControl::cancelProcess(void)
{
  stepper_quick_stop(); // 电机快速停止
  exit(true);
}



/////////////////////////////////////////////////////////////////////////////////////
///////////////////////// BLOCK_DETECT    start          ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////
///////////////////////// BLOCK_DETECT private variables ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//堵料检测


#define BLEN_B 1    // binary length B
#define BLEN_A 0    // binary length A

#define EN_B (1<<BLEN_B)   // encoder B
#define EN_A (1<<BLEN_A)   // encoder A

#define TTC_ENCODER_PULSES_PER_STEP   1
////////////////////////
// Setup Rotary Encoder Bit Values (for two pin encoders to indicate movement)
// These values are independent of which pins are used for EN_A and EN_B indications
// The rotary encoder part is also independent to the chipset used for the LCD
#if defined(EN_A) && defined(EN_B)
#define encrot0 0
#define encrot1 2
#define encrot2 3
#define encrot3 1
#endif

#define BLOCK_DETECT
#ifdef BLOCK_DETECT
#define __ROUND_LEN  61.26 //69.08                                            //堵料模块直径19.5   /*!< 堵料模块直径22，滚轮周长69 mm */
#define __PULSE_PER_ROUND 48 //36                                                  //一圈发36个脉冲
#define __BLOCK_DETECT_LEN_MM 6//4                                              /*!< 检测周期长度:10mm */
#define __STEPS_PER_MM_E_AXIS 96.0

#define __TTC_MM_PER_PULSE 1.28 //1.91                                         /*!< 滚轮前进脉冲1个脉冲周长滚动1.91mm */
#define BLOCK_PERIOD_PLUSE_PREDICT (__BLOCK_DETECT_LEN_MM/__TTC_MM_PER_PULSE/4) /*!< 检测周期对应的前进脉冲 10mm : 5.23个,除以4表示正常运行一次是4个脉冲 */
#define BLOCK_PERIOD_STEPS (__BLOCK_DETECT_LEN_MM*__STEPS_PER_MM_E_AXIS)      /*!< 检测周期对应的E电机脉冲数 */
#endif // BLOCK_DETECT

#ifdef BLOCK_DETECT

//static ULONG detect_ms = 0;
  bool fchk_block = false;
  bool block_check_ready = false;
  static UINT8 alert_cnt=0;

  static void block_alert()
  {
    static UINT8 alert_times = 2;//更改检测次数，由于断电返回后需要检测多一次

    if(t_power_off.is_power_off)
    {
      alert_times = 3;//检测到三次运行不正常，对堵料标志位置true

    }
    else
    {
      alert_times = 2;//检测到二次运行不正常，对堵料标志位置true
    }

    if(alert_cnt >= (alert_times-1))
    {
      fchk_block = true;
      alert_cnt=0;
      return;
    }
    ++alert_cnt;
  }

//static void block_clean()
//{
//  fchk_block = false;
//}
  extern LONG count_postion_e_axis_last;
  static void block_watch_new()
  {
    if (block_check_ready)
    {

      if ((LONG)blockDetect.encoderPosition_save < (LONG)(BLOCK_PERIOD_PLUSE_PREDICT))
      {
        block_alert();
      }
      else
      {
        alert_cnt=0;
      }
      block_check_ready = 0;
      blockDetect.encoderPosition_save = 0;
    }
  }

  void blockDetect_process()
  {
    if(t_power_off.is_power_off)//如果是断电状态则返回
    {
      return;
    }
    // put your main code here, to run repeatedly:
    if(IsPrint())
    {
      block_watch_new();
    }
  }

  /*20170803*/

  volatile FLOAT blockdetect_OldEposition;
#define COUNT_POS 6
  static FLOAT OldEposition[COUNT_POS];
  UINT32 old_sd_pos[COUNT_POS];
  /*20170807堵料修复*/
//20170928堵料时可以断电续打

  void BlockDetectControl(void)
  {
#ifdef BLOCK_DETECT
//  static UINT8 errCnt = 0;
    static INT _current_block_buffer_tail = -1;
    volatile planner_running_status_t * const runningStatus_buf = &runningStatus[block_buffer_tail];

    if(IsPrint() && (1U == t_gui_p.m109_heating_complete) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE))
    {
      if(_current_block_buffer_tail != block_buffer_tail)
      {
        _current_block_buffer_tail = block_buffer_tail;
        for(INT i=COUNT_POS-1; i>0; i--)
        {
          OldEposition[i] = OldEposition [i-1];
          old_sd_pos[i] = old_sd_pos[i-1];
        }
        OldEposition[0] = runningStatus_buf->axis_position[E_AXIS];
        old_sd_pos[0] = runningStatus_buf->sd_position;
      }
      if(fchk_block)
      {
        fchk_block = false;
        //检测到断料两次后，启动断料处理
//      if (errCnt >=1)
//      {
        //串口上传信息到上位机2017.7.6
        USER_EchoLogStr("IsNotHaveMatInPrint\r\n");
        t_gui_p.IsNotHaveMatInPrint=1; //在打印的时候没料了
        blockdetect_OldEposition = OldEposition[COUNT_POS-1];
        //匹配断电续打保存的数据，解决堵料时断电承接不上的问题；20170928
        runningStatus_buf->sd_position = old_sd_pos[5];
        runningStatus_buf->axis_position[E_AXIS] = blockdetect_OldEposition;
        sys_send_gcode_cmd("G1");//断电数据需要记3下sdpos，因为暂停打印中有发2次gcode命令，所以还差一次在这补足20170928
        printControl.pause(false); //暂停打印
//        errCnt = 0;
//      }
//      else
//      {
//        errCnt += 1;
////        errCntTotal += 1;
//        return;
//      }
      }
    }
#endif
  }

  extern bool block_check_ready;
  LONG count_postion_e_axis_last=0;  //上一个记录周期的出丝量
  void block_detect_change_status(void)
  {
#ifdef BLOCK_DETECT
    if(t_sys_data_current.enable_block_detect)
    {
      if(block_check_ready)
      {
        return;
      }

      if( (LONG)(sg_grbl::st_get_position_steps(E_AXIS) - count_postion_e_axis_last) > (LONG)BLOCK_PERIOD_STEPS )
      {
        count_postion_e_axis_last=sg_grbl::st_get_position_steps(E_AXIS);
        block_check_ready=true;
        blockDetect.available();
      }
    }
#endif
  }

#endif //#ifdef BLOCK_DETECT  

#ifdef __cplusplus
} // extern "C" {
#endif

BlockDetect::BlockDetect()
{
  encoderDiff=0; /* encoderDiff is updated from interrupt context and added to encoderPosition every LCD update */
  encoderPosition=0;
  lastEncoderBits=0;
  buttons=0;
  encoderDiffSub = 0;
  encoder_update_lock = 0;
  ttc_update_lock = 0;
}

void BlockDetect::init()
{
  if(t_sys_data_current.enable_block_detect)
  {
    gpio_block_detect_init();
  }
}

void BlockDetect::clean()
{
  encoderDiff=0;
  encoderPosition=0;
  lastEncoderBits=0;
  buttons=0;
  encoderDiffSub = 0;
  encoder_update_lock = 0;
  ttc_update_lock = 0;
}

void BlockDetect::ttc_update()
{
  if(encoder_update_lock)
  {
    return;

  }
  if(ttc_update_lock)
  {
    return;
  }
  ttc_update_lock = 1;

  INT count = 2;
  while(count--);

  buttons = 0;

  if (gpio_block_detect_is_a_detection())
  {
    buttons |= EN_A;
  }
  if (gpio_block_detect_is_b_detection())
  {
    buttons |= EN_B;
  }
  //manage encoder rotation
  UINT8 enc = 0;
  if (buttons & EN_A)
  {
    enc |= (1 << 0);
  }
  if (buttons & EN_B)
  {
    enc |= (1 << 1);
  }
//#define DEBUG_blockdetect//调试堵料编码
#ifdef DEBUG_blockdetect
  static UINT nn=0;
#endif
  if (enc != lastEncoderBits)
  {
#ifdef DEBUG_blockdetect
    ++nn;
    printf("%denc=%d\r\n",nn,enc);
    if(nn>48) nn=0;
#endif
    // 0 2 3 1 ++
    // 0 1 3 2 --
    switch (enc)
    {
    case encrot0:
      if (lastEncoderBits == encrot3)
      {
        ++encoderDiff;
      }
      else if (lastEncoderBits == encrot1)
      {
        ++encoderDiffSub;
      }
      break;
    case encrot1:
      if (lastEncoderBits == encrot0)
      {
        ++encoderDiff;
      }
      else if (lastEncoderBits == encrot2)
      {
        ++encoderDiffSub;
      }
      break;
    case encrot2:
      if (lastEncoderBits == encrot1)
      {
        ++encoderDiff;
      }
      else if (lastEncoderBits == encrot3)
      {
        ++encoderDiffSub;
      }
      break;
    case encrot3:
      if (lastEncoderBits == encrot2)
      {
        ++encoderDiff;
      }
      else if (lastEncoderBits == encrot0)
      {
        ++encoderDiffSub;
      }
      break;
    }
  }
  lastEncoderBits = enc;
  ttc_update_lock = 0;
}

void BlockDetect::available()
{
  encoder_update_lock = 1;

  if(std::abs((FLOAT)(encoderDiff-encoderDiffSub)) < 2)
  {
    encoderPosition = 0;
    encoderPosition_save = 0;
    encoder_update_lock = 0;
    return;
  }
  // 进料时检测，encoderDiffSub小于3时，判断为干扰点，过滤掉
  if((encoderDiffSub < 4) && (encoderDiffSub < encoderDiff))
  {
    encoderDiff += encoderDiffSub;
  }
  if((encoderDiffSub > encoderDiff) && (encoderDiff < 4))
  {
    encoderDiff -= encoderDiffSub;
  }
  if (std::abs((FLOAT)encoderDiff) >= TTC_ENCODER_PULSES_PER_STEP)   //如果旋钮的旋转有效
  {
    encoderPosition += encoderDiff / TTC_ENCODER_PULSES_PER_STEP; //旋钮计数

    encoderPosition_save = encoderPosition;
    encoderPosition = 0;
    encoderDiff = 0;
    encoderDiffSub = 0;
  }
  encoder_update_lock = 0;
}

FilamentControl filamentControl;
BlockDetect blockDetect;









