// infrared_z_zero_pos_adjust
#include "infrared_bed_level_adjust.h"
#include "view_common.h"
#include "interface.h"
#include "globalvariables.h"
#include "infrared_z_zero_adjust.h"
#include "globalvariables.h"
#include "config_model_tables.h"
#include "user_interface.h"
#include "stepper.h"
#include "planner.h"
#include "controlfunction.h"
#include "sysconfig_data.h"
#include <math.h>
#include "mechanical_bed_level_adjust.h"
#include "view_common.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef CAL_Z_ZERO_OFFSET

#define Timeoutms 200
#define FirstTimeoutms 600

  static UINT8 KeyValue=0;
  static UINT8 SlowUpdateData=0;
  static UINT8 FastUpdateData=0;

  static ULONG Timeout=0;
  static ULONG FirstTimeout=0;

  static UINT16 SetZOffsetZeroValueSharpt[24*12*5];     //热床温度显示区域数组，24高12宽3个
//  extern INT planner_moves_planned(void);
  static INT cal_zero_status = 0;
  extern void cal_z_zero_interface(void);

//  /***************************红外传感器与M3145S有用到这些2017.10.18*********************/
//  static FLOAT  left_bed_position = 20.0F; //喷嘴碰平台左边
//  static FLOAT  right_bed_position = 260.0F; //喷嘴碰平台右边
//  static FLOAT  front_bed_position = 1.0F; //喷嘴碰平台前边
//  static FLOAT  back_bed_position = 210.0F;  //喷嘴碰平台后边

  static void LongPress(void)
  {
    if(IstouchxyUp())
    {
      KeyValue=0;
      (void)sys_os_delay(200);
    }
    else if(Timeout<sys_task_get_tick_count())
    {
      Timeout=Timeoutms+sys_task_get_tick_count();
      SlowUpdateData=0;
      FastUpdateData=1;
    }
  }

  static void ShortPress(void)
  {
    SlowUpdateData=1;
    FastUpdateData=0;
  }

  static UINT8 set_z_offset_scan_key(void)
  {
    if(IstouchxyDown(288,118,352,186))
    {
      return 2;
    }
    else if(IstouchxyDown(382,118,446,186))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

  static void set_z_offset_value(void)
  {
    if(0==KeyValue)
    {
      Timeout=Timeoutms+sys_task_get_tick_count();
      FirstTimeout=FirstTimeoutms+sys_task_get_tick_count();
      KeyValue=set_z_offset_scan_key();
    }
    else
    {
      if(FirstTimeout<sys_task_get_tick_count())
      {
        LongPress();
      }
      else if(IstouchxyUp())
      {
        ShortPress();
      }
    }
    if((1==FastUpdateData)||(1==SlowUpdateData))
    {

      if(1==FastUpdateData)
      {
        if(1 == KeyValue)
        {
          t_sys_data_current.z_offset_value-=1.0F; // float
        }
        else if(2 == KeyValue)
        {
          t_sys_data_current.z_offset_value+=1.0F; // float
        }
      }
      else
      {
        if(1 == KeyValue)
        {
          t_sys_data_current.z_offset_value-=0.1F; // float
        }
        else if(2 == KeyValue)
        {
          t_sys_data_current.z_offset_value+=0.1F; // float
        }
        KeyValue=0;
      }
      FastUpdateData=0;
      SlowUpdateData=0;
      if(t_sys_data_current.z_offset_value>29.9F)
      {
        t_sys_data_current.z_offset_value=29.9F;
      }
      if(t_sys_data_current.z_offset_value<-29.9F)
      {
        t_sys_data_current.z_offset_value=-29.9F;
      }
      CHAR Textbuffer[20];

      snprintf(Textbuffer, sizeof(Textbuffer), "%2.1f",t_sys_data_current.z_offset_value);
      CopyTextDisplayRangeInfo(ZOffsetZeroTextSharp,SetZOffsetZeroValueSharpt, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)Textbuffer, ZOffsetZeroTextSharp,TextRangeBuf);
    }
  }

// 设置Z距离零件偏移量接口
  void set_z_offset_to_zero(void)
  {
    if(gui_is_refresh())
    {
      display_picture(105);
      SetTextDisplayRange(232,138,12*5,24,&ZOffsetZeroTextSharp);
      ReadTextDisplayRangeInfo(ZOffsetZeroTextSharp,SetZOffsetZeroValueSharpt);

      CHAR Textbuffer[20];
      snprintf(Textbuffer, sizeof(Textbuffer), "%2.1f",t_sys_data_current.z_offset_value);
      CopyTextDisplayRangeInfo(ZOffsetZeroTextSharp,SetZOffsetZeroValueSharpt, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)Textbuffer, ZOffsetZeroTextSharp,TextRangeBuf);
      return;
    }
    if(touchxy(138,250,224,292)) //确认键
    {
      static CHAR gcodeG1CommandBuf[50] = {0};
      memset(gcodeG1CommandBuf, 0, sizeof(gcodeG1CommandBuf));
      (void)snprintf(gcodeG1CommandBuf, sizeof(gcodeG1CommandBuf), "G1 Z%f I0 J0 H0 isInternal", t_sys_data_current.z_offset_value);
      sys_send_gcode_cmd(gcodeG1CommandBuf);
      gui_set_curr_display(cal_z_zero_interface);
      return ;
    }
    if(touchxy(270,250,356,292)) //取消键
    {
      gui_set_curr_display(cal_z_zero_interface);
      return ;
    }
    set_z_offset_value();
  }

  void DGUS_bed_level_first1(void);

// 校准z零点入口
  void cal_z_zero_interface(void)
  {
    if(gui_is_refresh())
    {
      display_picture(104);
      static CHAR gcodeG92CommandBuf[50] = {0};
      memset(gcodeG92CommandBuf, 0, sizeof(gcodeG92CommandBuf));
      (void)snprintf(gcodeG92CommandBuf, sizeof(gcodeG92CommandBuf), "G92 Z%f A1 isInternal", t_sys_data_current.z_offset_value);
      sys_send_gcode_cmd(gcodeG92CommandBuf);
    }

    // 确定
    if(touchxy(96,224,198,272))
    {
      gui_set_curr_display(set_z_offset_to_zero);
      return ;
    }

    // 取消
    if(touchxy(288,224,390,272))
    {
      // 退出，设置当前z零点
      sys_send_gcode_cmd("G92 Z0 A1 isInternal");
      gui_set_curr_display(maindisplayF);
      respond_gui_send_sem(CancelCalZZero);
      return ;
    }
  }

  void cal_z_zero_interface_auto_home(void)
  {
    if(gui_is_refresh())
    {
      display_picture(83);
    }

    // 归零完成，调整到校正z零点高度页面
    if((2 == cal_zero_status) && (0 == sg_grbl::planner_moves_planned()) && (1U == t_gui_p.G28_ENDSTOPS_COMPLETE))
    {
      gui_set_curr_display(cal_z_zero_interface);
    }
  }

  void infrared_z_zero_adjust_autohome(void)
  {
    if(0U == t_gui_p.G28_ENDSTOPS_COMPLETE)
    {
      sys_send_gcode_cmd("M121 isInternal"); // 打开限位检测
      sys_send_gcode_cmd("G1 Z+20 I0 H0 isInternal");// 平台向下移动20mm
      sys_send_gcode_cmd("M120 isInternal"); // 关闭限位检查


      if(!t_sys_data_current.IsMechanismLevel)
        sys_send_gcode_cmd("G28 C1 O1 isInternal");// 归零
      else
        sys_send_gcode_cmd("G28 isInternal");// 归零
    }

    if(((motion_3d_model.xyz_home_dir[0] == 1) || (motion_3d_model.xyz_home_dir[1] == 1))&&(!t_sys_data_current.IsMechanismLevel))
    {
      sys_send_gcode_cmd("G1 F600 Z10 isInternal");
      sys_send_gcode_cmd("G1 F2400 X0 Y0 isInternal");
      sys_send_gcode_cmd("G1 F600 Z0 isInternal");
    }
    else
      sys_send_gcode_cmd("G1 F2400 X120 Y100 isInternal");

    // Z零点补偿
    if(abs(t_sys_data_current.z_offset_value) > 0.0F)
    {
      static CHAR gcodeG1CommandBuf[50] = {0};
      memset(gcodeG1CommandBuf, 0, sizeof(gcodeG1CommandBuf));
      (void)snprintf(gcodeG1CommandBuf, sizeof(gcodeG1CommandBuf), "G1 F%d Z%f I0 J0 H0 isInternal", (INT)gcode::homing_feedrate[Z_AXIS], t_sys_data_current.z_offset_value);
      sys_send_gcode_cmd(gcodeG1CommandBuf);
    }

    if(!t_sys_data_current.IsMechanismLevel)
      sys_send_gcode_cmd("G92 Z0 isInternal");
  }

  void infrared_z_zero_adjust_interface(void)
  {
    if(isCalZero)
    {
      if(0 == cal_zero_status)
      {
        gui_set_curr_display(cal_z_zero_interface_auto_home);
        cal_zero_status = 1;
      }
      else if(1 == cal_zero_status)
      {
        t_sys_data_current.z_offset_value = 0.0F;
        infrared_z_zero_adjust_autohome();
        cal_zero_status = 2;
      }
    }
    else
    {
      cal_zero_status = 0;
    }
  }

  void infrared_adjust_init(void)
  {
//    if(M2030HY == t_sys_data_current.model_id || M2030 == t_sys_data_current.model_id)
//    {
//      left_bed_position = 24.0F;
//      right_bed_position = 104.0F;//
//      front_bed_position = 0.0F;//
//      back_bed_position = 150.0F;
////    middle_bed_position = 104.0F;
//    }
//    else if(M3145S == t_sys_data_current.model_id)
//    {
//      left_bed_position = 20.0F;
//      right_bed_position = 260.0F;
//      front_bed_position = 1.0F;
//      back_bed_position = 210.0F;
////    middle_bed_position = 150.0F;
//    }
    if(!t_sys_data_current.IsMechanismLevel)
      infrared_bed_level_adjust_init();
    else
      mechanical_bed_level_adjust_init();
    // M3145K只有校准平台时，开启红外限位
    if(2 == t_sys_data_current.enable_bed_level)
    {
      if(1 == t_sys.is_bed_level_down_to_zero)
      {
        motion_3d.is_open_infrared_z_min_check = false;
      }
      else if(2 == t_sys.is_bed_level_down_to_zero)
      {
        motion_3d.is_open_infrared_z_min_check = true;
      }
    }
//    t_sys_data_current.z_offset_value = -8.8F;
  }

//  /*********************************************************
//   *红外传感器调平台辅助系统
//   *20170814
//   *********************************************************
//  */
//#include "user_debug.h"
//#include "view_common.h"
////#include "marlin.h"
//#include "planner.h"
//#include "stepper.h"
//#include "temperature.h"
//#include "view_common.h"

//  bool DGUS_OK_KEY = false;
//#define BedLevelDownValue 20 //调平时平台下降的高度

//  static FLOAT firstZposition = 0;
////static FLOAT currentZposition = 0;
////static menufunc_t lastdisplay = maindisplayF;

//  static  FLOAT DGUS_target[MAX_NUM_AXIS]= {0,0,0,0,0};
//  static	FLOAT DGUS_feedrate = 30*60;//homing_feedrate[Z_AXIS];
//  static uint32_t PointID;//检测点的id，即当前点是第几个点

//  void DGUS_bed_level_second1(void);
//  void DGUS_bed_level_third1(void);
//  void DGUS_bed_level_fourth1(void);
//  void GetThreePoint(void);
//  void DGUS_bed_level_fifth1(void);
//  void DGUS_bed_level_sixth1(void);

//  bool DGUS_isCalBedPlatform = true;

//  uint8_t firstRedPin[3] = {0};
//  FLOAT zFirstHigh = 0.0F;

//  FLOAT zHigh[3] = {0.0F};

//  static CONST FLOAT INFRARED_Z_DOWN_HIGH = 20.0F;

//  static inline void getPosHigh(CONST FLOAT &x, CONST FLOAT &y, FLOAT &z)
//  {
//    // 平台下降到INFRARED_Z_DOWN_HIGH
//    motion_3d.is_open_infrared_z_min_check = false; // 关闭红外调平检测
//    st_enable_endstops(true); // 打开限位，避免当前位置错误，撞击下限位
//    DGUS_target[Z_AXIS] = INFRARED_Z_DOWN_HIGH;
//    process_buffer_line_normal(DGUS_target,DGUS_feedrate/60);

//    // 喷嘴移动到(x,y)
//    DGUS_target[X_AXIS] = x;
//    DGUS_target[Y_AXIS] = y;
//    process_buffer_line_normal(DGUS_target,DGUS_feedrate/60);

//    // 平台上升到-INFRARED_Z_DOWN_HIGH
//    st_check_endstop_z_hit_min(); // 清空限位缓存
//    motion_3d.is_open_infrared_z_min_check = true; // 打开红外调平检测
//    st_enable_endstops(true); // 打开限位检测，避免由于逻辑错乱，限位检测关闭导致无法限位
//    DGUS_target[Z_AXIS] = -INFRARED_Z_DOWN_HIGH;
//    process_buffer_line_normal(DGUS_target,DGUS_feedrate/60);

//    if(st_check_endstop_z_hit_min())
//    {
//      z = st_get_endstops_len(Z_AXIS);
//      DGUS_target[Z_AXIS] = z;
//      USER_EchoLogStr("endstop xyz %f %f %f\r\n", DGUS_target[X_AXIS], DGUS_target[Y_AXIS], DGUS_target[Z_AXIS]);
//    }
//    sg_grbl::planner_set_position(DGUS_target);

//    (void)sys_os_delay(100);
//  }

//  static inline void travel_to_pos(CONST FLOAT &x, CONST FLOAT &y, CONST FLOAT &z)
//  {
//    st_enable_endstops(false);
//    DGUS_target[X_AXIS] = x;
//    DGUS_target[Y_AXIS] = y;
//    DGUS_target[Z_AXIS] = z;
//    process_buffer_line_normal(DGUS_target,DGUS_feedrate/60);
//    USER_EchoLogStr("travel_to_pos:%f %f %f\r\n", x, y, z);
//  }

//  //左前点，第一个点
//  void DGUS_bed_level_first1(void)
//  {
//    static uint8_t OneceTime = 1;
//    if(OneceTime)//每次进来都归零一次，防止各轴不在零点就开始调平台
//    {
//      OneceTime = 0;
//      display_picture(83);

//      infrared_z_zero_adjust_autohome();

//      t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
//      return;
//    }

//    if((1U == t_gui_p.G28_ENDSTOPS_COMPLETE))
//    {
//      //如果已经归0
//      if(gui_is_refresh())
//      {
//        st_enable_endstops(true);
//        PointID = 1;
//        display_picture(85);

//        for(int8_t i=0; i < motion_3d.axis_num; ++i)
//        {
//          DGUS_target[i] = gcode::get_current_position(i);
//        }
//        sg_grbl::planner_set_position(DGUS_target);
//      }
//      if(!sg_grbl::planner_blocks_queued())//解锁xy电机，手工调节第一点
//      {
//        stepper_axis_enable(X_AXIS, false);
//        stepper_axis_enable(Y_AXIS, false);
//      }
//      // OneceTime =1;
//    }
//    else //如果未归零则返回
//      return;

//    static bool OneceTime1 = 1;
//    if(touchxy(84,215,184,264) || DGUS_OK_KEY)
//    {
//      DGUS_OK_KEY = false;
//      if(OneceTime1)
//      {
//        display_picture(91);//平台正在校准界面，避免在校准时按下按键
//        OneceTime1 = 0;
//      }

//      gui_set_curr_display(GetThreePoint);
//      OneceTime = 1;//下次进来，需要归零1次
//    }
//  }



////探测红外在三个点的状态，避免调一个点影响到其它点
//  void GetThreePoint(void)
//  {
//    display_picture(91);//平台正在校准界面，避免在校准时按下按键
//    DGUS_feedrate = 70*60;

//    getPosHigh(left_bed_position, front_bed_position, firstZposition);
//    getPosHigh(left_bed_position, back_bed_position, zHigh[0]);
//    zHigh[0] -= firstZposition;

//    (void)sys_os_delay(100);
//    buzz(100);

//    getPosHigh(left_bed_position, front_bed_position, firstZposition);
//    getPosHigh(right_bed_position, front_bed_position, zHigh[2]);
//    zHigh[2] -= firstZposition;

//    (void)sys_os_delay(100);
//    buzz(100);

//    getPosHigh(left_bed_position, front_bed_position, firstZposition);

//    //开始手动校准
//    gui_set_curr_display(DGUS_bed_level_second1);
//  }
//  //右前点，第三个点
//  void DGUS_bed_level_first2(void);
//  //左后点，第二个点
//  void DGUS_bed_level_second2(void);
////右后点,第四个点
//  void DGUS_bed_level_fourth2(void);
////探测红外在三个点的状态，避免调一个点影响到其它点
//  void GetThreePoint2(void)
//  {
//    display_picture(91);//平台正在校准界面，避免在校准时按下按键
//    DGUS_feedrate = 70*60;

//    getPosHigh(right_bed_position, front_bed_position, firstZposition);
//    getPosHigh(left_bed_position, front_bed_position, zHigh[0]);
//    zHigh[0] -= firstZposition;

//    (void)sys_os_delay(100);
//    buzz(100);


//    getPosHigh(right_bed_position, front_bed_position, firstZposition);
//    getPosHigh(right_bed_position, back_bed_position, zHigh[2]);
//    zHigh[2] -= firstZposition;

//    (void)sys_os_delay(100);
//    buzz(100);

//    getPosHigh(right_bed_position, front_bed_position, firstZposition);

//    //开始手动校准
//    gui_set_curr_display(DGUS_bed_level_first2);
//  }

//  void AdjustThreeAngle1(void)
//  {
//    static bool showPic = true;
//    //平台上升3次，测出平台高度与第一点比较
//    while(1)
//    {
//      //(zHigh[0] + zHigh[1] + zHigh[2])/3.0F -
//      if(abs( zHigh[PointID-2]) < 0.01F)
//      {
//        USER_EchoLogStr("adjust %f \r\n", zHigh[PointID-2]);
//        USER_EchoLogStr("adjust %d OK!\r\n",PointID);
//        buzz(150);
//        break;
//      }
//      else
//      {
//        //(zHigh[0] + zHigh[1] + zHigh[2])/3
//        if( 0.0F > zHigh[PointID-2])
//        {
//          if(showPic)
//          {
//            display_picture(86+PointID-2);//       DGUS_bed_level_Up();
//            showPic = 0;
//          }
//          if(HAL_GPIO_ReadPin(E1_STEP_PIN_GPIO,E1_STEP_PIN)  == 0)
//          {
//            USER_EchoLogStr("adjust %d OK!\r\n",PointID);
//            buzz(150);
//            showPic = 1;
//            break;
//          }
//        }
//        else
//        {
//          if(showPic)
//          {
//            display_picture(99+PointID-2);//       DGUS_bed_level_Down();
//            showPic = 0;
//          }
//          if(HAL_GPIO_ReadPin(E1_STEP_PIN_GPIO,E1_STEP_PIN)  == 1)
//          {
//            USER_EchoLogStr("adjust %d OK!\r\n",PointID);
//            buzz(150);
//            showPic = 1;
//            break;
//          }
//        }
//      }
//      (void)sys_os_delay(10);
//    }
//  }

////左后点，第二个点
//  void DGUS_bed_level_second1(void)
//  {
//    if(gui_is_refresh())
//    {
//      PointID = 2;
//      display_picture(91);//平台正在校准界面，避免在校准时按下按键

//      travel_to_pos(left_bed_position, back_bed_position, firstZposition);
//    }

//    if(PointID)
//    {
//      AdjustThreeAngle1();
//      gui_set_curr_display(DGUS_bed_level_fourth1);
//    }
//  }
////右前点，第三个点
//  void DGUS_bed_level_third1(void)
//  {
//    if(gui_is_refresh())
//    {
//      PointID = 3;

//      //移动喷嘴到右后点
//      display_picture(91);//平台正在校准界面，避免在校准时按下按键

//      travel_to_pos(right_bed_position, back_bed_position, firstZposition);
//    }
//    if(PointID)
//    {
//      AdjustThreeAngle1();
//      gui_set_curr_display(DGUS_bed_level_fourth1);
//    }
//  }
////右后点,第四个点
//  void DGUS_bed_level_fourth1(void)
//  {
//    if(gui_is_refresh())
//    {
//      PointID = 4;
//      //移动喷嘴到右后点
//      display_picture(91);//平台正在校准界面，避免在校准时按下按键

//      travel_to_pos(right_bed_position, front_bed_position, firstZposition);
//    }

//    if(PointID)
//    {
//      AdjustThreeAngle1();
//      gui_set_curr_display(GetThreePoint2);
//    }
//  }



//  //右前点，第三个点
//  void DGUS_bed_level_first2(void)
//  {
//    if(gui_is_refresh())
//    {
//      PointID = 2;

//      //移动喷嘴到右后点
//      display_picture(91);//平台正在校准界面，避免在校准时按下按键

//      travel_to_pos(left_bed_position, front_bed_position, firstZposition);
//    }
//    if(PointID)
//    {
//      AdjustThreeAngle1();
//      gui_set_curr_display(DGUS_bed_level_second2);
//    }
//  }
//  //左后点，第二个点
//  void DGUS_bed_level_second2(void)
//  {
//    if(gui_is_refresh())
//    {
//      PointID = 4;
//      display_picture(91);//平台正在校准界面，避免在校准时按下按键

//      travel_to_pos(right_bed_position, back_bed_position, (firstZposition));// + (zHigh[0] + zHigh[1] + zHigh[2])/3));
//    }

//    if(PointID)
//    {
//      AdjustThreeAngle1();
//      gui_set_curr_display(DGUS_bed_level_sixth1);
//    }
//  }

////右后点,第四个点
//  void DGUS_bed_level_fourth2(void)
//  {
//    if(gui_is_refresh())
//    {
//      PointID = 4;
//      //移动喷嘴到右后点
//      display_picture(91);//平台正在校准界面，避免在校准时按下按键
//      travel_to_pos(right_bed_position, front_bed_position, firstZposition);
//    }

//    if(PointID)
//    {
//      AdjustThreeAngle1();
//      gui_set_curr_display(DGUS_bed_level_sixth1);
//    }
//  }

////调平完成
//  void DGUS_bed_level_sixth1(void)
//  {
//    static ULONG BeepWaringTime=0;
//    if(gui_is_refresh())
//    {
//      display_picture(90);
//      PointID = 6;
//      respond_gui_send_sem(OpenBeep);
//      BeepWaringTime=sys_task_get_tick_count()+5000; //鸣叫5秒
//    }
//    if(BeepWaringTime < sys_task_get_tick_count()) //时间到关闭鸣叫
//    {
//      respond_gui_send_sem(CloseBeep);
//    }
//    if(touchxy(84,215,184,264) || DGUS_OK_KEY)
//    {
//      DGUS_OK_KEY = false;
//      respond_gui_send_sem(CloseBeep);
//      display_picture(83);//提示等待机器归零
//      infrared_z_zero_adjust_autohome();
//      gui_set_curr_display(maindisplayF);
//    }
//  }

#endif // CAL_Z_ZERO_OFFSET

#ifdef __cplusplus
} // extern "C" {
#endif



