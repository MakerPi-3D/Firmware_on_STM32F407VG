#include "gui_task.h"
#include "config_model_tables.h"
#include "uart.h"
#include "lcd.h"
#include "touch.h"
#include "view_pic_display.h"
#include "globalvariables.h"
#include "PrintControl.h"
#include "lcd_common.h"
#include "interface.h"
#include "Alter.h"
#include "user_interface.h"
#include "view_commonf.h"
#ifdef __cplusplus
extern "C" {
#endif


  INT WarningInfoFlag=1;
  void ErrInterfaceControl(void)
  {
    if(t_gui_p.IsWarning)  //是否显示错误界面
    {
      if(WarningInfoFlag)
      {
        WarningInfoFlag=0;
        gui_set_curr_display(WarningInfoF);
      }
    }
  }

  void transfileF(void)
  {
    if(gui_is_refresh())
    {
      display_picture(25);
    }
  }

  void TransFileInterfaceControl(void)
  {
    static INT transflag=1;
    static INT transprintflag=1;
    static INT transpause=0;
    if(t_gui_p.IsTransFile)  //是否显示传输界面
    {
      if(transflag)
      {
        transflag=0;
        transpause=1;
        transprintflag=1;
        //gui_set_curr_display(transfileF);
      }
    }
    else if(IsPrintSDFile()) //是否打印上传的文件
    {
      if(transprintflag)
      {
        transpause=0;
        transflag=1;
        transprintflag=0;
/*
        strcpy(printname,SDFileName);
        respond_gui_send_sem(FilePrintValue);
        pauseprint=0;
        print_flage = 1;
        gui_set_curr_display(maindisplayF);
*/
      }
    }
    else  //显示主界面
    {
      if(transpause)
      {
        transpause=0;
        transprintflag=0;
        transflag=1;
        pauseprint=0;
        print_flage = 0;
        gui_set_curr_display(maindisplayF);
      }
    }
  }

  void gui_first_display(void)
  {
    static bool is_first_display = true;
    if(is_first_display)
    {
      /**开机LOGO界面************************************************************/
      if(t_sys_data_current.logo_id)
      {
        DisplayLogoPicture(t_sys_data_current.logo_id);
        Set_BL(100); // 触摸屏使能引脚PB5
        (void)sys_os_delay(2000);//延时，显示logo图标一段时间
      }
      gui_set_curr_display(maindisplayF);
      Set_BL(100);
#if 0
      //品质测试用的固件20170825
      extern void board_test_display_function(void);//品质测试时上电直接进入测试界面
      motion_3d.enable_board_test = 1;
      gui_set_curr_display(board_test_display_function);
#endif
      sys_os_delay(50);
      currentdisplay();
      is_first_display = false;
    }
  }

  void guimain(void)
  {
    gui_first_display();
    if(t_sys_data_current.model_id == K5)
    {
      BL_Pro();
    }
    ErrInterfaceControl(); //错误界面显示控制
    TransFileInterfaceControl(); //传输界面控制
    DoorOpenWarningInfo_NotPrinting(); //M14R03,M14S 门打开高温提示
    (void)sys_os_delay(50);
    gui_display_touch_ctrl();	//触摸反应和界面数据刷新
  }


  void gui_task_loop(void)
  {
    process_serial_command();
    (void)sys_os_delay(50);
    guimain();   //GUI显示
    (void)sys_os_delay(50);
  }



#ifdef __cplusplus
} //extern "C" {
#endif



