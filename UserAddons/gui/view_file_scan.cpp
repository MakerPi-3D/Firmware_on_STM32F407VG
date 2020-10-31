#include "machinecustom.h"
#include "view_commonf.h"
#include  "view_common.h"
#include "globalvariables.h"
#include  "interface.h"
#include "sysconfig_data.h"
#include "user_interface.h"
#include "view_icons.h"

#ifdef __cplusplus
extern "C" {
#endif

static INT  catalogi = 0;      //文件个数

void drawup(USHORT x, USHORT y)        //画上一页按钮
{
  POINT_COLOR = (USHORT)DEFAULT_DRAW_COLOR;
  LCD_DrawLine(x - 24, y, x, y - 24);
  LCD_DrawLine(x, y - 24, x + 24, y);
  LCD_DrawLine(x - 24, y, x - 12, y);
  LCD_DrawLine(x + 12, y, x + 24, y);
  LCD_DrawLine(x - 12, y + 24, x - 12, y);
  LCD_DrawLine(x + 12, y + 24, x + 12, y);
  LCD_DrawLine(x - 12, y + 24, x + 12, y + 24);
}

void drawdown(USHORT x, USHORT y)      //画下一页按钮，x y为中心点
{
  POINT_COLOR = (USHORT)DEFAULT_DRAW_COLOR;
  LCD_DrawLine(x - 24, y, x, y + 24);
  LCD_DrawLine(x, y + 24, x + 24, y);
  LCD_DrawLine(x - 24, y, x - 12, y);
  LCD_DrawLine(x + 12, y, x + 24, y);
  LCD_DrawLine(x - 12, y - 24, x - 12, y);
  LCD_DrawLine(x + 12, y - 24, x + 12, y);
  LCD_DrawLine(x - 12, y - 24, x + 12, y - 24);
}

void DrawChangePageArrow(void)
{
  if (t_gui_p.CurrentPage > 1)  //判断是否有上一页
  {
    drawup(415, 120);
  }

  if (t_gui_p.IsHaveNextPage)   //判断是否有下一页
  {
    drawdown(415, 225);
  }
}


void DisplayFileItem(void)
{
  INT i;
  INT ii;
  CHAR printnameb[MK_MAX_LFN];
  catalogi = 0;

  for (i = 0; i < OnePageNum ; ++i)
  {
    if (t_gui_p.IsHaveFile[catalogi])
    {
      ++catalogi;                   //查询当前页面的文件
    }
  }

  for (ii = 0; ii < catalogi; ++ii)
  {
    //显示文件名称
    strcpy(printnameb, DisplayFileName[ii]);

    if (strlen(printnameb) > (MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)))
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
    }

    if (t_gui_p.IsDir[ii])
    {
      LCD_Color_Fill(55, (USHORT)(97 + (ii * (24 + 16))), 55 + 23, (USHORT)((97 + (ii * (24 + 16))) + 23), (PUSHORT)DirIcon);
    }
    else
    {
      LCD_Color_Fill(55, (USHORT)(97 + (ii * (24 + 16))), 55 + 23, (USHORT)((97 + (ii * (24 + 16))) + 23), (PUSHORT)FileIcon);
    }

    DisplayTextDefault((UINT8 *)printnameb, 55 + 24, 97 + (ii * (24 + 16)));
  }
}


void DisplayCurrentPage(void)
{
  CHAR buffer[20];
  snprintf(buffer, sizeof(buffer), "%2d", t_gui_p.CurrentPage); //虚拟打印当前页码

  if (t_gui_p.CurrentPage < 10)
  {
    if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id) //中文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 193, 262); //显示当前页码（小于10）
    }
    else if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id) //日文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 231, 262); //显示当前页码（小于10）
    }
    else if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id) //英文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 231, 262); //显示当前页码（小于10）
    }
    else if (PICTURE_IS_KOREA == t_sys_data_current.pic_id) //韓文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 231, 262); //显示当前页码（小于10）
    }
    else if (PICTURE_IS_RUSSIA == t_sys_data_current.pic_id) //俄文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 231, 262); //显示当前页码（小于10）
    }
    else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id) //繁体图片
    {
      DisplayTextDefault((UINT8 *)buffer, 193, 262); //显示当前页码（小于10）
    }
  }
  else
  {
    if (PICTURE_IS_CHINESE == t_sys_data_current.pic_id) //中文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 198, 262);
    }
    else if (PICTURE_IS_JAPANESE == t_sys_data_current.pic_id) //日文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 237, 262);
    }
    else if (PICTURE_IS_ENGLISH == t_sys_data_current.pic_id) //英文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 237, 262);
    }
    else if (PICTURE_IS_KOREA == t_sys_data_current.pic_id) //韓文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 237, 262);
    }
    else if (PICTURE_IS_RUSSIA == t_sys_data_current.pic_id) //俄文图片
    {
      DisplayTextDefault((UINT8 *)buffer, 237, 262);
    }
    else if (PICTURE_IS_CHINESE_TRADITIONAL == t_sys_data_current.pic_id) //繁体图片
    {
      DisplayTextDefault((UINT8 *)buffer, 198, 262);
    }
  }
}

UINT8 ClickBackButton(void)
{
  if (touchxy(0, 0, 150, 65))     //返回按钮
  {
    if (t_gui_p.IsRootDir)    //看是否为根目录
    {
      gui_set_curr_display(maindisplayF);
      return 1;
    }
    else                      //不是根目录，返回上级目录
    {
      gui_set_curr_display(filescanF);
      respond_gui_send_sem(BackLastDirValue);
      return 1;
    }
  }

  return 0;
}

UINT8 ClickFileItem(void)
{
  INT ii;

  if (touchxy(55, 97, 371, 252)) //判断是否选中条目
  {
    for (ii = 0; ii < catalogi; ++ii)
    {
      if (touchrange(55, 97 + (ii * 40), 315, 40)) //进一步判断
      {
        if (t_gui_p.IsDir[ii])       //看是否为文件夹
        {
          //是文件夹，重新获得数据，打开文件夹
          gui_set_curr_display(filescanF);
          strcpy(SettingInfoToSYS.DirName, DisplayFileName[ii]);
          respond_gui_send_sem(OpenDirValue);
          return 1;
        }
        else    //不是文件夹，跳转到文件打印界面
        {
          strcpy(printname, DisplayFileName[ii]);
          gui_set_curr_display(printconfirmF);
          return 1;
        }
      }
    }

    return 0;
  }

  return 0;
}

UINT8 ChangePage(void)
{
  if (t_gui_p.CurrentPage > 1) //看是否有上一页，按钮才生效
  {
    if (touchxy(395, 90, 480, 160))
    {
      gui_set_curr_display(filescanF);
      respond_gui_send_sem(LastPageValue);
      return 1;
    }
  }

  if (t_gui_p.IsHaveNextPage)  //看是否有下一页，按钮才生效
  {
    if (touchxy(395, 160, 480, 250))
    {
      gui_set_curr_display(filescanF);
      respond_gui_send_sem(NextPageValue);
      return 1;
    }
  }

  return 0;
}

void filescanF(void)
{
  if (gui_is_refresh())
  {
    sys_os_delay(50);
    display_picture(5);
    DrawChangePageArrow(); //显示翻页箭头
    DisplayFileItem(); //显示文件或文件夹条目
    DisplayCurrentPage(); //显示当前页面
  }

  if (ClickBackButton()) //点击了返回键处理
  {
    return;
  }

  if (ClickFileItem()) //点击了文件或文件夹条目处理
  {
    return;
  }

  if (ChangePage()) //点击了上一页或下一页处理
  {
    return;
  }

  if (gui_is_rtc()) //SD卡拔出返回主界面
  {
    if (0 == t_gui_p.SDIsInsert)
    {
      gui_set_curr_display(maindisplayF);  //返回主界面
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

