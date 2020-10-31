#include "view_common.h"
#include "sysconfig_data.h"
#include "user_interface.h"
#include "ff.h"
#include "font.h"
#include "controlfunction.h"
#include "threed_engine.h"
#include "uart.h"
#include "globalvariables.h"
#include "ALter.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace gui_common
{
  volatile bool is_finish_print_beep_once = true;
}

#define DEFAULT_TEXT_COLOR RED //文字显示颜色

volatile menufunc_t currentdisplay;
volatile menufunc_t predisplay;
static volatile uint8_t refresh_flag = 1;       //界面刷新信号
static volatile uint8_t rtc_flag = 0;           //实时rtc刷新信号
volatile INT print_flage = 0;        //正在打印标志
volatile INT pauseprint = 0;         //暂停打印标志
//INT keysound=1,alarmsound=1; //按键声音，报警声音

struct _textrange  NozzleTempTextRange;    //喷嘴温度的显示区域
struct _textrange  HotBedTempTextRange;    //热床温度的显示区域
struct _textrange  NozzleTargetTempTextRange;   //喷嘴目标温度的显示区域
struct _textrange  HotBedTargetTempTextRange;   //热床目标温度的显示区域
struct _textrange  PrintScheduleTextRange;    //打印进度的显示区域
struct _textrange  PrintTimeTextRange;    //打印时间的显示区域
struct _textrange  SpeedRange;    //速度的显示区域
struct _textrange  CavityTempTextRange;    //腔体温度的显示区域
struct _textrange  CavityTargetTempTextRange;   //腔体目标温度的显示区域
struct _textrange  RunTimeTextRange; // 运行时间的显示区域
struct _textrange  PrintSpeedTextSharp; //打印速度的显示区域
struct _textrange  FanSpeedTextSharp; //风扇速度的显示区域
struct _textrange  LedSwitchTextSharp;
struct _textrange  CavityTempOnTextSharp; //腔体温度开关的显示区域
struct _textrange  ZOffsetZeroTextSharp; //Z零点偏移的显示区域
struct _textrange  XPosTextShape; //X位置的显示区域
struct _textrange  YPosTextShape; //Y位置的显示区域
struct _textrange  ZPosTextShape; //Z位置的显示区域


char printname[MK_MAX_LFN];            //打印文件的名称
char printnameb[MK_MAX_LFN];            //打印文件的名称


void change_print_file_name(void)
{
  strcpy(printnameb, printname);

  if (strlen(printnameb) > (MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)))
  {
    printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0)] = 0;
    printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 1] = '.';
    printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 2] = '.';
    printnameb[MaxDisplayTextLength - (t_sys.lcd_ssd1963_43_480_272 ? 3 : 0) - 3] = '.';
  }
}

void gui_init_lcd_touch(void)
{
  LCD_Init();//触摸屏初始化
  (void)sys_delay(100);
  (void)tp_dev.init();//触摸初始化
  (void)sys_delay(100);
}

void gui_set_curr_display(const volatile menufunc_t display)
{
  currentdisplay = display;
  refresh_flag = 1;//界面刷新
}

struct _touch touch = {0};
static void gui_touch_scan(void)        //触摸扫描函数
{
  (void)tp_dev.scan(0);

  if (tp_dev.sta & (0x80))   //有触摸设置相关参数
  {
    touch.touch_flag = 1;
    touch.down_flag = 1;
    touch.up_flag = 0;
    touch.pixel_x = tp_dev.x[1];
    touch.pixel_y = tp_dev.y[1];
  }
  else                       //没触摸设置相关参数
  {
    touch.up_flag = 1;
    touch.down_flag = 0;
  }
}

static void gui_rtc_ctrl(void)
{
  static uint32_t rtc_refresh_timeout = 0;

  if (rtc_refresh_timeout < sys_task_get_tick_count())
  {
    rtc_flag = 1;
    rtc_refresh_timeout = sys_task_get_tick_count() + 500;
  }
}

void gui_display_touch_ctrl(void)
{
  gui_touch_scan(); //触摸扫描函数，处理扫描数据
  gui_rtc_ctrl(); //刷新rtc信号

  if (touch.touch_flag && touch.up_flag) //有触摸信号，进入函数处理
  {
    Set_BL(100);
    currentdisplay();                 //进入模块函数处理触摸信号
    touch.touch_flag = 0;              //关闭相关的tch信号
    touch.up_flag = 0;                 //关闭相关的tch信号
  }
  else if (rtc_flag)                  //有rtc信号，进入函数处理
  {
    currentdisplay();                 //进入模块函数处理rtc信号
    rtc_flag = 0;                       //给rtc_flag信号赋值
  }
  else if (refresh_flag)
  {
    Set_BL(100);
    currentdisplay();
    refresh_flag = 0;
  }
}

uint8_t gui_is_refresh(void)            //检测界面是否需要刷新
{
  if (refresh_flag)
  {
    refresh_flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

uint8_t gui_is_rtc(void)            //检测界面rtc是否需要刷新
{
  if (rtc_flag)
  {
    rtc_flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

#define TICKCOUNT  100
INT IstouchxyDown(INT touchx, INT touchy, INT rangex, INT rangey) //add by zouxb
{
  if (touch.touch_flag &&
      (touch.pixel_x > touchx) &&
      (touch.pixel_x <  rangex) &&
      (touch.pixel_y > touchy) &&      //触摸区域
      (touch.pixel_y < rangey))       //touchx<x<rangex;touchy<y<rangey
  {
    if (t_sys.key_sound)              //如果设置了按键响声
    {
      buzz(TICKCOUNT);     //响铃函数
    }

    return 1;
  }
  else
  {
    return 0;
  }
}

INT IstouchxyUp(void)  //add by zouxb
{
  if (touch.up_flag)
  {
    touch.touch_flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}


INT touchxy(INT touchx, INT touchy, INT rangex, INT rangey)
{
  if (touch.up_flag &&
      touch.touch_flag &&
      (touch.pixel_x > touchx) &&
      (touch.pixel_x <  rangex) &&
      (touch.pixel_y > touchy) &&
      (touch.pixel_y < rangey)) //touchx<x<rangex;touchy<y<rangey  //触摸区域
  {
    serial_print[0] = false;
    serial_print[1] = false;

    if (t_sys.key_sound)              //如果设置了按键响声
    {
      buzz(TICKCOUNT);     //响铃函数
    }

    return 1;
  }
  else
  {
    return 0;
  }
}

INT TouchXY_NoBeep(INT touchx, INT touchy, INT rangex, INT rangey)
{
  if (touch.up_flag &&
      touch.touch_flag &&
      (touch.pixel_x > touchx) &&
      (touch.pixel_x <  rangex) &&
      (touch.pixel_y > touchy) &&
      (touch.pixel_y < rangey)) //touchx<x<rangex;touchy<y<rangey  //触摸区域
  {
    (void)sys_os_delay(200);
    return 1;
  }
  else
  {
    return 0;
  }
}

INT touchrange(INT touchx, INT touchy, INT rangex, INT rangey)
{
  if (touch.up_flag && touch.touch_flag &&
      (touch.pixel_x > touchx) &&
      (touch.pixel_x < (touchx + rangex)) &&
      (touch.pixel_y > touchy) &&
      (touch.pixel_y < (touchy + rangey)))       //触摸区域
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小
//size:字体大小
FIL hanziziku;
void Get_HzMat(PUCHAR code, PUCHAR mat, UCHAR size)
{
  UCHAR qh, ql;
  UCHAR i;
  ULONG foffset;
  UINT readbyte;
  UCHAR csize = (UCHAR)(((size / 8) + ((size % 8) ? 1 : 0)) * (size)); //得到字体一个字符对应点阵集所占的字节数
  qh = *code;
  ql = *(++code);

  if ((qh < 0x81) || (ql < 0x40) || (ql == 0xff) || (qh == 0xff)) //非 常用汉字
  {
    for (i = 0; i < csize; ++i)*mat++ = 0x00; //填充满格

    return; //结束访问
  }

  if (ql < 0x7f)
  {
    ql -= 0x40; //注意!
  }
  else
  {
    ql -= 0x41;
  }

  qh -= 0x81;
  foffset = (((ULONG)190 * qh) + ql) * csize; //得到字库中的字节偏移量

  switch (size)
  {
  case 12:
    break;

  case 16:
    break;

  case 24:
    (void)f_open(&hanziziku, "0:/GBK24.FON", FA_READ);  //打開字庫文件
    (void)f_lseek(&hanziziku, foffset);
    (void)f_read(&hanziziku, mat, csize, &readbyte);
    (void)f_close(&hanziziku);
    break;

  default:
    break;
  }
}

//设置文字显示的区域范围
void SetTextDisplayRange(INT x, INT y, INT rangex, INT rangey, struct _textrange *p)
{
  if (t_sys.lcd_ssd1963_43_480_272 && (lcddev.height == 272))
  {
    X_1963(x);
    Y_1963(y);
  }

  p->x = x;
  p->y = y;
  p->rangex = rangex;
  p->rangey = rangey;
}

//将指定区域内的像素读出
void ReadTextDisplayRangeInfo(struct _textrange textrange, PUSHORT p)
{
  INT i, ii;

  for (i = 0; i < textrange.rangey; ++i)
  {
    for (ii = 0; ii < textrange.rangex; ++ii)
    {
      *p++ = LCD_ReadPoint((USHORT)(ii + textrange.x), (USHORT)(i + textrange.y));
    }
  }
}

//复制一份读出的像素数据
void CopyTextDisplayRangeInfo(struct _textrange textrange, PUSHORT psource, PUSHORT p)
{
  INT i, ii;

  for (i = 0; i < textrange.rangey; ++i)
  {
    for (ii = 0; ii < textrange.rangex; ++ii)
    {
      *p++ = *psource++;
    }
  }
}

//将合成了文字信息的像素数据重新写入指定区域
void WriteTextDisplayRangeInfo(struct _textrange textrange, PUSHORT p)
{
  INT i, ii;

  for (i = 0; i < textrange.rangey; ++i)
  {
    for (ii = 0; ii < textrange.rangex; ++ii)
    {
      LCD_Fast_DrawPoint((USHORT)(textrange.x + ii), (USHORT)(textrange.y + i), *p++);
    }
  }
}

//将单个英文字符点阵信息合成到读出的像素数据中
void WriteEnglishCharToBuf(struct _textrange textrange, CONST PUCHAR pstr, INT XHeadPos, PUSHORT p, INT size, USHORT colour)
{
  //参数*pstr字符，参数testrange是显示区域，参数p是数组，参数size是字体大小，colour是字体颜色
  INT CharCodeNum, BitData, YPos = 0, XPos = -1;
  UCHAR CharCode;
  UCHAR CharPos = *pstr - ' ';  //偏移量

  if (size == 24) //24*12的字体
  {
    for (CharCodeNum = 0; CharCodeNum < 36; ++CharCodeNum) //一个24*12字符对应36个字节数据
    {
      CharCode = asc2_2412[CharPos][CharCodeNum]; //24*12字符码  //从上到下，从左到右，高位在前

      if ((CharCodeNum % 3) == 0) //3个字节为1列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; ++BitData) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          p[XHeadPos + XPos + (YPos * textrange.rangex)] = colour; //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        ++YPos; //下一行
      }
    }
  }
}

//将单个中文字符点阵信息合成到读出的像素数据中
void WriteChineseCharToBuf(struct _textrange textrange, CONST PUCHAR pstr, INT XHeadPos, PUSHORT p, INT size, USHORT colour)
{
  //参数*pstr字符，参数testrange是显示区域，参数p是数组，参数size是字体大小，colour是字体颜色
  INT CharCodeNum, BitData, YPos = 0, XPos = -1;
  UCHAR CharCode;
  UCHAR dzk[72]; //一个汉字的点阵数据72字节Buf

  if (size == 24)
  {
    Get_HzMat(pstr, dzk, (UCHAR)size); //得到72字节的点阵数据

    for (CharCodeNum = 0; CharCodeNum < 72; ++CharCodeNum) //72字节
    {
      CharCode = dzk[CharCodeNum];  //24*24字符码

      if ((CharCodeNum % 3) == 0) //3个字节为一列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; ++BitData) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          p[XHeadPos + XPos + (YPos * textrange.rangex)] = colour; //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        YPos++; //下一行
      }
    }
  }
}

//在指定的区域写文字
void DisplayTextInRange(PUCHAR pstr, struct _textrange textrange, PUSHORT p, INT size, USHORT colour)
{
  //参数pstr字符串，参数testrange是显示区域，参数p是数组，参数size是字体大小，colour是字体颜色
  INT XPos = 0;

  while (*pstr != 0)
  {
    if (*pstr <= 126) // 英语
    {
      if (XPos <= (textrange.rangex - 12)) //防止溢出  //一个英文字符的宽度为12
      {
        WriteEnglishCharToBuf(textrange, pstr, XPos, p, size, colour); //将英文字符写入数组中
        XPos = XPos + 12; //一个英文字符的宽度为12
        pstr = pstr + 1; //一个字节
      }
    }
    else //汉字
    {
      if (XPos <= (textrange.rangex - 24)) //防止溢出  //一个中文字符的宽度为24
      {
        WriteChineseCharToBuf(textrange, pstr, XPos, p, size, colour); //将中文字符写入数组中
        XPos = XPos + 24; //一个中文字符的宽度为24
        pstr = pstr + 2; //两个字节
      }
    }
  }

  WriteTextDisplayRangeInfo(textrange, p); //将数组中的数据写到lcd中
}

void DisplayTextInRangeDefault(PUCHAR pstr, struct _textrange textrange, PUSHORT p)
{
  DisplayTextInRange(pstr, textrange, p, DefaultTextSize, DEFAULT_TEXT_COLOR);
}

//显示英文字符
void DisplayEnglishChar(CONST PUCHAR pstr, INT XHeadPos, INT YHeadPos, INT size, USHORT colour)
{
  //参数*pstr是字符，参数XHeadPos、YHeadPos是显示的起始位置，参数size是字体大小，colour是字体颜色
  INT CharCodeNum, BitData, YPos = 0, XPos = -1;
  UCHAR CharCode;
  UCHAR CharPos = *pstr - ' ';  //偏移量

  if (size == 24) //24*12字体
  {
    for (CharCodeNum = 0; CharCodeNum < 36; ++CharCodeNum) //36个字节
    {
      CharCode = asc2_2412[CharPos][CharCodeNum]; //24*12字符码

      if ((CharCodeNum % 3) == 0) //3个字节为一列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; ++BitData) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          LCD_Fast_DrawPoint((USHORT)(XHeadPos + XPos), (USHORT)(YHeadPos + YPos), colour); //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        ++YPos; //下一行
      }
    }
  }
}

//显示中文字符
static void DisplayChineseChar(CONST PUCHAR pstr, INT XHeadPos, INT YHeadPos, INT size, USHORT colour)
{
  //参数*pstr是中文字符，参数XHeadPos、YHeadPos是显示的起始位置，参数size是字体大小，colour是字体颜色
  INT CharCodeNum, BitData, YPos = 0, XPos = -1;
  UCHAR CharCode;
  UCHAR dzk[72]; //一个汉字的点阵数据72字节Buf

  if (size == 24)
  {
    Get_HzMat(pstr, dzk, (UCHAR)size); //得到72字节的点阵数据

    for (CharCodeNum = 0; CharCodeNum < 72; ++CharCodeNum) //72字节
    {
      CharCode = dzk[CharCodeNum];  //24*24字符码

      if ((CharCodeNum % 3) == 0) //3个字节为一列
      {
        ++XPos; //下一列
        YPos = 0; //行归零
      }

      for (BitData = 0; BitData < 8; ++BitData) //一个字节有8位，每一位都对应一个像素
      {
        if (CharCode & 0x80) //有效像素点写颜色数据
        {
          LCD_Fast_DrawPoint((USHORT)(XHeadPos + XPos), (USHORT)(YHeadPos + YPos), colour); //一个像素是16位的颜色数据
        }

        CharCode <<= 1; //下一个像素
        ++YPos; //下一行
      }
    }
  }
}

void DisplayText(PUCHAR pstr, INT x, INT y, INT size, USHORT colour)
{
  if (t_sys.lcd_ssd1963_43_480_272 && (lcddev.height == 272))
  {
    X_1963(x);
    x += 1;
    Y_1963(y);
  }

  //参数pstr字符串，参数xy显示区域，参数size字体大小
  while (*pstr != 0)
  {
    if (*pstr <= 126) //英语
    {
      DisplayEnglishChar(pstr, x, y, size, colour); //英文字符
      x += size / 2;
      ++pstr;
    }
    else // 汉字
    {
      DisplayChineseChar(pstr, x, y, size, colour); //中文字符
      x += size;
      pstr += 2;
    }
  }
}

void DisplayTextDefault(PUCHAR pstr, INT x, INT y)
{
  DisplayText(pstr, x, y, DefaultTextSize, DEFAULT_TEXT_COLOR);
}





#ifdef __cplusplus
} //extern "C" {
#endif







