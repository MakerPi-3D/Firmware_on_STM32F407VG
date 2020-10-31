#include "boardtest.h"
#include "machinecustom.h"
#include "globalvariables.h"
#include "functioncustom.h"
#include "autobedlevelinterface.h"
#include "stm32f4xx_hal.h"
#include "user_debug.h"
#include "controlfunction.h"
#include "planner.h"

#include  "interface.h"
#include "config_model_tables.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "sysconfig_data.h"
#include "user_interface.h"
#include "gcode_global_params.h"
#include "machine.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "view_commonf.h"              //包含界面函数
#include "view_common.h"
#include "temperature.h"

  extern UINT8 ModelSelect;

#define NoSelectModel 255

#ifdef __cplusplus
} //extern "C" {
#endif

BoardTest::BoardTest()
{
  isStepTest=0;
  isFanTest=0;
  isHeatTest=0;
  memset(nozzleHeatTimeStr,0,sizeof(nozzleHeatTimeStr));
  memset(bed50HeatTimeStr,0,sizeof(bed50HeatTimeStr));
  memset(bed70HeatTimeStr,0,sizeof(bed70HeatTimeStr));
  memset(bed115HeatTimeStr,0,sizeof(bed115HeatTimeStr));
  clockTime = 0;
  heatStatus =0;
  processOn = 1;
}

bool BoardTest::guiInterface(void)
{
  if(gui_is_refresh())
  {
    display_picture(78);
    displayInit();
    displayText();
    t_sys_data_current.have_set_machine = 1;
  }
  if(touchCheck())
  {
    return 1;
  }
  if(gui_is_rtc())
  {
    displayText();
  }
  return 1;
}

void BoardTest::displayInit(void)
{
  if(t_sys_data_current.model_id==AMP410W)
  {
    SetTextDisplayRange(344,20,12*3,24,&NozzleTempTextRange);
    SetTextDisplayRange(344,61,12*3,24,&HotBedTempTextRange);
    SetTextDisplayRange((344+(12*4)) + (t_sys.lcd_ssd1963_43_480_272?10:0),20,12*3,24,&NozzleTargetTempTextRange);
    SetTextDisplayRange((344+(12*4)) + (t_sys.lcd_ssd1963_43_480_272?10:0),61,12*3,24,&HotBedTargetTempTextRange);
  }
  else
  {
    SetTextDisplayRange(137,35,12*3,24,&NozzleTempTextRange);
    SetTextDisplayRange(345,35,12*3,24,&HotBedTempTextRange);
    SetTextDisplayRange((137+(12*4)) + (t_sys.lcd_ssd1963_43_480_272?10:0),35,12*3,24,&NozzleTargetTempTextRange);
    SetTextDisplayRange((345+(12*4)) + (t_sys.lcd_ssd1963_43_480_272?10:0),35,12*3,24,&HotBedTargetTempTextRange);
  }
  ReadTextDisplayRangeInfo(NozzleTempTextRange,NozzleTempTextRangeBuf);
  ReadTextDisplayRangeInfo(HotBedTempTextRange,HotBedTempTextRangeBuf);
  ReadTextDisplayRangeInfo(NozzleTargetTempTextRange,NozzleTargetTempTextRangeBuf);
  ReadTextDisplayRangeInfo(HotBedTargetTempTextRange,HotBedTargetTempTextRangeBuf);
}

void BoardTest::displayText(void)
{
  CHAR TextBuffer[20];
  //显示喷嘴温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d",(INT)t_gui.nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTempTextRange,NozzleTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTempTextRange,TextRangeBuf);
  //显示斜杠
  if(t_sys_data_current.model_id==AMP410W)
  {
    DisplayTextDefault ((PUCHAR)"/",(344+(12*3)) + (t_sys.lcd_ssd1963_43_480_272?5:0),20);
  }
  else
  {
    DisplayTextDefault ((PUCHAR)"/",(137+(12*3)) + (t_sys.lcd_ssd1963_43_480_272?5:0),35);
  }
  //显示喷嘴目标温度
  snprintf(TextBuffer, sizeof(TextBuffer), "%3d",(INT)t_gui.target_nozzle_temp);
  CopyTextDisplayRangeInfo(NozzleTargetTempTextRange,NozzleTargetTempTextRangeBuf, TextRangeBuf);
  DisplayTextInRangeDefault((PUCHAR)TextBuffer, NozzleTargetTempTextRange,TextRangeBuf);

  if(!t_custom_services.disable_hot_bed)
  {
    //显示热床温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d",(INT)t_gui.hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTempTextRange,HotBedTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTempTextRange,TextRangeBuf);
    //显示斜杠
    if(t_sys_data_current.model_id==AMP410W)
    {
      DisplayTextDefault ((PUCHAR)"/",(344+(12*3)) + (t_sys.lcd_ssd1963_43_480_272?5:0),61);
    }
    else
    {
      DisplayTextDefault ((PUCHAR)"/",(345+(12*3)) + (t_sys.lcd_ssd1963_43_480_272?5:0),35);
    }
    //显示热床目标温度
    snprintf(TextBuffer, sizeof(TextBuffer), "%3d",(INT)t_gui.target_hot_bed_temp);
    CopyTextDisplayRangeInfo(HotBedTargetTempTextRange,HotBedTargetTempTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)TextBuffer, HotBedTargetTempTextRange,TextRangeBuf);
  }
}
menufunc_t lcdtest_lastdisplay;//用于测试触摸计数时记住上一个界面
bool BoardTest::touchCheck(void)
{
  if (touchxy(352, 111, 460, 294))
  {
    if (t_gui_p.SDIsInsert)
    {
      respond_gui_send_sem(OpenSDCardValue);
      gui_set_curr_display(filescanF);
      return 1;
    }
    else
    {
      gui_set_curr_display(gui_view::NoUdiskF);
      return 1;
    }
  }

  //电机测试
  if (touchxy(240, 207, 320, 294))
  {
    respond_gui_send_sem(StepTestValue);
    /*
    //20170825一键测试
    respond_gui_send_sem(FanTestValue);
    respond_gui_send_sem(HeatTestValue);
    */
    return 1;
  }
  //风扇测试
  if (touchxy(20, 210, 101, 295))
  {
    respond_gui_send_sem(FanTestValue);
    return 1;
  }
  //加热测试
  if (touchxy(130, 207, 215, 294))
  {
    respond_gui_send_sem(HeatTestValue);
    return 1;
  }
  //最大行程
  if (touchxy(18, 100, 102, 186))
  {
    respond_gui_send_sem(RunMaxPos);
    return 1;
  }

  //加熱計時
  if (touchxy(130, 100, 211, 186))
  {
    respond_gui_send_sem(CalHeatTime);
    gui_set_curr_display(board_test_cal_heat_time_gui);
    return 1;
  }
  //TFTLCD屏误触发计数测试，卢工2017.4.20
  if (touchxy(240, 100, 320, 186))
  {
    lcdtest_lastdisplay = currentdisplay;
    gui_set_curr_display(board_test_cal_touch_count);
    return 1;
  }
  return 0;
}

void BoardTest::modelSelect(void)
{
  if(gui_is_refresh())
  {
    display_picture(31);

    switch (ModelSelect)
    {
    case M14:
      LCD_Fill_Default(89+5,41+5,89+5+20,41+5+12);
      break;
    case M2030:
      LCD_Fill_Default(202+5,41+5,202+5+20,41+5+12);
      break;
    case M2041:
      LCD_Fill_Default(315+5,41+5,315+5+20,41+5+12);
      break;
    case M2048:
      LCD_Fill_Default(424+5,41+5,424+5+20,41+5+12);
      break;
    case M3145:
      LCD_Fill_Default(91+5,119+5,91+5+20,119+5+12);
      break;
    case M4141:
      LCD_Fill_Default(202+5,119+5,202+5+20,119+5+12);
      break;
    case M4040:
      LCD_Fill_Default(315+5,119+5,315+5+20,119+5+12);
      break;
    case M4141S:
      LCD_Fill_Default(424+5,119+5,424+5+20,119+5+12);
      break;
    case AMP410W:
      LCD_Fill_Default(91+5,197+5,91+5+20,197+5+12);
      break;
    case M14R03:
      LCD_Fill_Default(202+5,197+5,202+5+20,197+5+12);
      break;
    case M2030HY:
      LCD_Fill_Default(315+5,197+5,315+5+20,197+5+12);
      break;
    case M14S:
      LCD_Fill_Default(424+5,197+5,424+5+20,197+5+12);
      break;
    default:
      break;
    }
  }

  if(touchxy(24,40,115,109))
  {
    ModelSelect=M14;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(136,40,229,109))
  {
    ModelSelect=M2030;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(252,40,342,109))
  {
    ModelSelect=M2041;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(362,40,452,109))
  {
    ModelSelect=M2048;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(25,119,117,189))
  {
    ModelSelect=M3145;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(136,119,229,189))
  {
    ModelSelect=M4141;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(252,119,342,189))
  {
    ModelSelect=M4040;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(362,119,452,189))
  {
    ModelSelect=M4141S;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(25,197,117,265))
  {
    ModelSelect=AMP410W;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(136,197,229,265))
  {
    ModelSelect=M14R03;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(252,197,342,265))
  {
    ModelSelect=M2030HY;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(362,197,452,265))
  {
    ModelSelect=M14S;
    gui_set_curr_display(board_test_model_select);
    return ;
  }
  if(touchxy(0,271,240,320))
  {
    if(ModelSelect!=NoSelectModel)
    {
      t_sys_data_current.model_id = ModelSelect;
      // 机器型号初始化
      sg_machine::machine_init();
      // 自动调平初始化
      auto_bed_level_adjust_init();
    }

    if(ModelSelect==NoSelectModel)
    {
      gui_set_curr_display(MachineSetting);
    }
    else
    {
      gui_set_curr_display(board_test_display_function);
    }
    ModelSelect=NoSelectModel;
    return ;
  }
  if(touchxy(240,271,420,320))
  {
    ModelSelect=NoSelectModel;
    gui_set_curr_display(MachineSetting);
    return ;
  }
}

void BoardTest::toggleStepStatus(void)
{
  if(!motion_3d.enable_board_test)
  {
    return;
  }
  if(0==isStepTest)
  {
    isStepTest=1;
    // 张敏聪 测试光轴
//    sys_send_gcode_cmd("G28 X0");
  }
  else
  {
    isStepTest=0;
  }
}

void BoardTest::toggleFanStatus(void)
{
  if(!motion_3d.enable_board_test)
  {
    return;
  }
  if(0==isFanTest)
  {
    isFanTest=1;
  }
  else
  {
    isFanTest=0;
  }
}

void BoardTest::toggleHeatStatus(void)
{
  if(!motion_3d.enable_board_test)
  {
    return;
  }
  if(0==isHeatTest)
  {
    isHeatTest=1;

  }
  else
  {
    isHeatTest=0;
  }
}

void BoardTest::stepTest(void)
{
  static ULONG steptest_last_time=0;
  if(steptest_last_time<sys_task_get_tick_count())
  {
    if(1==isStepTest)
    {
      if(sg_grbl::planner_moves_planned() == 0)
      {
        // 全部正转反转
        sys_send_gcode_cmd("G92 X0 Y75 Z0 E75 B75 isInternal");
        sys_send_gcode_cmd("G1 F2400 X75 Y0 Z15 E0 B0 isInternal");
        sys_send_gcode_cmd("G1 F2400 X0 Y75 Z0 E75 B75 isInternal");

        // X轴正转反转
        sys_send_gcode_cmd("G92 X0 isInternal");
        sys_send_gcode_cmd("G1 F2400 X50 isInternal");
        sys_send_gcode_cmd("G1 F2400 X0 isInternal");

        // Y轴正转反转
        sys_send_gcode_cmd("G92 Y50 isInternal");
        sys_send_gcode_cmd("G1 F2400 Y0 isInternal");
        sys_send_gcode_cmd("G1 F2400 Y50 isInternal");

        // Z轴正转反转
        sys_send_gcode_cmd("G92 Z0 isInternal");
        sys_send_gcode_cmd("G1 F2400 Z10 isInternal");
        sys_send_gcode_cmd("G1 F2400 Z0 isInternal");

        // EB轴正转反转
        sys_send_gcode_cmd("G92 E50 B50 isInternal");
        sys_send_gcode_cmd("G1 F2400 E0 B0 isInternal");
        sys_send_gcode_cmd("G1 F2400 E50 B50 isInternal");
      }
    }
    else
    {
      if(sg_grbl::planner_moves_planned() == 0)
      {
        sys_send_gcode_cmd("M84 X Y Z E B isInternal");
      }
    }
    steptest_last_time=sys_task_get_tick_count()+500;
  }
}

void BoardTest::fanTest(void)
{
  static INT fantest=0;
  static ULONG fantest_last_time=0;

  if(fantest_last_time<sys_task_get_tick_count())
  {
    if(1==isFanTest)
    {
      if(fantest)
      {
        gpio_e_motor_fan_control(true);
        gpio_board_fan_control(true);
        if(t_sys_data_current.model_id != M41G)
        {
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET); // 5v風扇
        }
        sys_send_gcode_cmd("M106 S255 isInternal");
        fantest=0;
      }
      else
      {
        gpio_e_motor_fan_control(false);
        gpio_board_fan_control(false);
        if(t_sys_data_current.model_id != M41G)
        {
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET); // 5v風扇
        }
        sys_send_gcode_cmd("M106 S0 isInternal");
        fantest=1;
      }
    }
    else
    {
      gpio_e_motor_fan_control(false);
      gpio_board_fan_control(false);
      if(t_sys_data_current.model_id != M41G)
      {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET); // 5v風扇
      }
      sys_send_gcode_cmd("M106 S0 isInternal");
    }
    fantest_last_time=sys_task_get_tick_count()+500;
  }
}

void BoardTest::heatTest(void)
{
//  static INT heatest=0;
  static ULONG heattest_last_time=0;

  if(heattest_last_time<sys_task_get_tick_count())
  {
    if(1==isHeatTest)
    {
      sys_send_gcode_cmd("M140 S100 isInternal");
      sys_send_gcode_cmd("M104 S220 isInternal");
    }
    else
    {
      sys_send_gcode_cmd("M140 S0 isInternal");
      sys_send_gcode_cmd("M104 S0 isInternal");
    }
    heattest_last_time=sys_task_get_tick_count()+500;
  }
}

void BoardTest::process(void)
{
  if(processOn)
  {
    heatTest();
    fanTest();
    stepTest();
    sys_os_delay(50);
  }
}

void BoardTest::runMaxPos(void)
{
  if(!motion_3d.enable_board_test)
  {
    return;
  }
  static CHAR gcodeG1XmaxCommandBuf[30] = {0};
  static CHAR gcodeG1YmaxCommandBuf[30] = {0};
  static CHAR gcodeG1ZmaxCommandBuf[30] = {0};
  sys_send_gcode_cmd("G90 isInternal");
  sys_send_gcode_cmd("M82 isInternal");
  sys_send_gcode_cmd("G28 X0 Y0 isInternal"); // XY归零命令
  sys_send_gcode_cmd("G28 Z0 isInternal"); // Z归零命令
  sys_send_gcode_cmd("G92 X0 Y0 Z0 isInternal"); // 设置XYZ当前位置

  memset(gcodeG1ZmaxCommandBuf, 0, sizeof(gcodeG1ZmaxCommandBuf));
  (void)snprintf(gcodeG1ZmaxCommandBuf, sizeof(gcodeG1ZmaxCommandBuf), "G1 Z%f isInternal", motion_3d_model.xyz_max_pos[2]);
  sys_send_gcode_cmd(gcodeG1ZmaxCommandBuf); // Z下降最大

  memset(gcodeG1XmaxCommandBuf, 0, sizeof(gcodeG1XmaxCommandBuf));
  (void)snprintf(gcodeG1XmaxCommandBuf, sizeof(gcodeG1XmaxCommandBuf), "G1 X%f isInternal", motion_3d_model.xyz_max_pos[0]);
  sys_send_gcode_cmd(gcodeG1XmaxCommandBuf); // X移动最大

  memset(gcodeG1YmaxCommandBuf, 0, sizeof(gcodeG1YmaxCommandBuf));
  (void)snprintf(gcodeG1YmaxCommandBuf, sizeof(gcodeG1YmaxCommandBuf), "G1 Y%f isInternal", motion_3d_model.xyz_max_pos[1]);
  sys_send_gcode_cmd(gcodeG1YmaxCommandBuf); // Y移动最大

  sys_send_gcode_cmd("G1 X0 isInternal");   // X归零命令
  sys_send_gcode_cmd("G1 Y0 isInternal");  // Y归零命令
  sys_send_gcode_cmd("G1 Z0 isInternal");  // Z归零命令
}

bool BoardTest::calHeatTimeGuiTouchCheck(void)
{
  return 0;
}

bool BoardTest::calHeatTimeGui(void)
{
  INT second = 0, minute = 0, hour= 0;
  CHAR buffer[32];

  static UINT8 HEATING_COMPLETE = 0 ;

  second= ((sys_task_get_tick_count()/1000) - clockTime);
  hour=second/3600;
  minute = (second-(hour*3600))/60;
  second = (second-(hour*3600))%60;

  if((sg_grbl::temperature_get_bed_current() >= 50) && (heatStatus == 1))
  {
    snprintf(bed50HeatTimeStr, sizeof(bed50HeatTimeStr), "Bed50 Time  = %3d:%02d:%02d",(INT)hour,(INT)minute,(INT)second); // 最多从源串中拷贝n－1个字符到目标串中，然后再在后面加一个0
    heatStatus = 2;
  }

  if((sg_grbl::temperature_get_bed_current() >= 70) && (heatStatus == 2))
  {
    snprintf(bed70HeatTimeStr, sizeof(bed70HeatTimeStr), "Bed70 Time  = %3d:%02d:%02d",(INT)hour,(INT)minute,(INT)second); // 最多从源串中拷贝n－1个字符到目标串中，然后再在后面加一个0
    heatStatus = 3;
  }

  if((1U == t_gui_p.m190_heating_complete) && (heatStatus == 3))
  {
    snprintf(bed115HeatTimeStr, sizeof(bed115HeatTimeStr), "Bed115 Time = %3d:%02d:%02d",(INT)hour,(INT)minute,(INT)second); // 最多从源串中拷贝n－1个字符到目标串中，然后再在后面加一个0
    heatStatus = 4;
  }

  if((0 == sg_grbl::temperature_is_extruder_heating(static_cast<INT>(gcode::tmp_extruder)))&&(HEATING_COMPLETE == 0 ) )
  {
    HEATING_COMPLETE = 1 ;
    second= ((sys_task_get_tick_count()/1000) - clockTime);
    hour=second/3600;
    minute = (second-(hour*3600))/60;
    second = (second-(hour*3600))%60;
    snprintf(nozzleHeatTimeStr, sizeof(nozzleHeatTimeStr), "Nozzle Time = %3d:%02d:%02d",(INT)hour,(INT)minute,(INT)second); // 最多从源串中拷贝n－1个字符到目标串中，然后再在后面加一个0
    DisplayTextDefault((UINT8*)nozzleHeatTimeStr,60,130);
  }

  if(gui_is_refresh())
  {
    display_picture(79);
    displayInit();
    displayText();

    SetTextDisplayRange(62+(12*14),86,12*14,24,&PrintTimeTextRange);
    ReadTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf);
  }
  if(calHeatTimeGuiTouchCheck())
  {
    return 1;
  }
  if(gui_is_rtc())
  {
    displayText();

    snprintf(buffer, sizeof(buffer), "Clock Time  = ");
    DisplayTextDefault((UINT8*)buffer,62,86);
    snprintf(buffer, sizeof(buffer), "%3d:%02d:%02d",hour,minute,second);
    CopyTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf, TextRangeBuf);
    DisplayTextInRangeDefault((PUCHAR)buffer, PrintTimeTextRange,TextRangeBuf);

    if(2 == heatStatus)
    {
      DisplayTextDefault((UINT8*)bed50HeatTimeStr,60,178);
    }
    else if(3 == heatStatus)
    {
      DisplayTextDefault((UINT8*)bed70HeatTimeStr,60,220);
    }
    else if(4 == heatStatus)
    {
      DisplayTextDefault((UINT8*)bed115HeatTimeStr,60,266);
      heatStatus = 5;
    }

  }
  if( (heatStatus == 5)&&(HEATING_COMPLETE == 1) )
  {
    beep(500);
  }
  return 1;
}

void BoardTest::calHeatTime(void)
{
  if(!motion_3d.enable_board_test)
  {
    return;
  }
  processOn = 0;
  t_gui_p.m109_heating_complete = 0U;
  t_gui_p.m190_heating_complete = 0U;
  if(t_custom_services.disable_hot_bed)
  {
    heatStatus = 5;
    snprintf(bed50HeatTimeStr, sizeof(bed50HeatTimeStr), "Bed50 Time  =   0: 0: 0");
    snprintf(bed70HeatTimeStr, sizeof(bed70HeatTimeStr), "Bed75 Time  =   0: 0: 0");
    snprintf(bed115HeatTimeStr, sizeof(bed115HeatTimeStr), "Bed115 Time =   0: 0: 0");
  }
  else
  {
    heatStatus = 1;
  }

  clockTime = sys_task_get_tick_count()/1000;

  sys_send_gcode_cmd("M104 S220 isInternal"); // 加热喷嘴，不等待
  if(t_sys_data_current.model_id == M3145)
  {
    sys_send_gcode_cmd("M190 S100 isInternal");  // 加热热床，等待
  }
  else
  {
    sys_send_gcode_cmd("M190 S115 isInternal");  // 加热热床，等待
  }

  sys_send_gcode_cmd("M109 S220 isInternal"); // 防止加热热床比喷嘴快，导致喷嘴无加热
  sys_send_gcode_cmd("M104 S0 isInternal"); // 喷嘴温度置零
  sys_send_gcode_cmd("M140 S0 isInternal");// 热床温度置零
}
/*
 *卢工屏幕触摸计数测试2017421
*/
//#include <math.h>
UINT32 Lcd_Count_touch = 0;//在检测按键触摸处使用
UINT8 BoardTest::ERRTouchCount(void)
{
  INT second = 0;
  CHAR buffer[32];

  if(gui_is_refresh())
  {
    display_picture(94);//94
    //		clockTime = sys_task_get_tick_count()/1000;//时间不清零
//    SetTextDisplayRange(62+12*14,86,12*14,24,&PrintTimeTextRange);
//    ReadTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf);
//
//		Lcd_Count_touch = 0;
  }
  second= ((sys_task_get_tick_count()/1000) - clockTime);

  if(gui_is_rtc())//按一次或500ms更新一次数据
  {
    display_picture(94);//79,94
    snprintf(buffer, sizeof(buffer), "Clock Time  = ");
    DisplayTextDefault((UINT8*)buffer,62,86);
    snprintf(buffer, sizeof(buffer), "%3d:%02d:%02d",second/3600,(second%3600)/60,(second%3600)%60);
    DisplayTextDefault((UINT8*)buffer,62+(12*14),86);
//    CopyTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf, TextRangeBuf);
//    DisplayTextInRangeDefault((PUCHAR)buffer, PrintTimeTextRange,TextRangeBuf);

    snprintf(buffer, sizeof(buffer), "Coutn_touch[0]=%d", (Lcd_Count_touch%(256*256)));//Lcd_Count_touch&0x0000ffff
    DisplayTextDefault((UINT8*)buffer,60,178);
    snprintf(buffer, sizeof(buffer), "Coutn_touch[1]=%d",Lcd_Count_touch/(256*256));
    DisplayTextDefault((UINT8*)buffer,60,266);

  }
  if(touchxy(0,0,150,65))         //返回按钮
  {
    if(motion_3d.enable_board_test)
    {
      ++Lcd_Count_touch;

    }
    if(lcdtest_lastdisplay != board_test_display_function)
    {
      motion_3d.enable_board_test = false;
    }
    gui_set_curr_display(lcdtest_lastdisplay);
  }
  if(touchxy(0,0,480,320))
  {
    if(motion_3d.enable_board_test)//从测试按键进来的才执行这个计数，且按一次计数一次，一直按着不会计数。为了防止测试界面进来不计数
    {
      ++Lcd_Count_touch;									 //非打印状态主界面和打印状态主界面中的隐藏按键，进来的不执行该计数2017422
    }
  }
  if(touchxy(400,0,480,65))         //清零按钮
  {
    Lcd_Count_touch = 0;
  }
  return 1;
}
#define Debug_PresureSensor
#ifdef Debug_PresureSensor
void BoardTest::PressureTest(void)
{
  CHAR buffer[32];
  static UINT8 Loop =0,count = 0;

  if(gui_is_refresh())
  {
    display_picture(94);//94
//		second= (sys_task_get_tick_count()+500);
    //		clockTime = sys_task_get_tick_count()/1000;//时间不清零
//    SetTextDisplayRange(62+12*14,86,12*14,24,&PrintTimeTextRange);
//    ReadTextDisplayRangeInfo(PrintTimeTextRange,TimeTextRangeBuf);
//
//		Lcd_Count_touch = 0;
  }


  if(gui_is_rtc())//按一次或500ms更新一次数据
  {
//		second= (sys_task_get_tick_count()+500);
    display_picture(94);//79,94
    snprintf(buffer, sizeof(buffer), "Clock Time  = ");
    DisplayTextDefault((UINT8*)buffer,62,86);
    ++count;
    count &= 0x03;
  }
  if(touchxy(0,0,150,65))         //返回按钮
  {
    gui_set_curr_display(lcdtest_lastdisplay);
  }
  if(touchxy(0,70,150,320))
  {
    Loop = 1;
  }
  if(touchxy(150,0,300,320))
  {
    Loop = 0;
  }
  if(Loop)
  {
    if(count == 0)
    {
      sys_send_gcode_cmd("G1 Z0 isInternal");
    }
    else if(count == 2)
    {
      sys_send_gcode_cmd("G1 Z30 isInternal");
    }

  }
  static UINT8 Zposition=0;
  if(touchxy(300,0,480,150))
  {
    if(Zposition < 255)
    {
      ++Zposition;
    }
    sprintf(buffer,"G1 Z%d isInternal",Zposition);
    sys_send_gcode_cmd(buffer);
  }
  else if(touchxy(300,170,480,320))
  {
    if(Zposition > 0)
    {
      Zposition--;
    }
    sprintf(buffer,"G1 Z%d isInternal",Zposition);
    sys_send_gcode_cmd(buffer);
  }
}
#endif

BoardTest boardTest;





