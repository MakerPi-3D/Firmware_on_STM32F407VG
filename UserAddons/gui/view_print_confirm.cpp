
#include "view_commonf.h"
#include "globalvariables.h"
#include "view_common.h"
#include  "interface.h"
#include  "ff.h"
#include "user_debug.h"
#include "view_pic_display.h"
#include "user_fatfs.h" //挂在sd
#include "sysconfig_data.h"
#include "file_sgcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FILE_COUNT 150


#define CONFIRM 1 //确定键键值
#define CANCEL  2 //取消键键值
void printconfirmF(void)
{
  char *printnameb;   //文件名缓存
  UINT32 keybuf = 0;//按键值缓存
  static UINT32 sgcodenamelen = 0;//sgcode显示的字符数不普通gcode少12个，所以当选择sgcode文件时将该变量赋值12
  INT length;//最后显示文件名的字节数，为了让名字显示在显示栏的中间

  /**************************界面刷新****************************/
  if (gui_is_refresh())
  {
    sgcodenamelen = 0;
    printnameb = (char *)malloc(MK_MAX_LFN);

    if (t_gui_p.IsRootDir)
    {
      (void)snprintf(printnameb, MK_MAX_LFN, "%s%s", CurrentPath, printname);
    }
    else
    {
      (void)snprintf(printnameb, MK_MAX_LFN, "%s/%s", CurrentPath, printname);
    }

    //(void)strcat(printnameb, printname);
    //    CHAR printnameb[MK_MAX_LFN];

    if (strstr(printname, ".sgcode"))
    {
      display_picture(98);
      sgcode_extract_bmp(printnameb);
      sgcodenamelen = 12;
    }
    else
    {
      display_picture(6);
    }

    strncpy(&printnameb[0], printname, MK_MAX_LFN);

    if (strlen(printnameb) > (MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen))
    {
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen] = 0;
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen - 1] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen - 2] = '.';
      printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - sgcodenamelen - 3] = '.';
    }

    length = (INT)strlen(printnameb);
    DisplayTextDefault((UINT8 *)printnameb, ((t_sys.lcd_ssd1963_43_480_272 ? 212 : 240) - ((length / 2) * 12)) - (sgcodenamelen * 10), 105); //240-(length/2)*12   是为了让文字显示在中间

    if (sgcodenamelen)
    {
      diplayBMP(t_sys.lcd_ssd1963_43_480_272 ? 24 : 44);
    }

    free(printnameb);
    printnameb = NULL;
  }

  /*************************按键检测***********************************/
  if (sgcodenamelen)
  {
    if (touchxy(0, 240, 135, 285))
    {
      keybuf = CONFIRM;
    }

    if (touchxy(136, 240, 260, 285))
    {
      sgcode_delete_bmp();
      keybuf = CANCEL;
    }
  }
  else
  {
    if (touchxy(136, 200, 240, 285))
    {
      keybuf = CONFIRM;
    }

    if (touchxy(240, 200, 425, 285))
    {
      keybuf = CANCEL;
    }
  }

  /************************按键值命令执行*****************************/
  if (keybuf == CONFIRM)
  {
    if (t_gui_p.IsDoorOpen)
    {
      gui_set_curr_display(DoorOpenWarning_StartPrint);
    }
    else
    {
      print_flage = 1;
      gui_set_curr_display(maindisplayF);
      strncpy(SettingInfoToSYS.PrintFileName, printname, sizeof(SettingInfoToSYS.PrintFileName));
      respond_gui_send_sem(FilePrintValue);
    }

    return ;
  }

  if (keybuf == CANCEL)
  {
    gui_set_curr_display(filescanF);
    return ;
  }

  if (gui_is_rtc())
  {
    if (0 == t_gui_p.SDIsInsert)
    {
      sgcode_delete_bmp();
      gui_set_curr_display(maindisplayF);
    }
  }
}

#ifdef __cplusplus
} //extern "C" {
#endif

