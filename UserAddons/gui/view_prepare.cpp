#include "view_common.h"
#include "view_commonf.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "Alter.h"
#ifdef __cplusplus
extern "C" {
#endif
extern bool IsProcessStopPrint(void);

void DisableStepInfo(void)
{
  if (gui_is_refresh())
  {
    display_picture(63);
  }

  if (touchxy(160, 200, 320, 300)) //确定键
  {
    gui_set_curr_display(prepareF);
    return ;
  }
}
//#define Servo_test
#ifdef Servo_test
/*
 *@name:servo_device
 *@brief:使用定时器5的计数器进行延时，实现舵机信号产生，使用到的引脚是B电机的EN、STEP、DIR都可以
 *@para: anglebuf舵机要移动到的角度位置，绝对位置
 *@date: 2017/6/7
 *@version:V1.0
 *@author:john
*/
//#include "pins.h"
extern TIM_HandleTypeDef htim5;
void servo_device(UINT16 anglebuf)
{
  anglebuf = anglebuf * 2000 / 180 + 500;
  UINT8 count = 35;
  __HAL_TIM_SET_AUTORELOAD(&htim5, 19999);
  HAL_TIM_Base_Start(&htim5);
  sys_task_enter_critical();
  HAL_GPIO_WritePin(E1_STEP_PIN_GPIO, E1_ENABLE_PIN, GPIO_PIN_RESET);//

  while (--count)
  {
    htim5.Instance->CNT = 0;
    HAL_GPIO_WritePin(E1_STEP_PIN_GPIO, E1_ENABLE_PIN, GPIO_PIN_SET);

    while (htim5.Instance->CNT < anglebuf);

    HAL_GPIO_WritePin(E1_STEP_PIN_GPIO, E1_ENABLE_PIN, GPIO_PIN_RESET);

    while (htim5.Instance->CNT < 19998);
  }

  sys_task_exit_critical();
}
#else
void servo_device(UINT16 anglebuf)
{
  anglebuf = anglebuf;
}
#endif
void prepareF(void)
{
  if (gui_is_refresh())
  {
#if LASER_MODE
    Gui_Mode = MODE_PREPARE;
#endif
    servo_device(120);

    // 日本定制固件，不显示预热pla、abs，只显示预热热床
    if ((5 == t_sys_data_current.custom_model_id) && (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id))
    {
      display_picture(99);
    }
    else
    {
      if (t_custom_services.disable_abs) //不能打印ABS
      {
        display_picture(44);
      }
      else
      {
        if ((7 == t_sys_data_current.custom_model_id) && (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id))
        {
          display_picture(106);
        }
        else
        {
          display_picture(4);
        }
      }
    }
  }

  if (touchxy(30, 0, 150, 65))
  {
    servo_device(10);
    gui_set_curr_display(maindisplayF);
    return ;
  }

  if (touchxy(18, 115, 113, 187))
  {
    //    gui_send_sem_home();
    goto_page_homing();
    respond_gui_send_sem(BackZeroValue);
    return ;
  }

  if (!IsProcessStopPrint() && touchxy(133, 115, 229, 187))
  {
#if LASER_MODE

    if (t_sys_data_current.IsLaser)
    {
      gui_set_curr_display(Laser_nonsupport);
      return;
    }

#endif
    gui_set_curr_display(loadfilament0F);
    goto_page_homing();
    respond_gui_send_sem(FeedFilamentValue);
    return ;
  }

  if (!IsProcessStopPrint() && touchxy(249, 115, 343, 187))
  {
#if LASER_MODE

    if (t_sys_data_current.IsLaser)
    {
      gui_set_curr_display(Laser_nonsupport);
      return;
    }

#endif
    gui_set_curr_display(unloadfilament0F);
    goto_page_homing();
    respond_gui_send_sem(BackFilamentValue);
    return ;
  }

  if (touchxy(364, 115, 459, 187))
  {
    gui_set_curr_display(MoveXYZ);
    return ;
  }

  if (touchxy(18, 218, 113, 290))
  {
    respond_gui_send_sem(DisableStepValue);
    gui_set_curr_display(DisableStepInfo);
    return ;
  }

  if ((5 == t_sys_data_current.custom_model_id) && (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id))
  {
    if (touchxy(133, 218, 229, 290))
    {
      gui_set_curr_display(maindisplayF);
      respond_gui_send_sem(PreHeatBedValue);
      return ;
    }

    if (touchxy(249, 218, 343, 290))
    {
      gui_set_curr_display(maindisplayF);
      respond_gui_send_sem(CoolDownValue);
      return ;
    }
  }
  else
  {
    if (touchxy(133, 218, 229, 290))
    {
#if LASER_MODE

      if (t_sys_data_current.IsLaser)
      {
        gui_set_curr_display(Laser_nonsupport);
        return;
      }

#endif
      gui_set_curr_display(maindisplayF);
      respond_gui_send_sem(PreHeatPLAValue);
      return ;
    }

    if (t_custom_services.disable_abs) //不能打印ABS,无预热ABS按键
    {
      if (touchxy(249, 218, 343, 290))
      {
#if LASER_MODE

        if (t_sys_data_current.IsLaser)
        {
          gui_set_curr_display(Laser_nonsupport);
          return;
        }

#endif
        gui_set_curr_display(maindisplayF);
        respond_gui_send_sem(CoolDownValue);
        return ;
      }
    }
    else
    {
      if (touchxy(249, 218, 343, 290))
      {
#if LASER_MODE

        if (t_sys_data_current.IsLaser)
        {
          gui_set_curr_display(Laser_nonsupport);
          return;
        }

#endif
        gui_set_curr_display(maindisplayF);
        respond_gui_send_sem(PreHeatABSValue);
        return ;
      }

      if (touchxy(364, 218, 459, 290))
      {
#if LASER_MODE

        if (t_sys_data_current.IsLaser)
        {
          gui_set_curr_display(Laser_nonsupport);
          return;
        }

#endif
        gui_set_curr_display(maindisplayF);
        respond_gui_send_sem(CoolDownValue);
        return ;
      }
    }
  }
}
#ifdef __cplusplus
}//extern "C" {
#endif


