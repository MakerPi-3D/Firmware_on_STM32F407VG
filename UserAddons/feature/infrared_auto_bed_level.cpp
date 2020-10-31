#include "infrared_auto_bed_level.h"
#include "stepper.h"
#include "user_interface.h"
#include "planner.h"
#include "view_pic_display.h"
#include "stm32f4xx_hal.h"
#include "math.h"

#include "planner.h"
#include "globalvariables.h"
#include "autobedleveling.h"
#include "interface.h"
#include "gcodebufferhandle.h"
#include "view_common.h"
#include "gcode_global_params.h"

//#include "stepper_pin.h"

#ifdef __cplusplus
extern "C" {
#endif
  /***************************红外传感器与M3145S有用到这些2017.10.18*********************/
  static FLOAT  left_bed_position; //= 1.0F; //喷嘴碰平台左边
  static FLOAT  right_bed_position;//= 210; //喷嘴碰平台右边
  static FLOAT  front_bed_position;//= 1.0F; //喷嘴碰平台前边
  static FLOAT  back_bed_position; // = 210;//280.0F;  //喷嘴碰平台后边
  static FLOAT  middle_bed_position;// = 125.0F;  //喷嘴碰平台后边
  CONST  FLOAT  bed_position_offset = 3.0F; //每点三角形偏移量
//校准平台时，5个点中又分4个点，通过查表法取值
  CONST FLOAT autobedlevelPoint[5][4][2]=
  {
    {{20.0F,1.0F},   {20.0F+bed_position_offset,1.0F},   {20.0F,1.0F+bed_position_offset},   {20.0F+bed_position_offset,1.0F+bed_position_offset}}, // float
    {{20.0F,210.0F}, {20.0F+bed_position_offset,210.0F}, {20.0F,210.0F-bed_position_offset}, {20.0F+bed_position_offset,210.0F-bed_position_offset}}, // float
    {{260.0F,210.0F},{260.0F-bed_position_offset,210.0F},{260.0F,210.0F-bed_position_offset},{260.0F-bed_position_offset,210.0F-bed_position_offset}}, // float
    {{260.0F,1.0F},  {260.0F-bed_position_offset,1.0F},  {260.0F,1.0F+bed_position_offset},  {260.0F-bed_position_offset,1.0F+bed_position_offset}}, // float
    {{150.0F,150.0F},{150.0F-bed_position_offset,150.0F},{150.0F,150.0F-bed_position_offset},{150.0F-bed_position_offset,150.0F-bed_position_offset}}, // float
  };

  static bool serial_check_ok = false;

  void infrared_auto_bed_init_position(CONST FLOAT &_left_bed_position, CONST FLOAT &_right_bed_position, CONST FLOAT &_front_bed_position, CONST FLOAT &_back_bed_position, CONST FLOAT &_middle_bed_position)
  {
    left_bed_position = _left_bed_position;
    right_bed_position = _right_bed_position;
    front_bed_position = _front_bed_position;
    back_bed_position = _back_bed_position;
    middle_bed_position = _middle_bed_position;
  }

  /**************************************************************************************/

  /*********************************************************
  *红外传感器调平台辅助系统
  *20170814
  *********************************************************
  */
#include "user_debug.h"

#define BedLevelDownValue 20 //调平时平台下降的高度
#define SizeError 0.2F //测量误差允许范围

  static FLOAT firstZposition = 0;
  static FLOAT currentZposition = 0;
  static menufunc_t lastdisplay = maindisplayF;

  static FLOAT DGUS_target[MAX_NUM_AXIS]= {0,0,0,0,0};
  static FLOAT DGUS_feedrate = 30*60;//sg_grbl::homing_feedrate[Z_AXIS];
  static UINT32 PointID;//检测点的id，即当前点是第几个点


  /*******************************外部变量**********************************************/
  FLOAT circlenum = 0;//螺丝旋转的圈数

  /*
   *M3螺距0.5mm，即旋转一圈螺母变化0.5mm
   *迪文屏显示小数的规律是：10->1.0；110->11.0。迪文屏数据变量设置整数位数2个，小数位数1个；所以相应的小数乘以十就能在迪文屏显示了
  */
//平台偏低，应上升高度
  void DGUS_bed_level_Up(void)
  {
    CHAR TextBuffer[20];

    if(gui_is_refresh())
    {
      circlenum = (firstZposition - currentZposition) + 0.1F;//提示多旋转0.2圈，多旋转可通过矩阵补偿
      circlenum*=2;  //circlenum=circlenum/0.5;乘法比除法效率高
      display_picture((86+PointID)-2);
      sg_grbl::st_synchronize();
      sys_send_gcode_cmd("G29 F2 isInternal");
      snprintf(TextBuffer, sizeof(TextBuffer), "%.1f",circlenum);

      SetTextDisplayRange(383,239,12*9,24,&PrintTimeTextRange);
      ReadTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf);
      CopyTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintTimeTextRange,TextRangeBuf);
    }
    if(touchxy(84,215,184,264) || serial_check_ok)//调节平台完成，按下ok键返回上一个界面
    {
      serial_check_ok = false;
      gui_set_curr_display(lastdisplay);
//		if(PointID != 5)//如果不是在检测中间点，就进入到这里，则说明调高了0点，中间点的值要重新赋
//		  t_sys_data_current.bed_level_z_at_middle = 0;
    }
  }

//平台偏高，应下降高度
  void DGUS_bed_level_Down(void)
  {
    CHAR TextBuffer[20];

    if(gui_is_refresh())
    {
      circlenum = currentZposition - firstZposition; // float
      circlenum*=2;  //circlenum=circlenum/0.5;乘法比除法效率高
      display_picture((99+PointID)-2);
      sg_grbl::st_synchronize();
      sys_send_gcode_cmd("G29 F3 isInternal");
      snprintf(TextBuffer, sizeof(TextBuffer), "%.1f",circlenum);

      SetTextDisplayRange(383,239,12*9,24,&PrintTimeTextRange);
      ReadTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf);
      CopyTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf, TextRangeBuf);
      DisplayTextInRangeDefault((PUCHAR)TextBuffer, PrintTimeTextRange,TextRangeBuf);
    }
    if(touchxy(84,215,184,264) || serial_check_ok)//调节平台完成，按下ok键返回上一个界面
    {
      serial_check_ok = false;
      gui_set_curr_display(lastdisplay);
    }
  }

  void DGUS_bed_level_Serial(UINT8 isUp)
  {
    USER_EchoLogStr("circlenum=%.1f  ",circlenum);
    if(isUp)
    {
      USER_EchoLogStr("UP:%d\r\n",PointID);
    }
    else
    {
      USER_EchoLogStr("DOWN:%d\r\n",PointID);
    }
  }

  void gcodeCMD_g29_interface(bool isSerial)
  {
    //extern void DGUS_bed_level_Serial(UINT8 isUp);
    if((!gcode::parseGcodeBufHandle.codeSeen('S')) && (!gcode::parseGcodeBufHandle.codeSeen('F')))
    {
//    isG29On = 1;
//    sys_os_delay(50);
//    GcodeCMD_G28();
//    autoBedLeveling.Gcode_G29();
      serial_check_ok = false;
      autoBedLeveling.isSerial = isSerial;
      gui_set_curr_display(DGUS_bed_level_first);
    }


    if(gcode::parseGcodeBufHandle.codeSeen('S'))
    {
      serial_check_ok = true;
    }

    if(gcode::parseGcodeBufHandle.codeSeen('F'))
    {
      if(1 == gcode::parseGcodeBufHandle.codeValue())
      {
        USER_EchoLogStr("G29 ok\r\n");
      }
      else if(2 == gcode::parseGcodeBufHandle.codeValue())
      {
        DGUS_bed_level_Serial(1);
      }
      else if(3 == gcode::parseGcodeBufHandle.codeValue())
      {
        DGUS_bed_level_Serial(0);
      }
      else if(4 == gcode::parseGcodeBufHandle.codeValue())
      {
        USER_EchoLogStr("G28 ok\r\n");
      }
      else if(5 == gcode::parseGcodeBufHandle.codeValue())
      {
        USER_EchoLogStr("adjustbedlevel finish\r\n");
      }
    }
  }

//平台上升下降2次，取平台高度平均值
  void DGUS_Run_Z_Probe(volatile float (&target)[MAX_NUM_AXIS],volatile float &Zposition)
  {
    UINT32 COUNT  = 4;
    UINT32 loop  = 4;
    Zposition = 0;

#ifdef ENABLE_AUTO_BED_LEVELING_DEBUG
    UINT32 RCount;
    CHAR databuf[30];
    FIL* file1;
    FRESULT f_res;
    (void)f_unlink("1:/senserdata.txt");
    file1 = (FIL*)malloc(sizeof(FIL));
    //创建新文件
    f_res=f_open(file1,"1:/senserdata.txt",FA_CREATE_NEW|FA_WRITE);
    USER_EchoLogStr("Create file1= %d\n",f_res);
    if(f_res)
    {

      f_res=f_open(file1,"1:/senserdata.txt",FA_WRITE);
      f_res=f_lseek(file1,f_size(file1));
      USER_EchoLogStr("f_lseek file1= %d\n",f_res);
      snprintf(databuf,sizeof(databuf),"############%d##############\r\n",PointID);
      f_res = f_write(file1,databuf,strlen(databuf),&RCount);
    }
    USER_EchoLogStr("f_write file1= %d\n",f_res);
    f_res=f_lseek(file1,f_size(file1));
    USER_EchoLogStr("f_lseek file1= %d\n",f_res);
#endif

    while(loop)
    {
      switch(loop)
      {
      case 4:
        //plan_buffer_line(target,DGUS_feedrate/60,active_extruder,0,100,0, 100);
////			  USER_EchoLogStr("loop = %d\r\n",loop);
        sg_grbl::st_synchronize();
        sg_grbl::planner_set_position(target);
        break;
      case 3:
////				USER_EchoLogStr("loop = %d\r\n",loop);
        target[X_AXIS] = autobedlevelPoint[PointID-1][1][0];
        target[Y_AXIS] = autobedlevelPoint[PointID-1][1][1];
        gcode::process_buffer_line_normal(target,DGUS_feedrate/60); // float
        sg_grbl::planner_set_position(target);
        break;
      case 2:
////				USER_EchoLogStr("loop = %d\r\n",loop);
        target[X_AXIS] = autobedlevelPoint[PointID-1][2][0];
        target[Y_AXIS] = autobedlevelPoint[PointID-1][2][1];
        gcode::process_buffer_line_normal(target,DGUS_feedrate/60); // float
        sg_grbl::planner_set_position(target);
        break;
      case 1:
////				USER_EchoLogStr("loop = %d\r\n",loop);
        target[X_AXIS] = autobedlevelPoint[PointID-1][3][0];
        target[Y_AXIS] = autobedlevelPoint[PointID-1][3][1];
        gcode::process_buffer_line_normal(target,DGUS_feedrate/60); // float
        sg_grbl::planner_set_position(target);
        break;
      default:
        break;
      }
      /*一、平台上升*/
      motion_3d.is_cal_bed_platform = false;
      target[Z_AXIS] -= (BedLevelDownValue+30); // float
      gcode::process_buffer_line_normal(target,DGUS_feedrate/60); // float
      while(sg_grbl::st_check_endstop_z_hit_min() == false)
        (void)sys_os_delay(50);

      /*二、记录平台当前高度*/
      target[Z_AXIS] = sg_grbl::st_get_position_length(Z_AXIS);
      Zposition += target[Z_AXIS]; // float
#ifdef ENABLE_AUTO_BED_LEVELING_DEBUG
      snprintf(databuf,sizeof(databuf),"%f\r\n",target[Z_AXIS]);
      f_res = f_write(file1,databuf,strlen(databuf),&RCount);
      USER_EchoLogStr("write file1= %d\r\n",f_res);
#endif
      sg_grbl::planner_set_position(target);

      /*三、平台下降*/
      motion_3d.is_cal_bed_platform = true;
      target[Z_AXIS] = BedLevelDownValue;
      gcode::process_buffer_line_normal(target,DGUS_feedrate/60); // float
      while(sg_grbl::st_get_position_length(Z_AXIS) < (BedLevelDownValue-1.0F)) // float
        (void)sys_os_delay(50);
      sg_grbl::planner_set_position(target);
      loop--;
    }
#ifdef ENABLE_AUTO_BED_LEVELING_DEBUG
    f_close(file1);
    free(file1);
//	USER_EchoLogStr("$$$$$$$$$$$$$$$$$___finish___$$$$$$$$$$$$$$$$$$$$$\r\n");
#endif //ENABLE_AUTO_BED_LEVELING_DEBUG
    Zposition /= COUNT;
//  Zposition *= 1.2F;//如果使用A4纸或便利贴则不用这句；1.2是使用浅蓝色便利贴和蓝色平台贴纸测得的数据，所得出的比例值
  }
  void DGUS_bed_level_second(void);
  void DGUS_bed_level_third(void);
  void DGUS_bed_level_fourth(void);
  void AdjustThreeAngle(void);//以第一点为基准，调节其余三个点
  void DGUS_bed_level_fifth(void);
  void DGUS_bed_level_sixth(void);

//左前点，第一个点
  void DGUS_bed_level_first(void)
  {
    static UINT8 OneceTime = 1;
    if(OneceTime)//每次进来都归零一次，防止各轴不在零点就开始调平台
    {
      OneceTime = 0;
      if(!autoBedLeveling.isSerial)
      {
        display_picture(83);
      }
      sys_send_gcode_cmd("G28 X0 Y0 S isInternal");//加多S参数表示不返回G28 ok
      sys_send_gcode_cmd("G28 Z0 isInternal");
      sg_grbl::st_synchronize();
      sys_send_gcode_cmd("G29 F1 isInternal");
      t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
      return;
    }

    if(1U == t_gui_p.G28_ENDSTOPS_COMPLETE)
    {
      //如果已经归0
      if(gui_is_refresh())
      {
        sg_grbl::st_enable_endstops(true);
        motion_3d.is_cal_bed_platform = false;
        sg_grbl::plan_set_process_auto_bed_level_status(false);
        PointID = 1;
        if(!autoBedLeveling.isSerial)
        {
          display_picture(85);
        }

        for(int8_t i=0; i < motion_3d.axis_num; ++i)
        {
          DGUS_target[i] = gcode::get_current_position(i);
        }
        sg_grbl::planner_set_position(DGUS_target);
      }
      if(!sg_grbl::planner_blocks_queued())//解锁xy电机，手工调节第一点
      {
        sg_grbl::st_enable_axis(X_AXIS, false);
        sg_grbl::st_enable_axis(Y_AXIS, false);
      }
    }
    else //如果未归零则返回
    {
      return;
    }

    if(touchxy(84,215,184,264) || serial_check_ok)
    {
      serial_check_ok = false;
      if(!autoBedLeveling.isSerial)
      {
        display_picture(91);//平台正在校准界面，避免在校准时按下按键
      }

      //平台下降
      motion_3d.is_cal_bed_platform = true;
      DGUS_target[Z_AXIS] = BedLevelDownValue;
      gcode::process_buffer_line_normal(DGUS_target,DGUS_feedrate/60); // float
      while(sg_grbl::st_get_position_length(Z_AXIS) < (BedLevelDownValue-1)) // float
        (void)sys_os_delay(50);
      //喷嘴移动到左前点
      DGUS_target[X_AXIS] = left_bed_position;
      DGUS_target[Y_AXIS] = front_bed_position;
      gcode::process_buffer_line_normal(DGUS_target,DGUS_feedrate/60); // float
      while(sg_grbl::st_get_position_length(X_AXIS) < (left_bed_position-1)) // float
        (void)sys_os_delay(50);
      gpio_infrated_bed_level_init(false);
      //平台上升3次碰探针，记录此时平台高度firstZposition，探针缩回，平台下降到20
      sg_grbl::st_check_endstop_z_hit_min();//清空限位缓存
      DGUS_Run_Z_Probe(DGUS_target,firstZposition);
      if( (fabs(firstZposition - t_sys_data_current.bed_level_z_at_left_front) > SizeError) || //红外模块的误差不超过0.2
          ((t_sys_data_current.bed_level_z_at_middle - firstZposition) < 0.1F) )                 //如果现存的中间点的值不大于第一点，就用第一个点的值做第5界面的比较值
      {
        t_sys_data_current.bed_level_z_at_middle = firstZposition;
      }
      t_sys_data_current.bed_level_z_at_left_front = firstZposition;
      gui_set_curr_display(DGUS_bed_level_second);
      OneceTime = 1;//下次进来，需要归零1次
    }

  }

//左后点，第二个点
  void DGUS_bed_level_second(void)
  {
    if(gui_is_refresh())
    {
      sg_grbl::st_check_endstop_z_hit_min();//清空限位缓存
      PointID = 2;
      lastdisplay = DGUS_bed_level_second;
      if(!autoBedLeveling.isSerial)
      {
        display_picture(91);//平台正在校准界面，避免在校准时按下按键
      }

      //喷嘴移动到左后点
      DGUS_target[X_AXIS] = left_bed_position;
      DGUS_target[Y_AXIS] = back_bed_position;
      gcode::process_buffer_line_normal(DGUS_target,DGUS_feedrate/60); // float
      while(sg_grbl::st_get_position_length(Y_AXIS) < (back_bed_position-1)) // float
        (void)sys_os_delay(50);
      sg_grbl::planner_set_position(DGUS_target);
      gcode::set_current_position(X_AXIS,DGUS_target[X_AXIS]);
      gcode::set_current_position(Y_AXIS,DGUS_target[Y_AXIS]);
      gcode::set_current_position(Z_AXIS,DGUS_target[Z_AXIS]);

    }

    if(PointID)
    {

      //平台上升3次，测出平台高度与第一点比较
      DGUS_Run_Z_Probe(DGUS_target,currentZposition);

      t_sys_data_current.bed_level_z_at_left_back = currentZposition;
      gui_set_curr_display(DGUS_bed_level_third);
    }
  }
//右前点，第三个点
  void DGUS_bed_level_third(void)
  {
    if(gui_is_refresh())
    {
      PointID = 3;
      lastdisplay = DGUS_bed_level_fourth;

      //移动喷嘴到右后点
      if(!autoBedLeveling.isSerial)
      {
        display_picture(91);//平台正在校准界面，避免在校准时按下按键
      }
      DGUS_target[X_AXIS] = right_bed_position;
      DGUS_target[Y_AXIS] = back_bed_position;
      gcode::process_buffer_line_normal(DGUS_target,DGUS_feedrate/60); // float
      sg_grbl::planner_set_position(DGUS_target);
      gcode::set_current_position(X_AXIS,DGUS_target[X_AXIS]);
      gcode::set_current_position(Y_AXIS,DGUS_target[Y_AXIS]);
      gcode::set_current_position(Z_AXIS,DGUS_target[Z_AXIS]);

    }

    if(PointID)
    {
      //平台上升3次，测出平台高度与第一点比较
      DGUS_Run_Z_Probe(DGUS_target,currentZposition);

      t_sys_data_current.bed_level_z_at_right_back = currentZposition;
      gui_set_curr_display(DGUS_bed_level_fourth);
    }
  }
//右后点,第四个点
  void DGUS_bed_level_fourth(void)
  {
    if(gui_is_refresh())
    {
      PointID = 4;
      lastdisplay = DGUS_bed_level_third;

      //移动喷嘴到右后点
      if(!autoBedLeveling.isSerial)
      {
        display_picture(91);//平台正在校准界面，避免在校准时按下按键
      }
      DGUS_target[X_AXIS] = right_bed_position;
      DGUS_target[Y_AXIS] = front_bed_position;//back_bed_position
      gcode::process_buffer_line_normal(DGUS_target,DGUS_feedrate/60);
      sg_grbl::planner_set_position(DGUS_target);
      gcode::set_current_position(X_AXIS,DGUS_target[X_AXIS]);
      gcode::set_current_position(Y_AXIS,DGUS_target[Y_AXIS]);
      gcode::set_current_position(Z_AXIS,DGUS_target[Z_AXIS]);

    }

    if(PointID)
    {
      DGUS_Run_Z_Probe(DGUS_target,currentZposition);

      t_sys_data_current.bed_level_z_at_right_front = currentZposition;
      gui_set_curr_display(AdjustThreeAngle);
    }
  }
//以第一点为基准，调节其余三个点
  void AdjustThreeAngle(void)
  {
    static UINT8 IsOrNotAdjust = 0;//是否调整了某个点
    if(gui_is_refresh())
    {
      lastdisplay = AdjustThreeAngle;
      if(!autoBedLeveling.isSerial)
      {
        display_picture(91);//平台正在校准界面，避免在校准时按下按键
      }
    }

    if(t_sys_data_current.bed_level_z_at_left_back < (firstZposition-SizeError))//平台相对于左前点低了，所以要调高
    {
      IsOrNotAdjust = 1;
      PointID = 2;
      currentZposition = t_sys_data_current.bed_level_z_at_left_back;
      gui_set_curr_display(DGUS_bed_level_Up);
      t_sys_data_current.bed_level_z_at_left_back = firstZposition;
    }
    else if(t_sys_data_current.bed_level_z_at_right_back < (firstZposition-SizeError))//平台相对于左前点低了，所以要调高
    {
      IsOrNotAdjust = 1;
      PointID = 3;
      currentZposition = t_sys_data_current.bed_level_z_at_right_back;
      gui_set_curr_display(DGUS_bed_level_Up);
      t_sys_data_current.bed_level_z_at_right_back = firstZposition;
    }
    else if(t_sys_data_current.bed_level_z_at_right_front < (firstZposition-SizeError))//平台相对于左前点低了，所以要调高
    {
      IsOrNotAdjust = 1;
      PointID = 4;
      currentZposition = t_sys_data_current.bed_level_z_at_right_front;
      gui_set_curr_display(DGUS_bed_level_Up);
      t_sys_data_current.bed_level_z_at_right_front = firstZposition;
    }
    else if(IsOrNotAdjust)
    {
      IsOrNotAdjust = 0;
      gui_set_curr_display(DGUS_bed_level_second);
    }
    else
    {
      IsOrNotAdjust = 0;
      gui_set_curr_display(DGUS_bed_level_fifth);
    }
  }

//中间点，第五个点
  void DGUS_bed_level_fifth(void)
  {
    if(gui_is_refresh())
    {
      PointID = 5;
      lastdisplay = DGUS_bed_level_fifth;

      //移动喷嘴到右后点
      if(!autoBedLeveling.isSerial)
      {
        display_picture(91);//平台正在校准界面，避免在校准时按下按键
      }
      DGUS_target[X_AXIS] = middle_bed_position;
      DGUS_target[Y_AXIS] = middle_bed_position;
      gcode::process_buffer_line_normal(DGUS_target,DGUS_feedrate/60); // float
      sg_grbl::planner_set_position(DGUS_target);
      gcode::set_current_position(X_AXIS,DGUS_target[X_AXIS]);
      gcode::set_current_position(Y_AXIS,DGUS_target[Y_AXIS]);
      gcode::set_current_position(Z_AXIS,DGUS_target[Z_AXIS]);
    }

    if(PointID)
    {
      //平台上升3次，测出平台高度与第一点比较
      DGUS_Run_Z_Probe(DGUS_target,currentZposition);

      // 设置平台矩阵
      sg_grbl::plan_set_bed_level_equation();
      motion_3d.is_cal_bed_platform = true;
      DGUS_target[Z_AXIS] = 0.0F;
      // 矩阵变换，转化移动坐标
      sg_grbl::plan_set_process_auto_bed_level_status(true);
      sg_grbl::plan_buffer_line_get_xyz(DGUS_target[X_AXIS], DGUS_target[Y_AXIS], DGUS_target[Z_AXIS]);
      sg_grbl::plan_set_process_auto_bed_level_status(false);
      motion_3d.is_cal_bed_platform = false;
      currentZposition -= DGUS_target[Z_AXIS];//测量出的中间点高度减去斜度引起的高度差
      DGUS_target[Z_AXIS] = BedLevelDownValue;

      // 如果是烧录固件后第一次校准平台，t_sys.bed_level_z_at_middle会被赋值为第一个点的高度（在测量第一个点高度的函数里）
      if((currentZposition - t_sys_data_current.bed_level_z_at_middle) > SizeError)//中间点比上一次测量高度大于0.2
      {
        gui_set_curr_display(DGUS_bed_level_Down);
      }
      else//调平完成
      {
        autoBedLevelingAdjust.calculateBedLevelFinish();
        motion_3d.is_cal_bed_platform = true;
        sg_grbl::plan_set_process_auto_bed_level_status(true);
        sg_grbl::st_synchronize();
        sys_send_gcode_cmd("G29 F4 isInternal");
        t_sys_data_current.bed_level_z_at_middle = currentZposition;
        gui_set_curr_display(DGUS_bed_level_sixth);
      }
    }
  }

//调平完成
  void DGUS_bed_level_sixth(void)
  {
    static ULONG BeepWaringTime=0;
    if(gui_is_refresh())
    {
      if(!autoBedLeveling.isSerial)
      {
        display_picture(90);
      }
      PointID = 6;

      gpio_infrated_bed_level_init(true);

//      gui_send_sem_open_beep();
      respond_gui_send_sem(OpenBeep);
      BeepWaringTime=sys_task_get_tick_count()+5000; //鸣叫5秒
      sys_send_gcode_cmd("G29 F5 isInternal");
    }
    if(BeepWaringTime < sys_task_get_tick_count()) //时间到关闭鸣叫
    {
//      gui_send_sem_close_beep();
      respond_gui_send_sem(CloseBeep);
    }
    if(touchxy(84,215,184,264) || serial_check_ok)
    {
      serial_check_ok = false;
//      gui_send_sem_close_beep();
      respond_gui_send_sem(CloseBeep);
      if(!autoBedLeveling.isSerial)
      {
        display_picture(83);//提示等待机器归零
      }
//    autoBedLevelingAdjust.calculateBedLevelFinish();
//    autoBedLevelingAdjust.isCalBedPlatform = true;
//    autoBedLeveling.hasProcessAutoBedLeveling(true);
//    USER_EchoLogStr("G28 ok\r\n");
      if(!autoBedLeveling.isSerial)
      {
        gui_set_curr_display(maindisplayF);
      }
      autoBedLeveling.isSerial = 0;
//    sys_os_delay(50);
//    isG29On = 0;
    }

  }

#ifdef __cplusplus
} // extern "C" {
#endif

