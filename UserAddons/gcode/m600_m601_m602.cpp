#include "globalvariables_ccmram.h"
#include "temperature.h"
#include "sys_function.h"
#include "interface.h"
#include "stepper.h"
#include "gcodebufferhandle.h"
#include "planner.h"
#include "PrintControl.h"
#include "autobedlevelinterface.h"
#include "gcode_global_params.h"
#include "user_debug.h"
#include "controlfunction.h"

namespace gcode
{
#define FILAMENTCHANGE_XPOS 0  //当X轴返回零点时，设置X轴为0点坐标
#define FILAMENTCHANGE_YPOS 0  //当Y轴返回零点时，设置Y轴为0点坐标
#define FILAMENTCHANGE_PREPARE_RETRACT -80
#define FILAMENTCHANGE_FIRSTRETRACT -50  //第二步退料未接收到'E'或'B'命令时用
#define FILAMENTCHANGE_FINALRETRACT -80	 //第三步退料未接收到'L'命令时用
#define FIRST_LOAD_MM 80
#define FINAL_LOAD_MM 155	//返回打印前，最后一次改变出丝量
#define EB_DIFF_MM   (INT)(159.7-116.7) //消除EB电机高度差

  // init position
  static float m600_target[MAX_NUM_AXIS];
  static float m600_lastpos[MAX_NUM_AXIS]= {0.0f};
	


  static void m600_xy_homing(const float m600_feedrate)
  {
    sg_grbl::planner_set_position(m600_target);
    m600_target[(INT)X_AXIS] = 1.5F * motion_3d_model.xyz_max_length[(INT)X_AXIS] * motion_3d_model.xyz_home_dir[(INT)X_AXIS];// float
    gcode::process_buffer_line_normal(m600_target, m600_feedrate/60); // float
    m600_target[(INT)Y_AXIS] = 1.5F * motion_3d_model.xyz_max_length[(INT)Y_AXIS] * motion_3d_model.xyz_home_dir[(INT)Y_AXIS];// float
    gcode::process_buffer_line_normal(m600_target, m600_feedrate/60); // float

#ifdef FILAMENTCHANGE_XPOS
    m600_target[X_AXIS]= motion_3d_model.xyz_home_pos[X_AXIS];
#endif
#ifdef FILAMENTCHANGE_YPOS
    m600_target[Y_AXIS]= motion_3d_model.xyz_home_pos[Y_AXIS];
#endif
    sg_grbl::planner_set_position(m600_target);
  }

  /**
    * @brief 控制X,Y轴返回零点，Z保持高度
    * @param target：指向现时的XYZ轴位置
    * @retval None
    * @Date  2017/03/09
    */
  static void m600_xy_home_process(void)
  {
    float xy_homing_feedrate = 0.0f;
    xy_homing_feedrate = gcode::homing_feedrate[X_AXIS];
    if(gcode::homing_feedrate[Y_AXIS]< xy_homing_feedrate)
    {
      xy_homing_feedrate = gcode::homing_feedrate[Y_AXIS];
    }
    //XY正常速度归零
    m600_xy_homing(xy_homing_feedrate);

    //XY慢速归零
    m600_target[X_AXIS] += -5*motion_3d_model.xyz_home_dir[(INT)X_AXIS];
    m600_target[Y_AXIS] += -5*motion_3d_model.xyz_home_dir[(INT)Y_AXIS];
    gcode::process_buffer_line_normal(m600_target, xy_homing_feedrate/60);// float
    m600_xy_homing(150);
  }

  static void m600_init_current_position(void)
  {
    for(int8_t i=0; i < motion_3d.axis_num; ++i)
    {
      // 执行中途换料，需要执行暂停操作
      m600_target[i]=gcode::get_current_position(i);
      m600_lastpos[i]=gcode::get_current_position(i);
    }
  }

  static void m600_z_60_down(void)
  {
    float z_feedrate = gcode::homing_feedrate[Z_AXIS]; // float
    sg_grbl::planner_set_position(m600_target);
    m600_target[Z_AXIS]+=60.0F; // float
    if(m600_target[Z_AXIS] > motion_3d_model.xyz_max_pos[Z_AXIS]) //防止超过最大位置
    {
      m600_target[Z_AXIS] = motion_3d_model.xyz_max_pos[Z_AXIS];
    }
    //限制z轴移动速度为 350，减小噪声； 2017512增加速度350；可以在300~350之间取值，大了会导致下降噪声大 add by john
    if(motion_3d_model.xyz_max_pos[Z_AXIS] > 350)//z轴大于350说明是大机器，小机器不用控制速度201716
    {
      z_feedrate = 350;
    }
    gcode::process_buffer_line_normal(m600_target, z_feedrate/60.0F); // float
  }


  static void m600_unloadfilament_second(void)
  {
    // add by suzhiwei 20160825: 使用混色新喷头电机组 E电机喷嘴距离159.7mm,B电机喷嘴距离116.7mm，E电机比B多退43mm
    if(t_sys_data_current.enable_color_mixing)
    {
      //第二步(1) 退料 消除E\B 电机高度差异
      m600_target[E_AXIS]+= FILAMENTCHANGE_PREPARE_RETRACT;// float
      m600_target[B_AXIS]+= FILAMENTCHANGE_PREPARE_RETRACT;// float
      gcode::process_buffer_line_normal(m600_target, 8400/60);
    }

    //第二步(2)，退料 FILAMENTCHANGE_FIRSTRETRACT
    //retract by E
    if(gcode::parseGcodeBufHandle.codeSeen('E'))
    {
      m600_target[E_AXIS]+= gcode::parseGcodeBufHandle.codeValue(); // float
    }
    else
    {
#ifdef FILAMENTCHANGE_FIRSTRETRACT
      m600_target[E_AXIS]+= FILAMENTCHANGE_FIRSTRETRACT; // float
#endif
    }
    //retract by B
    if(t_sys_data_current.enable_color_mixing)
    {
      if(gcode::parseGcodeBufHandle.codeSeen('B'))
      {
        m600_target[B_AXIS]+= gcode::parseGcodeBufHandle.codeValue(); // float
      }
      else
      {
#ifdef FILAMENTCHANGE_FIRSTRETRACT
        m600_target[B_AXIS]+= FILAMENTCHANGE_FIRSTRETRACT; // float
#endif // FILAMENTCHANGE_FIRSTRETRACT
      }
    }
    gcode::process_buffer_line_normal(m600_target, (((t_sys_data_current.enable_color_mixing)?(6000/60):(8400/60))));
  }

  static void m600_unloadfilament_third(void)
  {
    //第三步，退料 FILAMENTCHANGE_FINALRETRACT
    if(gcode::parseGcodeBufHandle.codeSeen('L'))
    {
      m600_target[E_AXIS]+= gcode::parseGcodeBufHandle.codeValue(); // float
      if(t_sys_data_current.enable_color_mixing)
      {
        m600_target[B_AXIS]+= gcode::parseGcodeBufHandle.codeValue(); // float
      }
    }
    else
    {
#ifdef FILAMENTCHANGE_FINALRETRACT
      if(t_sys_data_current.enable_color_mixing)
      {
        m600_target[E_AXIS]+= FILAMENTCHANGE_FINALRETRACT - 10 ; // float
        m600_target[B_AXIS]+= FILAMENTCHANGE_FINALRETRACT - 10 ; // float
      }
      else
      {
        m600_target[E_AXIS]+= FILAMENTCHANGE_FINALRETRACT - 20 ; // float
      }
#endif
    }
    gcode::process_buffer_line_normal(m600_target, 300/60);
  }

  /**
  * @brief 移动XY回零点，并退料
  * @param target：指向现时的XYZ轴位置
  * @retval None
  * @Date  2017/03/09
  * @Create add by suzhiwei 20160825
  */
  static void m600_unloadfilament(void)
  {
    //第一步 先进料3cm，让料融化
    m600_target[E_AXIS]+= (t_sys_data_current.enable_color_mixing?100:30) ;//160:30
    m600_target[B_AXIS]+= (t_sys_data_current.enable_color_mixing?100:30) ;//160:30
    gcode::process_buffer_line_normal(m600_target, 250/60);
    //第二步
    m600_unloadfilament_second();
    //第三步
    m600_unloadfilament_third();
  }

  /**
  * @brief 退料完成，声音提示
  * @param None
  * @retval None
  * @Date  2017/03/09
  */
  static void m600_beep_notice(void)
  {
    static UINT8 LCDBuzzCount=5;
    //串口上传信息到上位机2017.7.6
    USER_EchoLogStr("M600_notice\r\n");
    while(!is_confirm_load_filament)
    {
      if(LCDBuzzCount>0)
      {
        buzz(100);
        LCDBuzzCount--;
      }
      (void)sys_os_delay(100);
    }
    LCDBuzzCount=5;
  }


  static void m600_loadfilament_fast(void)
  {
    //第二步：快速进料 使料到达加热管
    if(gcode::parseGcodeBufHandle.codeSeen('L'))
    {
      m600_target[E_AXIS]+= -gcode::parseGcodeBufHandle.codeValue(); // float
      if(t_sys_data_current.enable_color_mixing)
      {
        m600_target[B_AXIS]+= -gcode::parseGcodeBufHandle.codeValue(); // float
      }
    }
    else
    {
      m600_target[E_AXIS]+=FIRST_LOAD_MM; // float
      if(t_sys_data_current.enable_color_mixing)
      {
        m600_target[B_AXIS]+=FIRST_LOAD_MM; // float
      }
    }
    //auto filament feeding 2014-12-31//should do nothing
    gcode::process_buffer_line_normal(m600_target, (t_sys_data_current.enable_color_mixing?330:300)/60); //2017.4.22保持原样，还没进到喷嘴，拉力够大
  }

  static void m600_loadfilament_slow(void)
  {
    //第三步：慢速进料 FILAMENTCHANGE_FIRSTRETRACT
    if(gcode::parseGcodeBufHandle.codeSeen('L'))
    {
      m600_target[E_AXIS]+= -gcode::parseGcodeBufHandle.codeValue(); // float
      if(t_sys_data_current.enable_color_mixing)
      {
        m600_target[B_AXIS]+= -gcode::parseGcodeBufHandle.codeValue(); // float
      }
    }
    else
    {
      m600_target[E_AXIS]+=(FINAL_LOAD_MM-90);//较少E电机出丝量20170525
      if(t_sys_data_current.enable_color_mixing)
      {
        m600_target[B_AXIS]+=FINAL_LOAD_MM; // float
      }
    }
    gcode::process_buffer_line_normal(m600_target,(t_sys_data_current.enable_color_mixing?140: 140)/60);  //auto filament feeding 2014-12-31should do nothing//2017.4.22更改为140：140，增大挤出头拉力
  }

  /**
    * @brief 使X,Y,Z轴返回打印位置
    * @param target：指向现时的XYZE电机的位置
  		   lastpos：指向打印时的各电机位置
    * @retval None
    * @Date  2017/03/09
    */
  static void m600_return_normal(void)
  {
    FLOAT feedrate = 6000;
// add by suzhiwei 20160825: 使用混色新喷头电机组 E电机喷嘴距离159.7mm,B电机喷嘴距离116.7mm，E电机比B多退43mm
    if(t_sys_data_current.enable_color_mixing)
    {
      //第一步：进料 消除E\B 电机高度差异
      m600_target[E_AXIS]+= ((-1)*FILAMENTCHANGE_PREPARE_RETRACT) + EB_DIFF_MM;//多进43
      m600_target[B_AXIS]+= (-1)*FILAMENTCHANGE_PREPARE_RETRACT;
      gcode::process_buffer_line_normal(m600_target, 300/60);
    }
    //第二步：快速进料 使料到达加热管
    m600_loadfilament_fast();
    //第三步：慢速进料 FILAMENTCHANGE_FIRSTRETRACT
    m600_loadfilament_slow();

    m600_xy_home_process();//返回零点，防止换料时移动挤出头
    //第四步：还原xy
    m600_target[X_AXIS] = m600_lastpos[X_AXIS];
    m600_target[Y_AXIS] = m600_lastpos[Y_AXIS];
    gcode::process_buffer_line_normal(m600_target, 20.0F);//(FLOAT)feedrate*1.5F/60.0F); //move xy back
    // 第五步：还原Z
    // 平台下降到最低位置，限位开关关闭，平台上升，限位开关开启，避免平台上不去
    sg_grbl::st_enable_endstops(false);
    m600_target[Z_AXIS] = m600_lastpos[Z_AXIS];
    gcode::process_buffer_line_normal(m600_target, (feedrate*1.5F)/60.0F); //move z back
  }


  /**
      * @brief 实现整个中途换料功能
      * @param _current_position[MAX_NUM_AXIS]：所有坐标轴的当前坐标
      * @retval None
      * @Date  2017/03/16
      */
  void process_command_M_600(void)
  {
    m600_init_current_position();
    USER_EchoLogStr("M600 ok!\r\n");
    sg_grbl::st_enable_endstops(true);
		
    m600_filament_change_status = 1;
    m600_z_60_down();//Z轴向下移动60

    m600_filament_change_status = 2;
    m600_xy_home_process();//XY归零
    m600_unloadfilament();//退丝

    m600_filament_change_status = 3;
    m600_beep_notice();//退丝完成提醒

    m600_filament_change_status = 4;
    m600_return_normal();

    // finish
    m600_filament_change_status = 5;

    m600_lastpos[E_AXIS]-=1.5f;//改善中途换料断层现象
    sg_grbl::planner_set_position(m600_lastpos); // Fixed by suzhiwei 20160823, bug: 修复中途换料对接不上

    USER_EchoLogStr("ChangeFilament finish!\r\n");//串口上传信息到上位机2017.7.6
    m600_is_midway_change_material = false; // 重置中途换料状态
    is_confirm_load_filament = false;  // 重置中途换料确认进丝按钮
  }




  void m600_process(void)
  {
    static INT16 mid_way_chg_mat_print_status = 0; /*!< 中途换料，打印状态：0为没打印状态，1打印状态，2为暂停状态 */
    //正常下的中途换料
    sg_grbl::st_synchronize();
    sg_grbl::plan_set_process_auto_bed_level_status(false);
    if(parseGcodeBufHandle.codeSeen('S'))
    {
      mid_way_chg_mat_print_status = static_cast<INT16>(parseGcodeBufHandle.codeValueLong()) ;
    }
    SetPrintStatus(false);  //此处设置暂停 跟M14R03的门检测功能有关 当换料的时候，门打开依然可以弹出提示
    process_command_M_600();
    if(1 == mid_way_chg_mat_print_status)
    {
      SetPrintStatus(true);
    }
    else if(2 == mid_way_chg_mat_print_status)
    {
      SetPausePrintingStatus(true);
    }
    sg_grbl::plan_set_process_auto_bed_level_status(true);
  }

  void m601_process(void)
  {
    if(t_gui.target_nozzle_temp < 150)
    {
      respond_gui_send_sem(PauseToResumeNozzleTemp);
      while((sg_grbl::temperature_get_extruder_current(active_extruder)) < (t_gui.target_nozzle_temp-5))//等待加热完成
      {
        sys_os_delay(50U);
      }
    }
    //断料续打时的换料
    process_command_M_600();
  }

  void m602_process(void)
  {
    gcodeCMD_m602_bed_level_interface(active_extruder);
  }
}








