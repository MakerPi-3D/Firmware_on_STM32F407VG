#include "view_common.h"
#include "view_commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "user_debug.h"
#include "interface.h"
//#include "ConfigurationStore.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "threed_engine.h"
#include "user_interface.h"
#include "Alter.h"


#ifdef __cplusplus
extern "C" {
#endif

//#ifdef ENABLE_AUTO_BED_LEVELING
//extern void gui_bed_level_nozzle_heat(void);
extern void DGUS_bed_level_first(void);
extern bool is_enable_bed_level(void);
//#endif // #ifdef ENABLE_AUTO_BED_LEVELING
void gui_adjust_z_zero_high(void);

void SettingInterface5(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    //    USER_EchoLogStr("I5\r\n");//串口上传信息到上位机2017.7.6
    display_picture(13);//按键、警报、触摸校准

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(123 + 5, 146 + 5, 123 + 5 + 20, 146 + 5 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(264 + 5, 147 + 5, 264 + 5 + 20, 147 + 5 + 12);
    }
  }

  if (touchxy(48, 145, 154, 228))
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(123 + 5, 146 + 5, 123 + 5 + 20, 146 + 5 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(188, 145, 295, 228))
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(264 + 5, 147 + 5, 264 + 5 + 20, 147 + 5 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(327, 145, 433, 228))
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }
}
/*2、3界面有断料校准，以前用红外模块时用到，现在的模块不用校准20171024*/
/*
void SettingInterface2(void)
{
  if(gui_is_refresh())
  {
    USER_EchoLogStr("I2\r\n");//串口上传信息到上位机2017.7.6
    display_picture(46);//按键、警报、屏幕校准、z轴校准

    if(t_sys.key_sound)
    {
      LCD_Fill_Default(91+3,113+3,91+3+20,113+3+12);
    }
    if(t_sys.alarm_sound)
    {
      LCD_Fill_Default(203+3,113+3,203+3+20,113+3+12);
    }
  }
  if(touchxy(22,113,118,186)) //按键声音设置键
  {
    if(t_sys.key_sound)
    {
      t_sys.key_sound=0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound=1;
      LCD_Fill_Default(91+3,113+3,91+3+20,113+3+12);
      (void)sys_os_delay(500);
    }
  }
  if(touchxy(135,113,230,186))  //报警声音设置键
  {
    if(t_sys.alarm_sound)
    {
      t_sys.alarm_sound=0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound=1;
      LCD_Fill_Default(203+3,113+3,203+3+20,113+3+12);
      (void)sys_os_delay(500);
    }
  }
  if(touchxy(248,113,345,186)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }
  if(touchxy(365,113,461,186)) //Z行程测量键
  {
    gui_set_curr_display(gui_view::CalculatingZMaxLimit);

    SettingInfoToSYS.GUISempValue=CalculateZMaxPos;
    GUISendSempToSYS();
    return ;
  }

//  if(touchxy(22,219,118,294)) //断料校准键
//  {
//    gui_set_curr_display(MatCheckCalibrateReady);
//    return ;
//  }
}

void SettingInterface3(void)
{
  if(gui_is_refresh())
  {
    USER_EchoLogStr("I3\r\n");//串口上传信息到上位机2017.7.6
    display_picture(13);//按键、警报、触摸校准

    if(t_sys.key_sound)
    {
      LCD_Fill_Default(91+3,152+3,91+3+20,152+3+12);
    }
    if(t_sys.alarm_sound)
    {
      LCD_Fill_Default(203+3,152+3,203+3+20,152+3+12);
    }
  }
  if(touchxy(22,152,118,226)) //按键声音设置键
  {
    if(t_sys.key_sound)
    {
      t_sys.key_sound=0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound=1;
      LCD_Fill_Default(91+3,152+3,91+3+20,152+3+12);
      (void)sys_os_delay(500);
    }
  }
  if(touchxy(135,152,230,226))  //报警声音设置键
  {
    if(t_sys.alarm_sound)
    {
      t_sys.alarm_sound=0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound=1;
      LCD_Fill_Default(203+3,152+3,203+3+20,152+3+12);
      (void)sys_os_delay(500);
    }
  }
  if(touchxy(248,152,345,226)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }
//  if(touchxy(365,152,461,226)) //断料校准键
//  {
//    gui_set_curr_display(MatCheckCalibrateReady);
//    return ;
//  }
}
*/
void SettingInterface4(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    //    USER_EchoLogStr("I4\r\n");//串口上传信息到上位机2017.7.6
    display_picture(46);//按键、警报、屏幕校准、z轴校准

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(91 + 3, 152 + 3, 91 + 3 + 20, 152 + 3 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(203 + 3, 152 + 3, 203 + 3 + 20, 152 + 3 + 12);
    }
  }

  if (touchxy(22, 152, 118, 226)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(91 + 3, 152 + 3, 91 + 3 + 20, 152 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(135, 152, 230, 226)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(203 + 3, 152 + 3, 203 + 3 + 20, 152 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(248, 152, 345, 226)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }

  if (touchxy(365, 152, 461, 226)) //测量行程键
  {
    gui_set_curr_display(gui_view::CalculatingZMaxLimit);
    respond_gui_send_sem(CalculateZMaxPos);
    return ;
  }
}
//机器配置
void SettingInterface1(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    USER_EchoLogStr("I1\r\n");//串口上传信息到上位机2017.7.6
    display_picture(30);//按键、警报、屏幕校准、机器配置

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(91 + 3, 152 + 3, 91 + 3 + 20, 152 + 3 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(203 + 3, 152 + 3, 203 + 3 + 20, 152 + 3 + 12);
    }
  }

  if (touchxy(22, 152, 118, 226)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(91 + 3, 152 + 3, 91 + 3 + 20, 152 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(135, 152, 230, 226)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(203 + 3, 152 + 3, 203 + 3 + 20, 152 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(248, 152, 345, 226)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }

  if (touchxy(365, 152, 461, 226)) //机器配置键
  {
    gui_set_curr_display(MachineSetting);
    return ;
  }
}

void SettingInterface8(void);
void SettingInterface9(void);

void SettingInterface9(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    display_picture(93);//按键、警报、触摸校准、断电、自动调平、堵料关

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(91 + 3, 113 + 3, 91 + 3 + 20, 113 + 3 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(203 + 3, 113 + 3, 203 + 3 + 20, 113 + 3 + 12);
    }
  }

  if (touchxy(22, 113, 118, 186)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(91 + 3, 113 + 3, 91 + 3 + 20, 113 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(135, 113, 230, 186)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(203 + 3, 113 + 3, 203 + 3 + 20, 113 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(248, 113, 345, 186)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }

  if (touchxy(365, 113, 461, 186)) //Z行程测量键
  {
    gui_set_curr_display(gui_view::CalculatingZMaxLimit);
    respond_gui_send_sem(CalculateZMaxPos);
    return ;
  }

  if (touchxy(22, 219, 118, 294)) //计算平台
  {
    gui_set_curr_display(DGUS_bed_level_first);
    return;
  }

  if (touchxy(135, 219, 230, 294)) //堵料开关
  {
    gui_set_curr_display(SettingInterface8);
    t_sys_data_current.enable_block_detect = 1;
    return ;
  }
}

void SettingInterface8(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    USER_EchoLogStr("I8\r\n");//串口上传信息到上位机2017.7.6
    display_picture(92);//按键、警报、触摸校准、断电、自动调平、堵料开

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(91 + 3, 113 + 3, 91 + 3 + 20, 113 + 3 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(203 + 3, 113 + 3, 203 + 3 + 20, 113 + 3 + 12);
    }
  }

  if (touchxy(22, 113, 118, 186)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(91 + 3, 113 + 3, 91 + 3 + 20, 113 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(135, 113, 230, 186)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(203 + 3, 113 + 3, 203 + 3 + 20, 113 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(248, 113, 345, 186)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }

  if (touchxy(365, 113, 461, 186)) //Z行程测量键
  {
    gui_set_curr_display(gui_view::CalculatingZMaxLimit);
    respond_gui_send_sem(CalculateZMaxPos);
    return ;
  }

  if (touchxy(22, 219, 118, 294)) //计算平台
  {
    gui_set_curr_display(DGUS_bed_level_first);
    return ;
  }

  if (touchxy(135, 219, 230, 294)) //堵料开关
  {
    gui_set_curr_display(SettingInterface9);
    t_sys_data_current.enable_block_detect = 0;
    return ;
  }
}

void SettingInterface6(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    USER_EchoLogStr("I6\r\n");//串口上传信息到上位机2017.7.6
    display_picture(80);//按键、警报、触摸校准、断电、校准平台

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(91 + 3, 113 + 3, 91 + 3 + 20, 113 + 3 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(203 + 3, 113 + 3, 203 + 3 + 20, 113 + 3 + 12);
    }

#if LASER_MODE

    if (t_sys_data_current.IsLaser)
    {
      LCD_Fill_Default(203 + 3, 225 + 3, 203 + 3 + 20, 225 + 3 + 12);
    }

#endif
  }

  if (touchxy(22, 113, 118, 186)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(91 + 3, 113 + 3, 91 + 3 + 20, 113 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(135, 113, 230, 186)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(203 + 3, 113 + 3, 203 + 3 + 20, 113 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(248, 113, 345, 186)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }

  if (touchxy(365, 113, 461, 186)) //Z行程测量键
  {
#if LASER_MODE

    if (t_sys_data_current.IsLaser)
    {
      gui_set_curr_display(Laser_nonsupport);
      return;
    }

#endif

    if (t_sys_data_current.IsMechanismLevel && (2 == t_sys_data_current.enable_bed_level || 1 == t_sys_data_current.enable_bed_level))
    {
      gui_set_curr_display(gui_adjust_z_zero_high);
      goto_page_homing();
      respond_gui_send_sem(BackZeroValue);
    }
    else
    {
      gui_set_curr_display(gui_view::CalculatingZMaxLimit);
      respond_gui_send_sem(CalculateZMaxPos);
    }

    return ;
  }

  if (touchxy(22, 219, 118, 294)) //计算平台
  {
#if LASER_MODE

    if (t_sys_data_current.IsLaser)
    {
      gui_set_curr_display(Laser_nonsupport);
      return;
    }

#endif

    if (2 == t_sys_data_current.enable_bed_level)
    {
      respond_gui_send_sem(StartCalBedLevel);
    }
    else
    {
      gui_set_curr_display(DGUS_bed_level_first);
    }

    return ;
  }

#if LASER_MODE

  if (t_sys_data_current.IsLaserMode && touchxy(150, 220, 230, 290)) //更换激光头
  {
    gui_set_curr_display(Laser_change_warn);
  }

#endif

  if (touchxy(248, 217, 345, 306) && t_sys_data_current.ui_number == 2)
  {
    Gui_Mode = MODE_SET_LAN;
    gui_set_curr_display(PictureSetting);
  }
}

void SettingInterface7(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_SET;
#endif
    USER_EchoLogStr("I7\r\n");//串口上传信息到上位机2017.7.6
    display_picture(81);//按键、警报、触摸校准、自动调平

    if (t_sys.key_sound)
    {
      LCD_Fill_Default(91 + 3, 152 + 3, 91 + 3 + 20, 152 + 3 + 12);
    }

    if (t_sys.alarm_sound)
    {
      LCD_Fill_Default(203 + 3, 152 + 3, 203 + 3 + 20, 152 + 3 + 12);
    }
  }

  if (touchxy(22, 152, 118, 226)) //按键声音设置键
  {
    if (t_sys.key_sound)
    {
      t_sys.key_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.key_sound = 1;
      LCD_Fill_Default(91 + 3, 152 + 3, 91 + 3 + 20, 152 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(135, 152, 230, 226)) //报警声音设置键
  {
    if (t_sys.alarm_sound)
    {
      t_sys.alarm_sound = 0;
      gui_set_curr_display(settingF);
      return ;
    }
    else
    {
      t_sys.alarm_sound = 1;
      LCD_Fill_Default(203 + 3, 152 + 3, 203 + 3 + 20, 152 + 3 + 12);
      (void)sys_os_delay(500);
    }
  }

  if (touchxy(248, 152, 345, 226)) //触摸校正键
  {
    tp_dev.adjust();
    gui_set_curr_display(settingF);
    return ;
  }

  if (touchxy(365, 152, 461, 226)) //计算平台
  {
    gui_set_curr_display(DGUS_bed_level_first);
    return ;
  }
}

void settingF(void)
{
  if (!t_sys_data_current.have_set_machine) //还没设置机器
  {
    SettingInterface1(); //按键、警报、触摸校准、设置机型
  }
  //  else if((!motion_3d.disable_z_max_limit) && (t_sys_data_current.enable_material_check))  //已设置机器，且Z轴有下限位开关, 且有断料检测功能
  //  {
  //    SettingInterface2(); //显示没有机器配置选项，但有测量行程选项、有断料校准的界面
  //  }
  //  else if(t_sys_data_current.enable_material_check)  //已设置机器，且有断料检测功能
  //  {
  //    SettingInterface3(); //按键、警报、触摸校准
  //  }
  else if (!motion_3d.disable_z_max_limit) //已设置机器，且Z轴有下限位开关
  {
    if (1 == t_sys_data_current.enable_bed_level)
    {
      if (t_sys_data_current.enable_block_detect)
      {
        SettingInterface8();//按键、警报、触摸校准、断电、自动调平、堵料开
      }
      else
        SettingInterface6(); //按键、警报、触摸校准、断电、校准平台
    }
    else if (2 == t_sys_data_current.enable_bed_level)
    {
      if (t_sys_data_current.enable_block_detect)
      {
        SettingInterface8();//按键、警报、触摸校准、断电、自动调平、堵料开
      }
      else
        SettingInterface6(); //按键、警报、触摸校准、断电、校准平台
    }
    else
      SettingInterface4(); //显示没有机器配置选项，但有测量行程选项的界面
  }
  else if (1 == t_sys_data_current.enable_bed_level) //已设置机器，且Z轴有下限位开关
  {
    SettingInterface7(); //按键、警报、触摸校准、自动调平
  }
  else //已设置机器，且Z轴没有下限位开关、没有断料检测功能
  {
    SettingInterface5(); //按键、警报、触摸校准
  }

  if (touchxy(0, 0, 150, 70)) //返回键
  {
    SaveBezzerSound();
    gui_set_curr_display(maindisplayF);
    return ;
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

