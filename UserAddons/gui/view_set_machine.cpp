#include "view_common.h"
#include "view_commonf.h"
#include "globalvariables.h"
#include "config_model_tables.h"
#include "machinecustom.h"
#include "functioncustom.h"
#include  "interface.h"
//#include "ConfigurationStore.h"
#include "sysconfig_data.h"
#include "config_motion_3d.h"
#include "Alter.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NoSelectModel 255
#define SelectM14 0
#define SelectM2030 1
#define SelectM2041 2
#define SelectM2048 3
#define SelectM3145 4
#define SelectM4141 5
#define SelectM4040 6
#define SelectM4141S 7
#define SelectAMP410W 8
#define SelectM14R03 9
#define SelectM2030HY 10
#define SelectM14S 11
#define SelectM3145S 12
#define SelectM15 13
#define SelectM3036 14
#define SelectM4141S_New 15
#define SelectM41G 16
#define SelectM3145T 17
#define SelectM3145K 18
#define SelectK5 19
#define SelectF2030TP 20

UINT8 ModelSelect = NoSelectModel;
extern void SettingInterface6(void);

void NextModelSetting(void);

void ModelSetting(void)
{
  if (gui_is_refresh())
  {
    display_picture(31);

    switch (ModelSelect)
    {
    case SelectM14:
      LCD_Fill_Default(89 + 5, 41 + 5, 89 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM2030:
      LCD_Fill_Default(202 + 5, 41 + 5, 202 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM2041:
      LCD_Fill_Default(315 + 5, 41 + 5, 315 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM2048:
      LCD_Fill_Default(424 + 5, 41 + 5, 424 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM3145:
      LCD_Fill_Default(91 + 5, 119 + 5, 91 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectM4141:
      LCD_Fill_Default(202 + 5, 119 + 5, 202 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectM4040:
      LCD_Fill_Default(315 + 5, 119 + 5, 315 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectM4141S:
      LCD_Fill_Default(424 + 5, 119 + 5, 424 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectAMP410W:
      LCD_Fill_Default(91 + 5, 197 + 5, 91 + 5 + 20, 197 + 5 + 12);
      break;

    case SelectM14R03:
      LCD_Fill_Default(202 + 5, 197 + 5, 202 + 5 + 20, 197 + 5 + 12);
      break;

    case SelectM2030HY:
      LCD_Fill_Default(315 + 5, 197 + 5, 315 + 5 + 20, 197 + 5 + 12);
      break;

    case SelectM14S:
      LCD_Fill_Default(424 + 5, 197 + 5, 424 + 5 + 20, 197 + 5 + 12);
      break;

    default:
      break;
    }
  }

  if (touchxy(24, 40, 115, 109))
  {
    ModelSelect = SelectM14;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(136, 40, 229, 109))
  {
    ModelSelect = SelectM2030;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(252, 40, 342, 109))
  {
    ModelSelect = SelectM2041;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(362, 40, 452, 109))
  {
    ModelSelect = SelectM2048;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(25, 119, 117, 189))
  {
    ModelSelect = SelectM3145;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(136, 119, 229, 189))
  {
    ModelSelect = SelectM4141;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(252, 119, 342, 189))
  {
    ModelSelect = SelectM4040;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(362, 119, 452, 189))
  {
    ModelSelect = SelectM4141S;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(25, 197, 117, 265))
  {
    ModelSelect = SelectAMP410W;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(136, 197, 229, 265))
  {
    ModelSelect = SelectM14R03;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(252, 197, 342, 265))
  {
    ModelSelect = SelectM2030HY;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(362, 197, 452, 265))
  {
    ModelSelect = SelectM14S;
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(28, 270, 130, 320)) //确认键
  {
    if (ModelSelect != NoSelectModel)
    {
      t_sys_data_current.model_id = ModelSelect;
      respond_gui_send_sem(ConfirmChangeModelValue);
    }

    ModelSelect = NoSelectModel;
    gui_set_curr_display(MachineSetting);
    return ;
  }

  if (touchxy(192, 271, 290, 320)) //取消键
  {
    ModelSelect = NoSelectModel;
    gui_set_curr_display(MachineSetting);
    return ;
  }

  if (touchxy(352, 271, 454, 320)) //下一页键
  {
    ModelSelect = NoSelectModel;
    gui_set_curr_display(NextModelSetting);
    return ;
  }
}

void logoSetting(void)
{
  if (gui_is_refresh())
  {
    DisplayLogoPicture(3);

    //由于Picture3是logo界面选择图片，所以id没有3
    switch (t_sys_data_current.logo_id)
    {
    case 0:
      LCD_Fill_Default(89 + 5, 41 + 5, 89 + 5 + 20, 41 + 5 + 12);
      break;

    case 1:
      LCD_Fill_Default(202 + 5, 41 + 5, 202 + 5 + 20, 41 + 5 + 12);
      break;

    case 2:
      LCD_Fill_Default(315 + 5, 41 + 5, 315 + 5 + 20, 41 + 5 + 12);
      break;

    case 4:
      LCD_Fill_Default(424 + 5, 41 + 5, 424 + 5 + 20, 41 + 5 + 12);
      break;

    case 5:
      LCD_Fill_Default(91 + 5, 119 + 5, 91 + 5 + 20, 119 + 5 + 12);
      break;

    case 6:
      LCD_Fill_Default(202 + 5, 119 + 5, 202 + 5 + 20, 119 + 5 + 12);
      break;

    default:
      break;
    }
  }

  if (touchxy(24, 40, 115, 109))
  {
    t_sys_data_current.logo_id = 0;
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(136, 40, 229, 109))
  {
    t_sys_data_current.logo_id = 1;
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(252, 40, 342, 109))
  {
    t_sys_data_current.logo_id = 2;
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(362, 40, 452, 109))
  {
    t_sys_data_current.logo_id = 4;
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(25, 119, 117, 189))
  {
    t_sys_data_current.logo_id = 5;
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(136, 119, 229, 189))
  {
    t_sys_data_current.logo_id = 6;
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(28, 270, 130, 320)) //确认键
  {
    //    if(t_sys_data_current.logo_id)
    //      t_sys_data_current.enable_LOGO_interface = 1;//开启logo
    //    else
    //      t_sys_data_current.enable_LOGO_interface = 0;//关闭logo
    respond_gui_send_sem(ConfirmChangelogoValue);
    gui_set_curr_display(MachineSetting);
    return ;
  }

  if (touchxy(192, 271, 290, 320)) //取消键
  {
    gui_set_curr_display(MachineSetting);
    return ;
  }

  //  if(touchxy(352,271,454,320)) //下一页键
  //  {
  //    ModelSelect=NoSelectModel;
  //    gui_set_curr_display(NextModelSetting);
  //    return ;
  //  }
}


void NextModelSetting(void)
{
  if (gui_is_refresh())
  {
    display_picture(42);

    switch (ModelSelect)
    {
    case SelectM3145S:
      LCD_Fill_Default(89 + 5, 41 + 5, 89 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM15:
      LCD_Fill_Default(202 + 5, 41 + 5, 202 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM3036:
      LCD_Fill_Default(315 + 5, 41 + 5, 315 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM4141S_New:
      LCD_Fill_Default(424 + 5, 41 + 5, 424 + 5 + 20, 41 + 5 + 12);
      break;

    case SelectM41G:
      LCD_Fill_Default(91 + 5, 119 + 5, 91 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectM3145T:
      LCD_Fill_Default(202 + 5, 119 + 5, 202 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectM3145K:
      LCD_Fill_Default(315 + 5, 119 + 5, 315 + 5 + 20, 119 + 5 + 12);
      break;

    case SelectF2030TP:
      LCD_Fill_Default(424 + 5, 119 + 5, 424 + 5 + 20, 119 + 5 + 12);
      break;

    //    case SelectAMP410W:
    //      LCD_Fill_Default(91+5,197+5,91+5+20,197+5+12);
    //      break;
    //    case SelectM14R03:
    //      LCD_Fill_Default(202+5,197+5,202+5+20,197+5+12);
    //      break;
    //    case SelectM2030HY:
    //      LCD_Fill_Default(315+5,197+5,315+5+20,197+5+12);
    //      break;
    //    case SelectM14S:
    //      LCD_Fill_Default(424+5,197+5,424+5+20,197+5+12);
    //      break;
    default:
      break;
    }
  }

  if (touchxy(24, 40, 115, 109))
  {
    ModelSelect = SelectM3145S;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(136, 40, 229, 109))
  {
    ModelSelect = SelectM15;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(252, 40, 342, 109))
  {
    ModelSelect = SelectM3036;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(362, 40, 452, 109))
  {
    ModelSelect = SelectM4141S_New;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(25, 119, 117, 189))
  {
    ModelSelect = SelectM41G;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(136, 119, 229, 189))
  {
    ModelSelect = SelectM3145T;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(252, 119, 342, 189))
  {
    ModelSelect = SelectM3145K;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  if (touchxy(362, 119, 452, 189))
  {
    ModelSelect = SelectF2030TP;
    gui_set_curr_display(NextModelSetting);
    return ;
  }

  //  if(touchxy(25,197,117,265))
  //  {
  //    ModelSelect=SelectAMP410W;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(136,197,229,265))
  //  {
  //    ModelSelect=SelectM14R03;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(252,197,342,265))
  //  {
  //    ModelSelect=SelectM2030HY;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(362,197,452,265))
  //  {
  //    ModelSelect=SelectM14S;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  if (touchxy(28, 270, 130, 320)) //确认键
  {
    if (ModelSelect != NoSelectModel)
    {
      t_sys_data_current.model_id = ModelSelect;
      respond_gui_send_sem(ConfirmChangeModelValue);
    }

    ModelSelect = NoSelectModel;
    gui_set_curr_display(MachineSetting);
    return ;
  }

  if (touchxy(192, 271, 290, 320)) //取消键
  {
    ModelSelect = NoSelectModel;
    gui_set_curr_display(MachineSetting);
    return ;
  }
}


#define NoSelectPicture 0
#define SelectChinesePicture 1
#define SelectJapanesePicture 2
#define SelectEnglishPicture 3
#define SelectKoreaPicture 4
#define SelectRussiaPicture 5
#define SelectChineseTraditionalPicture 6
static UINT8 PictureSelect = NoSelectPicture;

void PictureSetting(void)
{
  //  if(gui_is_refresh())
  //  {
  //    display_picture(39);

  //    switch (PictureSelect)
  //    {
  //    case SelectChinesePicture:
  //      LCD_Fill_Default(168+5,114+5,168+5+20,114+5+12);
  //      break;
  //    case SelectJapanesePicture:
  //      LCD_Fill_Default(360+5,114+5,360+5+20,114+5+12);
  //      break;
  //    default:
  //      break;
  //    }
  //  }

  //  if(touchxy(93,114,198,196))
  //  {
  //    PictureSelect=SelectChinesePicture;
  //    gui_set_curr_display(PictureSetting);
  //    return ;
  //  }
  //  if(touchxy(283,114,390,196))
  //  {
  //    PictureSelect=SelectJapanesePicture;
  //    gui_set_curr_display(PictureSetting);
  //    return ;
  //  }

  //  if(touchxy(60,215,225,300))  //确认键
  //  {
  //    if(PictureSelect!=NoSelectPicture)
  //    {
  //      SettingInfoToSYS.ChangePicture=PictureSelect;
  //      respond_gui_send_sem(ConfirmChangePictureValue);
  //    }
  //    PictureSelect=NoSelectPicture;
  //    gui_set_curr_display(MachineSetting);

  //    return ;
  //  }
  //  if(touchxy(250,215,425,300))  //取消键
  //  {
  //    PictureSelect=NoSelectPicture;
  //    gui_set_curr_display(MachineSetting);
  //    return ;
  //  }
  if (gui_is_refresh())
  {
    display_picture(39);

    switch (PictureSelect)
    {
    case SelectChinesePicture:
      LCD_Fill_Default(86 + 5, 38 + 5, 86 + 5 + 20, 38 + 5 + 12);
      break;

    case SelectJapanesePicture:
      if (t_sys_data_current.ui_number != 2)
        LCD_Fill_Default(202 + 5, 41 + 5, 202 + 5 + 20, 41 + 5 + 12);

      break;

    case SelectEnglishPicture:
      LCD_Fill_Default(312 + 5, 39 + 5, 312 + 5 + 20, 39 + 5 + 12);
      break;

    case SelectKoreaPicture:
      if (t_sys_data_current.ui_number != 2)
        LCD_Fill_Default(424 + 5, 41 + 5, 424 + 5 + 20, 41 + 5 + 12);

      break;

    case SelectRussiaPicture:
      if (t_sys_data_current.ui_number != 2)
        LCD_Fill_Default(91 + 5, 119 + 5, 91 + 5 + 20, 119 + 5 + 12);

      break;

    case SelectChineseTraditionalPicture:
      LCD_Fill_Default(199 + 5, 116 + 5, 199 + 5 + 20, 116 + 5 + 12);
      break;

    default:
      break;
    }
  }

  if (touchxy(24, 40, 115, 109))
  {
    PictureSelect = SelectChinesePicture;
    gui_set_curr_display(PictureSetting);
    return ;
  }

  if (touchxy(136, 40, 229, 109))
  {
    if (t_sys_data_current.ui_number == 2)
      return;

    PictureSelect = SelectJapanesePicture;
    gui_set_curr_display(PictureSetting);
    return ;
  }

  if (touchxy(252, 40, 342, 109))
  {
    PictureSelect = SelectEnglishPicture;
    gui_set_curr_display(PictureSetting);
    return ;
  }

  //  if(touchxy(362,40,452,109))
  //  {
  //    PictureSelect=SelectKoreaPicture;
  //    gui_set_curr_display(PictureSetting);
  //    return ;
  //  }
  if (touchxy(25, 119, 117, 189))
  {
    if (t_sys_data_current.ui_number == 2)
      return;

    PictureSelect = SelectRussiaPicture;
    gui_set_curr_display(PictureSetting);
    return ;
  }

  if (touchxy(136, 119, 229, 189))
  {
    PictureSelect = SelectChineseTraditionalPicture;
    gui_set_curr_display(PictureSetting);
    return ;
  }

  //  if(touchxy(136,119,229,189))
  //  {
  //    ModelSelect=SelectM4141;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(252,119,342,189))
  //  {
  //    ModelSelect=SelectM4040;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(362,119,452,189))
  //  {
  //    ModelSelect=SelectM4141S;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(25,197,117,265))
  //  {
  //    ModelSelect=SelectAMP410W;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(136,197,229,265))
  //  {
  //    ModelSelect=SelectM14R03;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(252,197,342,265))
  //  {
  //    ModelSelect=SelectM2030HY;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  //  if(touchxy(362,197,452,265))
  //  {
  //    ModelSelect=SelectM14S;
  //    gui_set_curr_display(ModelSetting);
  //    return ;
  //  }
  if (touchxy(55, 271, 240, 320)) //确认键
  {
    if (PictureSelect != NoSelectPicture)
    {
      t_sys_data_current.pic_id = PictureSelect;
      respond_gui_send_sem(ConfirmChangePictureValue);
    }

    PictureSelect = NoSelectPicture;

    if (Gui_Mode == MODE_SET_LAN)
    {
      gui_set_curr_display(settingF);
      Gui_Mode = MODE_SET;
    }
    else
      gui_set_curr_display(MachineSetting);

    return ;
  }

  if (touchxy(240, 271, 420, 320)) //取消键
  {
    PictureSelect = NoSelectPicture;

    if (Gui_Mode == MODE_SET_LAN)
    {
      gui_set_curr_display(settingF);
      Gui_Mode = MODE_SET;
    }
    else
      gui_set_curr_display(MachineSetting);

    return ;
  }
}

//#define FunctionOpen 1
//#define FunctionClose 0
//UINT8 BaseFunction=FunctionClose;
//UINT8 ColorMixingFunction=FunctionClose;
//UINT8 PowerOffRecoveryFunction=FunctionClose;
//UINT8 MatCheckFunction=FunctionClose;
//UINT8 BlockDetectFunction=FunctionClose;
//UINT8 BedLevelFunction=FunctionClose;
//UINT8 IsSoftfilament=FunctionClose;
//UINT8 IsV5Extruder=FunctionClose;
//void FunctionSetting(void);

//void lcd_fill_redblock(void)
//{
//  if(FunctionOpen==BaseFunction)
//  {
//    LCD_Fill_Default(89+5,88+5,89+5+20,88+5+12);
//  }
//  if(FunctionOpen==ColorMixingFunction)
//  {
//    LCD_Fill_Default(202+5,88+5,202+5+20,88+5+12);
//  }
//  if(FunctionOpen==PowerOffRecoveryFunction)
//  {
//    LCD_Fill_Default(314+5,88+5,314+5+20,88+5+12);
//  }
//  if(FunctionOpen==MatCheckFunction)
//  {
//    LCD_Fill_Default(424+5,88+5,424+5+20,88+5+12);
//  }
//  if(FunctionOpen==BlockDetectFunction)
//  {
//    LCD_Fill_Default(88+5,178+5,88+5+20,178+5+12);
//  }
//  if(FunctionOpen==BedLevelFunction || 2 == BedLevelFunction)
//  {
//    LCD_Fill_Default(202+5,178+5,202+5+20,178+5+12);
//  }
//  if(FunctionOpen==IsSoftfilament)
//  {
//    LCD_Fill_Default(314+5,178+5,314+5+20,178+5+12);
//  }
//  if(FunctionOpen==IsV5Extruder)
//  {
//    LCD_Fill_Default(424+5,178+5,424+5+20,178+5+12);
//  }
//}

//void function_touchscan(void)
//{
//  if(TouchXY_NoBeep(25,87,430,248))//选择功能键区域
//  {
//    if(touchxy(25,87,117,157)) //基础功能
//    {
//      BaseFunction= (BaseFunction ? FunctionClose : FunctionOpen);
//    }
//    else if(touchxy(136,87,229,157)) //混色功能
//    {
//      ColorMixingFunction= (ColorMixingFunction ? FunctionClose : FunctionOpen);
//    }
//    else if(touchxy(252,87,342,157)) //断电续打功能
//    {
//      PowerOffRecoveryFunction= (PowerOffRecoveryFunction ? FunctionClose : FunctionOpen);
//    }
//    else if(touchxy(359,87,451,157)) //断料检测功能
//    {
//      // 20171130 新版限位开关判断条件,低电平则表示无料
//      // 设置断料检测为2
//      MatCheckFunction= (MatCheckFunction ? FunctionClose : FunctionOpen);
//      if(MatCheckFunction)
//        BlockDetectFunction=0;
//    }
//    else if(touchxy(25,178,115,245)) //堵料检测功能
//    {
//      BlockDetectFunction= (BlockDetectFunction ? FunctionClose : FunctionOpen);
//      if(BlockDetectFunction)
//        MatCheckFunction=0;
//    }
//    else if(touchxy(136,178,230,248)) //自动调平功能
//    {
//      BedLevelFunction= (BedLevelFunction ? FunctionClose : 2);//FunctionOpen);
//    }
//    else if(touchxy(252,178,342,248)) //使用软料齿轮
//    {
//      IsSoftfilament= (IsSoftfilament ? FunctionClose : FunctionOpen);
//    }
//    else if(touchxy(359,178,451,248)) //使用软料齿轮
//    {
//      IsV5Extruder= (IsV5Extruder ? FunctionClose : FunctionOpen);
//    }
//    gui_set_curr_display(FunctionSetting);
//    return ;
//  }
//  if(TouchXY_NoBeep(60,250,430,320))//确认或取消键
//  {
//    if(touchxy(60,250,240,320)) //确认键
//    {
//      if(BaseFunction || ColorMixingFunction || PowerOffRecoveryFunction || MatCheckFunction || BlockDetectFunction || BedLevelFunction || IsSoftfilament || IsV5Extruder)
//      {
//        t_sys_data_current.enable_color_mixing=ColorMixingFunction;
//        t_sys_data_current.enable_powerOff_recovery=PowerOffRecoveryFunction;
//        t_sys_data_current.enable_material_check= MatCheckFunction;
//        t_sys_data_current.enable_block_detect=BlockDetectFunction;
//        t_sys_data_current.enable_bed_level=BedLevelFunction;
//        t_sys_data_current.enable_soft_filament = IsSoftfilament;
//        t_sys_data_current.enable_v5_extruder = IsV5Extruder;
//        respond_gui_send_sem(ConfirmChangeFunctionValue);
//      }
//    }
//    (void)touchxy(240,250,430,320); //取消键
//    gui_set_curr_display(MachineSetting);
//    return ;
//  }
//}

//void FunctionSetting(void)
//{
//  if(gui_is_refresh())
//  {
//    display_picture(40);
//    lcd_fill_redblock(); //填充红色块
//  }
//  function_touchscan();//扫描选择功能按键
//}


void MachineSetting(void)
{
  if (gui_is_refresh())
  {
    display_picture(38);
  }

  // 定制机型8-3，K5机器不能设置机型
  if ((!((8 == t_sys_data_current.custom_model_id) && (K5 == t_sys_data_current.model_id))) && touchxy(48, 145, 154, 228)) //机型设置
  {
    gui_set_curr_display(ModelSetting);
    return ;
  }

  if (touchxy(48, 235, 154, 320)) //logo选择设置
  {
    gui_set_curr_display(logoSetting);
    return ;
  }

  if (touchxy(188, 145, 295, 228)) //界面设置
  {
    gui_set_curr_display(PictureSetting);
    return ;
  }

  if (touchxy(327, 145, 433, 228)) //功能设置
  {
    gui_set_curr_display(FunctionSetting);
    return ;
  }

  if (touchxy(328, 28, 435, 110)) //測試设置
  {
    t_sys_data_current.have_set_machine = 0;
    motion_3d.enable_board_test = 1;
    gui_set_curr_display(board_test_model_select);
    return ;
  }

  if (touchxy(0, 0, 145, 65))
  {
    gui_set_curr_display(settingF);
    return ;
  }
}

#ifdef __cplusplus
}//extern "C" {
#endif


