#include "Alter.h"
#include "config_model_tables.h"
#include "stm32f4xx_hal.h"
#include "sysconfig_data.h"
#include "user_debug.h"
#include "globalvariables.h"
#include "PrintControl.h"
#include "temperature.h"
#include "user_debug.h"
#include "view_common.h"
#include "view_commonf.h"
#include "lcd.h"
#include "infrared_bed_level_adjust.h"
#include "planner.h"
#include "math.h"
#include "lcd_common.h"
#include "user_interface.h"
#include "config_motion_3d.h"
#include "math.h"
#include "config_motion_3d.h"
#include "stepper.h"
#include "interface.h"
#include "machinecustom.h"
#include "cmsis_os.h"
#include "mechanical_bed_level_adjust.h"
#include "gcode_global_params.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SSD_CMD_PLL  0xE4
#define SSD_CMD_FRE  0xE7
//修改M41G 5V_FAN做断料检测IO
  void Mat_Cut_Init(void)
  {
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }

  void Fan_5v_Init(void)
  {
    if(t_sys_data_current.model_id != M41G)
    {
      GPIO_InitTypeDef GPIO_InitStruct;
      GPIO_InitStruct.Pin = GPIO_PIN_14;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
  }

//K5屏幕休眠，花屏处理和放宽报警条件
  CHAR TempMinErr_cnt,poweroffrunonce;
  void Set_BL(USHORT per)   //百分比  设置值0-100
  {
    TIM3->CCR2=TIME_PER-per*10;
  }

  static UCHAR BL_Mark=0;
  static unsigned long lcdReInitTimeOut=0;//黑屏状态下每隔2分钟初始化屏幕
  //黑屏时间2分钟
  void BL_Pro(void)
  {
    static ULONG BLTimeOut=sys_task_get_tick_count() + 120000;

    if(t_sys_data_current.model_id != K5)
    {
      return;
    }
    if(touch.touch_flag)
    {
      if(BLTimeOut<sys_task_get_tick_count())
      {
        touch.touch_flag=0;
        BL_Mark=0;
      }
      BLTimeOut=sys_task_get_tick_count()+30000;
    }
    if((BLTimeOut<sys_task_get_tick_count())&&(!BL_Mark))
    {
      if(!infrared_bed_level_mark()&&!mechanical_bed_level_mark())
      {
        Set_BL(10);
      }
      BL_Mark=1;
      lcdReInitTimeOut = sys_task_get_tick_count()+1000*60*2;
    }
//    if(infrared_bed_level_mark() || mechanical_bed_level_mark())
    {
      While_Pro();  //白屏修复
    }
  }

  uint16_t Get_SSD_Value(uint16_t CMD)
  {
    LCD_WR_REG(CMD);
    return LCD_RD_DATA();
  }

  void While_Pro(void)
  {
    //白屏修复
    static unsigned char SSD_ON;
    static unsigned long lcdInitTimeOut=0;
    if((Get_SSD_Value(SSD_CMD_FRE) & 0x00FF) != 0x01)
    {
      SSD_ON = 1;
    }
    if((SSD_ON == 1 && lcdInitTimeOut < sys_task_get_tick_count()))
    {
      LCD_Init();
      gui_set_curr_display(currentdisplay);
      SSD_ON = 0;
      lcdInitTimeOut = sys_task_get_tick_count()+100;
    }
    else
    {
      if(BL_Mark == 1 && lcdReInitTimeOut < sys_task_get_tick_count())
      {
        LCD_Init();
        gui_set_curr_display(currentdisplay);
        lcdReInitTimeOut = sys_task_get_tick_count()+1000*60*5;
      }
    }
  }

  CHAR Check_TempMinErr(void)
  {
    static ULONG TempMinErr_TimeOut = 0;
    if(t_sys_data_current.model_id!=K5)
    {
      return 1;
    }
    ++TempMinErr_TimeOut;
    if(TempMinErr_TimeOut>=10)
    {
      ++TempMinErr_cnt;
      TempMinErr_TimeOut=0;
    }
    if(TempMinErr_cnt>2)
    {
      TempMinErr_cnt=0;
      return 1;
    }
    else
    {
      return 0;
    }
  }

  bool Zaxis_RunOnce;
  MODEValueTypeDef Gui_Mode = MODE_MIAN;
  float Laser_Print_Position = 150.0f;
#if LASER_MODE
//新增激光头功能
  extern UINT8 MinTempWarningPopSet;
  extern UINT8 HeatFailWarningPopSet;
  extern void SaveLaser(void);
  extern void xy_to_zero(void);
  void Laser_change_warn(void)
  {
    if(gui_is_refresh())
    {
      Gui_Mode = MODE_SET;
      if(!t_sys_data_current.IsLaser)
        display_picture(126);
      else
        display_picture(127);
    }

    if(touchxy(80,160,240,290))    //耗材已拔出
    {
      if(t_sys_data_current.IsLaser)
      {
//				change_mark = 1;
        t_sys_data_current.IsLaser = 0;
        xy_to_zero();
        t_gui_p.G28_ENDSTOPS_COMPLETE = 1U;
      }
      else
      {
        t_sys_data_current.IsLaser = 1;
        MinTempWarningPopSet=1;
        HeatFailWarningPopSet = 1;
        t_gui_p.G28_ENDSTOPS_COMPLETE = 0U;
        sys_send_gcode_cmd("G28 isInternal");
      }
      if(2 == t_sys_data_current.enable_v5_extruder)
        gcode::set_fan_speed(0);
      gpio_e_motor_fan_control(false);
      gui_set_curr_display(Laser_change_sure);
    }

    if(touchxy(260,160,410,290))    //耗材未拔出
    {
      if(!t_sys_data_current.IsLaser)
      {
        Gui_Mode = MODE_FILUM;
        gui_set_curr_display(unloadfilament0F);
        goto_page_homing();
        respond_gui_send_sem(BackFilamentValue);
      }
      else
      {
        gui_set_curr_display(settingF);
        sg_grbl::temperature_disable_heater();
      }
    }

    if(touchxy(30,0,180,80))    //耗材未拔出
    {
      gui_set_curr_display(settingF);
      if(!t_sys_data_current.IsLaser)
      {
        MinTempWarningPopSet=0;
        HeatFailWarningPopSet = 0;
      }
      else
      {
        MinTempWarningPopSet=1;
        HeatFailWarningPopSet = 1;
      }
    }


  }

  void Laser_change_sure(void)
  {
    if((1U == t_gui_p.G28_ENDSTOPS_COMPLETE) && sg_grbl::planner_moves_planned() == 0)
    {
      gcode::set_current_position(X_AXIS, 100.0f);
      gcode::set_current_position(Y_AXIS, 100.0f);
      gcode::process_buffer_line_normal_4_curr(gcode::homing_feedrate[X_AXIS]/60);
      if(gui_is_refresh())
      {
        if(t_sys_data_current.IsLaser)
          display_picture(124);
        else
          display_picture(123);
      }

      if(touchxy(80,170,240,300))     //确认
      {
//			change_mark = 0;
        xy_to_zero();

//			gcode::set_current_position(X_AXIS, motion_3d_model.xyz_home_pos[X_AXIS]);
//			gcode::set_current_position(Y_AXIS, motion_3d_model.xyz_home_pos[Y_AXIS]);
//			gcode::process_buffer_line_normal_4_curr(sg_grbl::homing_feedrate[X_AXIS]/60);

        gui_set_curr_display(settingF);
        if(!t_sys_data_current.IsLaser)
        {
          MinTempWarningPopSet=0;
          HeatFailWarningPopSet = 0;
        }
        else
        {
          t_gui.move_xyz_pos[Z_AXIS] = 150;
        }
        SaveLaser();
      }

      if(touchxy(260,170,400,300))    //取消
      {
//			change_mark = 0;
        gui_set_curr_display(Laser_change_warn);
        if(t_sys_data_current.IsLaser)
        {
          t_sys_data_current.IsLaser = 0;
        }
        else
        {
          t_sys_data_current.IsLaser = 1;
        }
      }

    }
  }

  void Laser_nonsupport(void)
  {
    if(gui_is_refresh())
    {
      display_picture(125);
    }
    if(touchxy(180,170,310,270))
    {
      switch(Gui_Mode)
      {
      case MODE_SET:
        gui_set_curr_display(settingF);
        break;

      case MODE_PREPARE:
        gui_set_curr_display(prepareF);
        break;

      default:
        break;
      }
    }
  }
#endif

  


#ifdef __cplusplus
} // extern "C" {
#endif




