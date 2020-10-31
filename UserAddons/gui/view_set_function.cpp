#include "globalvariables.h"
#include "view_common.h"
#include "view_commonf.h"
#include "sysconfig_data.h"
#include  "interface.h"
#ifdef __cplusplus
extern "C" {
#endif

static void FunctionSettingPage2(void);

///////////////////////////////////////////////////////////////////////
#define FunctionOpen 1                /*!< 功能打开 */
#define FunctionClose 0               /*!< 功能关闭 */
// 第1页
static UCHAR BaseFunction = FunctionClose;                  /*!< 基础 */
static UCHAR ColorMixingFunction = FunctionClose;           /*!< 混色 */
static UCHAR PowerOffRecoveryFunction = FunctionClose;      /*!< 断电 */
static UCHAR MatCheckFunction = FunctionClose;              /*!< 断料 */
static UCHAR BlockDetectFunction = FunctionClose;           /*!< 堵料 */
static UCHAR BedLevelFunction = FunctionClose;              /*!< 自动调平 */
static UCHAR IsSoftfilament = FunctionClose;                /*!< 软料（小齿轮） */
static UCHAR IsV5Extruder = FunctionClose;                  /*!< V5喷嘴（E电机风扇与扇热风扇一起） */
// 第2页
static UCHAR IsV51Extruder = FunctionClose;                 /*!< V5喷嘴（喷嘴风扇与扇热风扇一起） */
static UCHAR enableCavityTemp = FunctionClose;              /*!< 腔体温度开关 */
static UCHAR typeOfThermistor = FunctionClose;              /*!< 热敏电阻类型 */
static UCHAR enableHighTemp = FunctionClose;                /*!< 热敏电阻类型 */
static UCHAR Is2GT = FunctionClose;                         /*!< 同步轮类型 */
static UCHAR IsMechanismLevel = FunctionClose;                         /*!< 同步轮类型 */
static UCHAR IsLaserMode = FunctionClose;                         /*!< 同步轮类型 */

static void lcd_fill_redblock(UCHAR pageId)
{
  if (((1 == pageId) && (FunctionOpen == BaseFunction)) || ((2 == pageId) && (FunctionOpen == IsV51Extruder)))
  {
    LCD_Fill_Default(83, 78, 83 + 20, 78 + 12);
  }

  if (((1 == pageId) && (FunctionOpen == ColorMixingFunction)) || ((2 == pageId) && (FunctionOpen == enableCavityTemp)))
  {
    LCD_Fill_Default(198, 78, 198 + 20, 78 + 12);
  }

  if (((1 == pageId) && (FunctionOpen == PowerOffRecoveryFunction)) || ((2 == pageId) && (FunctionOpen == typeOfThermistor)))
  {
    LCD_Fill_Default(312, 78, 312 + 20, 78 + 12);
  }

  if (((1 == pageId) && (FunctionOpen == MatCheckFunction)) || ((2 == pageId) && (FunctionOpen == enableHighTemp)))
  {
    LCD_Fill_Default(428, 78, 428 + 20, 78 + 12);
  }

  if (((1 == pageId) && (FunctionOpen == BlockDetectFunction)) || ((2 == pageId) && (FunctionOpen == IsMechanismLevel)))
  {
    LCD_Fill_Default(83, 157, 83 + 20, 157 + 12);
  }

  if (((1 == pageId) && ((FunctionOpen == BedLevelFunction) || (2 == BedLevelFunction))) || ((2 == pageId) && (FunctionOpen == IsLaserMode)))
  {
    LCD_Fill_Default(198, 157, 198 + 20, 157 + 12);
  }

  if (((1 == pageId) && (FunctionOpen == IsSoftfilament)) || ((2 == pageId) && (FunctionOpen == Is2GT)))
  {
    LCD_Fill_Default(312, 157, 312 + 20, 157 + 12);
  }

  if (((1 == pageId) && (FunctionOpen == IsV5Extruder)))
  {
    LCD_Fill_Default(428, 157, 428 + 20, 157 + 12);
  }
}

static void function_touchscan(UCHAR pageId)
{
  if (TouchXY_NoBeep(25, 88, 452, 248)) //选择功能键区域
  {
    if (touchxy(0, 50, 124, 145)) //基础功能
    {
      if (1 == pageId)
      {
        BaseFunction = (BaseFunction ? FunctionClose : FunctionOpen);
      }
      else if (2 == pageId)
      {
        IsV51Extruder = (IsV51Extruder ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(125, 50, 239, 145)) //混色功能
    {
      if (1 == pageId)
      {
        ColorMixingFunction = (ColorMixingFunction ? FunctionClose : FunctionOpen);
      }
      else if (2 == pageId)
      {
        enableCavityTemp = (enableCavityTemp ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(240, 50, 354, 145)) //断电续打功能
    {
      if (1 == pageId)
      {
        PowerOffRecoveryFunction = (PowerOffRecoveryFunction ? FunctionClose : FunctionOpen);
      }
      else if (2 == pageId)
      {
        typeOfThermistor = (typeOfThermistor ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(355, 50, 480, 145)) //断料检测功能
    {
      if (1 == pageId)
      {
        // 20171130 新版限位开关判断条件,低电平则表示无料
        // 设置断料检测为2
        MatCheckFunction = (MatCheckFunction ? FunctionClose : FunctionOpen);

        if (MatCheckFunction)
        {
          BlockDetectFunction = 0;
        }
      }
      else if (2 == pageId)
      {
        enableHighTemp = (enableHighTemp ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(0, 121, 124, 224)) //堵料检测功能
    {
      if (1 == pageId)
      {
        BlockDetectFunction = (BlockDetectFunction ? FunctionClose : FunctionOpen);

        if (BlockDetectFunction)
        {
          MatCheckFunction = 0;
        }
      }
      else if (2 == pageId)
      {
        IsMechanismLevel = (IsMechanismLevel ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(125, 121, 239, 224)) //自动调平功能
    {
      if (1 == pageId)
      {
        BedLevelFunction = (BedLevelFunction ? FunctionClose : 2); //FunctionOpen);
      }
      else if (2 == pageId)
      {
        IsLaserMode = (IsLaserMode ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(240, 121, 354, 224)) //使用软料齿轮
    {
      if (1 == pageId)
      {
        IsSoftfilament = (IsSoftfilament ? FunctionClose : FunctionOpen);
      }
      else if (2 == pageId)
      {
        Is2GT = (Is2GT ? FunctionClose : FunctionOpen);
      }
    }
    else if (touchxy(355, 121, 480, 224)) //使用V5喷嘴
    {
      if (1 == pageId)
      {
        IsV5Extruder = (IsV5Extruder ? FunctionClose : FunctionOpen);
      }
    }

    if ((1 == pageId) && IsV5Extruder)
    {
      IsV51Extruder = FunctionClose;
    }
    else if ((2 == pageId) && IsV51Extruder)
    {
      IsV5Extruder = FunctionClose;
    }

    if (1 == pageId)
    {
      gui_set_curr_display(FunctionSetting);
    }
    else if (2 == pageId)
    {
      gui_set_curr_display(FunctionSettingPage2);
    }

    return ;
  }

  if (TouchXY_NoBeep(0, 230, 480, 320)) //确认或取消键
  {
    if (touchxy(0, 230, 150, 320)) //确认键
    {
      if (BaseFunction || ColorMixingFunction || PowerOffRecoveryFunction || MatCheckFunction || BlockDetectFunction || BedLevelFunction || IsSoftfilament || IsV5Extruder || IsV51Extruder || Is2GT)
      {
        t_sys_data_current.enable_color_mixing = ColorMixingFunction;
        t_sys_data_current.enable_powerOff_recovery = PowerOffRecoveryFunction;
        t_sys_data_current.enable_material_check = MatCheckFunction;
        t_sys_data_current.enable_block_detect = BlockDetectFunction;
        t_sys_data_current.enable_bed_level = BedLevelFunction;
        t_sys_data_current.enable_soft_filament = IsSoftfilament;
        t_sys_data_current.enable_cavity_temp = enableCavityTemp;
        t_sys_data_current.enable_type_of_thermistor = typeOfThermistor;
        t_sys_data_current.enable_high_temp = enableHighTemp;
        t_sys_data_current.is_2GT = Is2GT;
        t_sys_data_current.IsMechanismLevel = IsMechanismLevel;
        t_sys_data_current.IsLaserMode = IsLaserMode;

        if (IsV51Extruder)
        {
          t_sys_data_current.enable_v5_extruder = IsV51Extruder + 1;
        }
        else if (IsV5Extruder)
        {
          t_sys_data_current.enable_v5_extruder = IsV5Extruder;
        }
        else
        {
          t_sys_data_current.enable_v5_extruder = 0;
        }

        respond_gui_send_sem(ConfirmChangeFunctionValue);
      }
    }

    if (touchxy(160, 230, 320, 320)) //下一页
    {
      if (1 == pageId)
      {
        gui_set_curr_display(FunctionSettingPage2);
      }
      else if (2 == pageId)
      {
        gui_set_curr_display(FunctionSetting);
      }

      return ;
    }

    (void)touchxy(330, 230, 480, 320); //取消键
    gui_set_curr_display(MachineSetting);
    return ;
  }
}

void FunctionSetting(void)
{
  if (gui_is_refresh())
  {
    display_picture(40);
    lcd_fill_redblock(1); //填充红色块
  }

  function_touchscan(1);//扫描选择功能按键
}

static void FunctionSettingPage2(void)
{
  if (gui_is_refresh())
  {
    display_picture(113);
    lcd_fill_redblock(2); //填充红色块
  }

  function_touchscan(2);//扫描选择功能按键
}



#ifdef __cplusplus
}//extern "C" {
#endif

















